#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QPoint>
#include <QColor>

// 绘制形状数据结构
struct DrawnArrow
{
    QPoint start;
    QPoint end;
    QColor color;
    int width;
};

struct DrawnRectangle
{
    QRect rect;
    QColor color;
    int width;
};

class ScreenshotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenshotWidget(QWidget *parent = nullptr);
    ~ScreenshotWidget();

    void startCapture();
    void startCaptureFullScreen(); // 直接截取全屏并显示工具栏

signals:
    void screenshotTaken();
    void screenshotCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupToolbar();
    void updateToolbarPosition();
    void saveScreenshot();
    void copyToClipboard();
    void cancelCapture();
    void drawArrow(QPainter &painter, const QPoint &start, const QPoint &end, const QColor &color, int width);

    QPixmap screenPixmap; // 屏幕截图
    QPoint startPoint;    // 选择起始点
    QPoint endPoint;      // 选择结束点
    bool selecting;       // 是否正在选择
    bool selected;        // 是否已选择完成
    QRect selectedRect;   // 选中的矩形区域

    // 工具栏
    QWidget *toolbar;
    QPushButton *btnSave;
    QPushButton *btnCopy;
    QPushButton *btnCancel;
    QPushButton *btnRect;  // 矩形工具
    QPushButton *btnArrow; // 箭头工具
    QPushButton *btnText;  // 文字工具
    QPushButton *btnPen;   // 画笔工具

    // 尺寸显示标签
    QLabel *sizeLabel;

    // 屏幕设备像素比
    qreal devicePixelRatio;

    // 放大镜相关
    QPoint currentMousePos;
    bool showMagnifier;

    // 绘制相关
    enum DrawMode
    {
        None,
        Rectangle,
        Arrow,
        Text,
        Pen
    };
    DrawMode currentDrawMode;

    // 存储绘制的形状
    QVector<DrawnArrow> arrows;
    QVector<DrawnRectangle> rectangles;

    // 当前绘制的临时数据
    bool isDrawing;
    QPoint drawStartPoint;
    QPoint drawEndPoint;
};

#endif // SCREENSHOTWIDGET_H
