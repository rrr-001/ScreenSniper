#include "aimanager.h"
#include "aiconfigmanager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QDebug>

AiManager::AiManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void AiManager::generateImageDescription(const QImage &image)
{
    AIConfigManager::ServiceType serviceType = AIConfigManager::instance().getServiceType();
    QString apiKey = AIConfigManager::instance().getApiKey();

    // 验证 API Key
    if (apiKey.isEmpty())
    {
        emit errorOccurred("请在 config.ini 中配置正确的 API Key");
        return;
    }

    QString base64Image = imageToBase64(image);

    // --------------------------
    //       OpenAI 模式
    // --------------------------
    if (serviceType == AIConfigManager::OpenAI)
    {
        QString endpoint = AIConfigManager::instance().getEndpoint(); // https://api.openai.com/v1/responses
        QUrl url(endpoint);
        QNetworkRequest request(url);

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

        // image_url 对象
        QJsonObject imgUrlObj;
        imgUrlObj["url"] = "data:image/png;base64," + base64Image;

        // content 数组
        QJsonArray contentArray;

        // text
        QJsonObject textObj;
        textObj["type"] = "text";
        textObj["text"] = "请详细描述这张图片的内容，如有文字请提取。";
        contentArray.append(textObj);

        // image
        QJsonObject imgObj;
        imgObj["type"] = "input_image";
        imgObj["image_url"] = imgUrlObj;
        contentArray.append(imgObj);

        // user message
        QJsonObject messageObj;
        messageObj["role"] = "user";
        messageObj["content"] = contentArray;

        QJsonArray inputArray;
        inputArray.append(messageObj);

        // final body
        QJsonObject jsonBody;
        jsonBody["model"] = AIConfigManager::instance().getModelName(); // gpt-4o
        jsonBody["input"] = inputArray;
        jsonBody["max_tokens"] = 1000000; // 注意tokens设置过小可能导致限流 429

        // debug（你可以打开看看请求是否正确）
        qDebug() << "=== OpenAI Request ===";
        qDebug().noquote() << QJsonDocument(jsonBody).toJson(QJsonDocument::Indented);

        QNetworkReply *reply = m_networkManager->post(
            request,
            QJsonDocument(jsonBody).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
                { onOpenAIReply(reply); });

        return;
    }

    // --------------------------
    //       阿里云模式
    // --------------------------
    QUrl url("https://dashscope.aliyuncs.com/api/v1/services/aigc/multimodal-generation/generation");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QString base64WithPrefix = "data:image/png;base64," + base64Image;

    QJsonObject textObj;
    textObj["text"] = "请详细描述这张图片中的内容，如果是文字请提取出来。";

    QJsonObject imgObj;
    imgObj["image"] = base64WithPrefix;

    QJsonArray contentArray;
    contentArray.append(imgObj);
    contentArray.append(textObj);

    QJsonObject msgObj;
    msgObj["role"] = "user";
    msgObj["content"] = contentArray;

    QJsonArray messages;
    messages.append(msgObj);

    QJsonObject input;
    input["messages"] = messages;

    QJsonObject jsonBody;
    jsonBody["model"] = AIConfigManager::instance().getModelName();
    jsonBody["input"] = input;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(jsonBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            { onAliyunReply(reply); });
}

// =======================================
//        OpenAI Response 解析
// =======================================
void AiManager::onOpenAIReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        // 获取 HTTP 状态码
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // 特殊处理 429 错误：账号被限流了
        if (statusCode == 429)
        {
            emit errorOccurred("请求过于频繁，请稍后再试。OpenAI API 有速率限制，建议等待几秒后重试。");
            return;
        }

        // 其他错误
        QString errorMsg = "网络错误";
        if (statusCode > 0)
        {
            errorMsg += QString(" (HTTP %1)").arg(statusCode);
        }
        errorMsg += ": " + reply->errorString();

        emit errorOccurred(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    qDebug() << "=== OpenAI Response ===";
    qDebug().noquote() << doc.toJson(QJsonDocument::Indented);

    if (!doc.isObject())
    {
        emit errorOccurred("解析失败：不是有效 JSON");
        return;
    }

    QJsonObject obj = doc.object();

    // 错误处理
    if (obj.contains("error"))
    {
        QString msg = obj["error"].toObject()["message"].toString();
        QString errorType = obj["error"].toObject()["type"].toString();

        // 详细的错误信息
        QString fullMsg = "OpenAI API 错误";
        if (!errorType.isEmpty())
        {
            fullMsg += " (" + errorType + ")";
        }
        fullMsg += ": " + msg;

        emit errorOccurred(fullMsg);
        return;
    }

    // 正确格式：output → content → text
    if (obj.contains("output"))
    {
        QJsonArray output = obj["output"].toArray();

        if (!output.isEmpty())
        {
            QJsonObject first = output[0].toObject();

            if (first.contains("content"))
            {
                QJsonArray contentArr = first["content"].toArray();

                QString result;

                for (auto v : contentArr)
                {
                    QJsonObject c = v.toObject();
                    if (c["type"].toString() == "text")
                    {
                        result += c["text"].toString();
                    }
                }

                if (!result.isEmpty())
                {
                    emit descriptionGenerated(result);
                    return;
                }
            }
        }
    }

    emit errorOccurred("OpenAI 返回格式无法解析");
}

// =======================================
//        阿里云 Response 解析
// =======================================
void AiManager::onAliyunReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred("网络错误: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
    {
        emit errorOccurred("阿里云 JSON 解析失败");
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("code") && obj.contains("message"))
    {
        emit errorOccurred("阿里云错误: " + obj["message"].toString());
        return;
    }

    if (obj.contains("output"))
    {
        QJsonArray choices = obj["output"].toObject()["choices"].toArray();
        if (!choices.isEmpty())
        {
            QJsonObject msg = choices[0].toObject()["message"].toObject();
            QJsonArray content = msg["content"].toArray();

            QString result;
            for (auto v : content)
            {
                QJsonObject c = v.toObject();
                if (c.contains("text"))
                    result += c["text"].toString();
            }

            emit descriptionGenerated(result);
            return;
        }
    }

    emit errorOccurred("阿里云响应解析失败");
}

// =======================================
//        Base64（无换行）
// =======================================
QString AiManager::imageToBase64(const QImage &image)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    // 防止 Base64 自动换行（必须 KeepTrailingEquals）
    return QString::fromLatin1(bytes.toBase64(QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals));
}
