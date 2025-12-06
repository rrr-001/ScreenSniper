#ifndef OCRRESULTDIALOG_H
#define OCRRESULTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class OcrResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OcrResultDialog(const QString &text, const QString &backend = "Native", QWidget *parent = nullptr);
    QString getText() const;

private slots:
    void copyToClipboard();
    void selectAll();

private:
    void setupUi();
    void applyStyles();
    QString getLocalizedText(const QString &key, const QString &defaultText);

    QTextEdit *m_textEdit;
    QPushButton *m_copyButton;
    QPushButton *m_selectAllButton;
    QPushButton *m_closeButton;
    QLabel *m_tipLabel;
    QString m_originalText;
    QString m_backend;
};

#endif // OCRRESULTDIALOG_H
