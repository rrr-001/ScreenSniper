#ifndef MACOCR_H
#define MACOCR_H

#include <QString>
#include <QPixmap>

class MacOCR
{
public:
    static QString recognizeText(const QPixmap &pixmap);
};

#endif // MACOCR_H
