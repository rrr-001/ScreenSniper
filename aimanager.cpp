#include "aimanager.h"
#include "aiconfigmanager.h"

AiManager::AiManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void AiManager::generateImageDescription(const QImage &image)
{
    QString apiKey = AIConfigManager::instance().getApiKey();
    if (apiKey.isEmpty() || apiKey == "sk-c30504bf26dd4dfebb3342e7f2a9af4d") {
        emit errorOccurred("请在 config.ini 中配置正确的 API Key");
        return;
    }

    // 1. 准备 URL
    QUrl url("https://dashscope.aliyuncs.com/api/v1/services/aigc/multimodal-generation/generation");
    QNetworkRequest request(url);

    // 2. 设置 Header
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    // 3. 处理图片为 Base64
    QString base64Image = "data:image/png;base64," + imageToBase64(image);

    // 4. 构造 JSON Body (适配 Qwen-VL 格式)
    /*
     {
        "model": "qwen-vl-max",
        "input": {
            "messages": [
                {
                    "role": "user",
                    "content": [
                        {"image": "data:image/png;base64,..."},
                        {"text": "请简要描述这张图片的内容"}
                    ]
                }
            ]
        }
     }
    */

    QJsonObject textContent;
    textContent["text"] = "请详细描述这张图片中的内容，如果是文字请提取出来。";

    QJsonObject imageContent;
    imageContent["image"] = base64Image;

    QJsonArray contentArray;
    contentArray.append(imageContent);
    contentArray.append(textContent);

    QJsonObject message;
    message["role"] = "user";
    message["content"] = contentArray;

    QJsonArray messages;
    messages.append(message);

    QJsonObject input;
    input["messages"] = messages;

    QJsonObject jsonBody;
    jsonBody["model"] = AIConfigManager::instance().getModelName();
    jsonBody["input"] = input;

    // 5. 发送 POST 请求
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(jsonBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onNetworkReply(reply);
    });
}

void AiManager::onNetworkReply(QNetworkReply *reply)
{
    reply->deleteLater(); // 稍后自动释放

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("网络错误: " + reply->errorString());
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();

    // 检查是否有 API 错误
    if (jsonObj.contains("code") && jsonObj.contains("message")) {
        // DashScope 只有在出错时才会在顶层返回 code (非 HTTP 200 情况或业务错误)
    }

    if (jsonObj.contains("output")) {
        QJsonObject output = jsonObj["output"].toObject();
        if (output.contains("choices")) {
            QJsonArray choices = output["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                if (firstChoice.contains("message")) {
                    QJsonObject msg = firstChoice["message"].toObject();
                    if (msg.contains("content")) {
                        QJsonArray content = msg["content"].toArray();
                        // Qwen-VL 返回的 content 可能是数组，包含 text
                        QString resultText;
                        for(const auto& item : content) {
                            if(item.toObject().contains("text")) {
                                resultText += item.toObject()["text"].toString();
                            }
                        }
                        emit descriptionGenerated(resultText);
                        return;
                    }
                }
            }
        }
    }

    // 如果解析失败
    if (jsonObj.contains("message")) {
        emit errorOccurred("API 错误: " + jsonObj["message"].toString());
    } else {
        emit errorOccurred("解析响应失败");
    }
}

QString AiManager::imageToBase64(const QImage &image)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QString::fromLatin1(byteArray.toBase64().data());
}
