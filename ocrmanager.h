#ifndef OCRMANAGER_H
#define OCRMANAGER_H

#include <QString>
#include <QPixmap>
#include <QMutex>
#include <QCache>

// Tesseract前向声明
#ifdef USE_TESSERACT
namespace tesseract
{
    class TessBaseAPI;
}
#endif

class OcrManager
{
public:
    static QString recognize(const QPixmap &pixmap);
    static QString getBackendType();

    // 异步识别接口
    static void recognizeAsync(const QPixmap &pixmap, std::function<void(const QString &)> callback);

    // 清理资源
    static void cleanup();

    // 设置OCR参数
    static void setLanguage(const QString &language);
    static void setImageScaleFactor(double factor);

private:
    // 私有构造函数，防止外部实例化
    OcrManager();

    // 私有析构函数
    ~OcrManager();

    // 单例获取
    static OcrManager *instance();

    // 初始化OCR引擎
    bool initializeOcr();

    // 实际的OCR识别方法
    QString doRecognize(const QPixmap &pixmap);

    // 图像预处理
    QImage preprocessImage(const QImage &image);

    // Tesseract相关
#ifdef USE_TESSERACT
    tesseract::TessBaseAPI *m_tesseractApi;
    bool m_tesseractInitialized;
#endif

    // 配置参数
    QString m_language;
    double m_imageScaleFactor;

    // 缓存，使用图像哈希值作为键
    QCache<uint, QString> m_resultCache;

    // 互斥锁，确保线程安全
    QMutex m_mutex;

    // 单例实例
    static OcrManager *m_instance;
    static QMutex m_instanceMutex;
};

#endif // OCRMANAGER_H
