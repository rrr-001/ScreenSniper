#ifndef AIMANAGER_H
#define AIMANAGER_H

#include <QObject>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AiManager : public QObject
{
    Q_OBJECT
public:
    explicit AiManager(QObject *parent = nullptr);

    Q_INVOKABLE void generateImageDescription(const QImage &image);

signals:
    void descriptionGenerated(const QString &text);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *m_networkManager;

    QString imageToBase64(const QImage &image);

    void onOpenAIReply(QNetworkReply *reply);
    void onAliyunReply(QNetworkReply *reply);
};

#endif // AIMANAGER_H
