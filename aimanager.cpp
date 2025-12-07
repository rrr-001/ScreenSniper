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
        // QJsonObject imgUrlObj;
        // imgUrlObj["url"] = "data:image/png;base64," + base64Image;

        // content 数组
        QJsonArray contentArray;

        // text
        QJsonObject textObj;
        textObj["type"] = "input_text";
        textObj["text"] = "请描述这张图片的内容";
        contentArray.append(textObj);

        // image
        QJsonObject imgObj;
        imgObj["type"] = "input_image";
        imgObj["image_url"] = "data:image/png;base64," + base64Image;
        // 注意新版是直接传URL！！
        contentArray.append(imgObj);

        // user message
        QJsonObject messageObj;
        messageObj["role"] = "user";
        messageObj["content"] = contentArray;

        QJsonArray messagesArray;
        messagesArray.append(messageObj);

        // final body
        QJsonObject jsonBody;
        jsonBody["model"] = AIConfigManager::instance().getModelName();
        jsonBody["input"] = messagesArray; // 注意这里是 input !
        // jsonBody["max_tokens"] = 800; // 官方文档里面没有最大tokens数的字段

        // 创建一个用于显示的简化版本（截断 base64）
        QString base64Preview = base64Image.left(100) + "...[省略]..." + base64Image.right(50);

        QJsonObject imgObjPreview;
        imgObjPreview["type"] = "input_image";
        imgObjPreview["image_url"] = "data:image/png;base64," + base64Preview;

        QJsonArray contentArrayPreview;
        contentArrayPreview.append(textObj);
        contentArrayPreview.append(imgObjPreview);

        QJsonObject messageObjPreview;
        messageObjPreview["role"] = "user";
        messageObjPreview["content"] = contentArrayPreview;

        QJsonArray messagesArrayPreview;
        messagesArrayPreview.append(messageObjPreview);

        QJsonObject jsonBodyPreview;
        jsonBodyPreview["model"] = AIConfigManager::instance().getModelName();
        jsonBodyPreview["input"] = messagesArrayPreview;

        QByteArray requestData = QJsonDocument(jsonBody).toJson(QJsonDocument::Indented);

        qDebug() << "========================================";
        qDebug() << "=== OpenAI 请求信息 ===";
        qDebug() << "端点:" << endpoint;
        qDebug() << "模型:" << AIConfigManager::instance().getModelName();
        qDebug() << "Base64 图片长度:" << base64Image.length() << "字符";
        qDebug() << "请求体总大小:" << requestData.size() << "字节";
        qDebug() << "\n--- JSON 结构预览 (base64已截断) ---";
        qDebug().noquote() << QJsonDocument(jsonBodyPreview).toJson(QJsonDocument::Indented);
        qDebug() << "========================================";

        QNetworkReply *reply = m_networkManager->post(request, requestData);
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
    textObj["text"] = "请详细描述这张图片中的内容";

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

    // 获取 HTTP 状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "========================================";
    qDebug() << "=== OpenAI 响应信息 ===";
    qDebug() << "HTTP 状态码:" << statusCode;
    qDebug() << "错误码:" << reply->error();

    QByteArray data = reply->readAll();
    qDebug() << "响应体大小:" << data.size() << "字节";

    QJsonDocument doc = QJsonDocument::fromJson(data);

    qDebug() << "--- 完整响应 JSON ---";
    qDebug().noquote() << doc.toJson(QJsonDocument::Indented);
    qDebug() << "========================================";

    // 直接提取并 emit message
    if (doc.isObject())
    {
        QJsonObject obj = doc.object();

        // 检查是否有 error.message
        if (obj.contains("error"))
        {
            QString errorMessage = obj["error"].toObject()["message"].toString();
            if (!errorMessage.isEmpty())
            {
                emit errorOccurred(errorMessage);
                return;
            }
        }
    }

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

    // OpenAI Chat API 格式：choices → message → content
    if (obj.contains("choices"))
    {
        QJsonArray choices = obj["choices"].toArray();

        if (!choices.isEmpty())
        {
            QJsonObject firstChoice = choices[0].toObject();

            if (firstChoice.contains("message"))
            {
                QJsonObject message = firstChoice["message"].toObject();

                if (message.contains("content"))
                {
                    QString result = message["content"].toString();

                    if (!result.isEmpty())
                    {
                        emit descriptionGenerated(result);
                        return;
                    }
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
