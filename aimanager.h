#ifndef AIMANAGER_H
#define AIMANAGER_H

#include <QObject>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QBuffer>

class AiManager : public QObject
{
    Q_OBJECT
public:
    explicit AiManager(QObject *parent = nullptr);

    // 传入截图，发起请求
    void generateImageDescription(const QImage &image);

signals:
    // 请求成功，返回描述文本
    void descriptionGenerated(QString text);
    // 请求失败，返回错误信息
    void errorOccurred(QString errorMsg);

private slots:
    void onNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString imageToBase64(const QImage &image);
};

#endif // AIMANAGER_H
