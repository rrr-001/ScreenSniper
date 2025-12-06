#include "ocrresultdialog.h"
#include "i18nmanager.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QScreen>
#include <QFont>
#include <QTimer>

OcrResultDialog::OcrResultDialog(const QString &text, const QString &backend, QWidget *parent)
    : QDialog(parent), m_originalText(text), m_backend(backend)
{
    setupUi();
    applyStyles();

    // 设置文本
    m_textEdit->setPlainText(text);

    // 自动选中全部文本
    m_textEdit->selectAll();
    m_textEdit->setFocus();
}

void OcrResultDialog::setupUi()
{
    setWindowTitle(getLocalizedText("ocr_complete_title", "OCR 识别完成"));

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题标签
    QLabel *titleLabel = new QLabel(getLocalizedText("ocr_result_title", "识别到的文字已复制到剪贴板："), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // 文本编辑框
    m_textEdit = new QTextEdit(this);
    m_textEdit->setMinimumSize(600, 300);
    m_textEdit->setMaximumSize(900, 600);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_textEdit->setAcceptRichText(false);

    // 设置字体 - 使用系统等宽字体
    QFont textFont = m_textEdit->font();
    textFont.setFamily("Menlo");
    textFont.setPointSize(12);
    m_textEdit->setFont(textFont);

    mainLayout->addWidget(m_textEdit);

    // 提示信息（仅对 macOS 原生 API 显示）
    if (m_backend == "Native")
    {
        m_tipLabel = new QLabel(this);
        m_tipLabel->setText(QString("<hr>%1").arg(
            getLocalizedText("ocr_macos_tip",
                             "提示：当前正在使用 macOS 原生 OCR API。\n"
                             "如果需要更高级的 OCR 功能（如更多语言支持），\n"
                             "请安装 Tesseract 库并在项目中进行配置。")));
        m_tipLabel->setWordWrap(true);
        QFont tipFont = m_tipLabel->font();
        tipFont.setPointSize(10);
        m_tipLabel->setFont(tipFont);
        m_tipLabel->setStyleSheet("QLabel { color: #666; padding: 10px 0; }");
        mainLayout->addWidget(m_tipLabel);
    }
    else
    {
        m_tipLabel = nullptr;
    }

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_copyButton = new QPushButton(getLocalizedText("copy_button", "复制"), this);
    m_selectAllButton = new QPushButton(getLocalizedText("select_all_button", "全选"), this);
    m_closeButton = new QPushButton(getLocalizedText("close_button", "关闭"), this);

    m_copyButton->setMinimumWidth(80);
    m_selectAllButton->setMinimumWidth(80);
    m_closeButton->setMinimumWidth(80);

    buttonLayout->addWidget(m_copyButton);
    buttonLayout->addWidget(m_selectAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_copyButton, &QPushButton::clicked, this, &OcrResultDialog::copyToClipboard);
    connect(m_selectAllButton, &QPushButton::clicked, this, &OcrResultDialog::selectAll);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // 设置对话框大小
    adjustSize();

    // 居中显示
    if (parentWidget())
    {
        move(parentWidget()->geometry().center() - rect().center());
    }
    else
    {
        // 获取主屏幕
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen)
        {
            QRect screenGeometry = screen->geometry();
            move(screenGeometry.center() - rect().center());
        }
    }
}

void OcrResultDialog::applyStyles()
{
    // 为对话框设置样式
    QString dialogStyle = R"(
        QDialog {
            background-color: #ffffff;
        }
        QTextEdit {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 8px;
            background-color: #f9f9f9;
            selection-background-color: #0078d7;
            selection-color: white;
        }
        QPushButton {
            background-color: #0078d7;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
        QPushButton:pressed {
            background-color: #004578;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
    )";

    setStyleSheet(dialogStyle);
}

QString OcrResultDialog::getText() const
{
    return m_textEdit->toPlainText();
}

void OcrResultDialog::copyToClipboard()
{
    QString text = m_textEdit->toPlainText();
    if (!text.isEmpty())
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);

        // 临时改变按钮文本以提供反馈
        QString originalText = m_copyButton->text();
        m_copyButton->setText(getLocalizedText("copied_button", "已复制!"));
        m_copyButton->setEnabled(false);

        // 1秒后恢复
        QTimer::singleShot(1000, this, [this, originalText]()
                           {
            m_copyButton->setText(originalText);
            m_copyButton->setEnabled(true); });
    }
}

void OcrResultDialog::selectAll()
{
    m_textEdit->selectAll();
    m_textEdit->setFocus();
}

QString OcrResultDialog::getLocalizedText(const QString &key, const QString &defaultText)
{
    // 使用 I18nManager 获取本地化文本
    I18nManager *i18n = I18nManager::instance();
    if (i18n != nullptr)
    {
        QString text = i18n->getText(key, defaultText);
        if (!text.isEmpty())
        {
            return text;
        }
    }
    return defaultText;
}
