#include "macocr.h"
#include <QImage>
#include <QBuffer>
#include <QDebug>

#ifdef Q_OS_MAC
#import <Vision/Vision.h>
#import <AppKit/AppKit.h>
#endif

QString MacOCR::recognizeText(const QPixmap& pixmap) {
#ifdef Q_OS_MAC
    if (pixmap.isNull()) {
        return QString();
    }

    QImage image = pixmap.toImage();
    // Convert QImage to CGImage via NSData/NSImage to handle formats correctly
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    
    NSData* nsData = [NSData dataWithBytes:byteArray.constData() length:byteArray.size()];
    NSImage* nsImage = [[NSImage alloc] initWithData:nsData];
    
    if (!nsImage) {
        return QString();
    }
    
    CGImageRef cgImage = [nsImage CGImageForProposedRect:nil context:nil hints:nil];
    if (!cgImage) {
        return QString();
    }

    __block QString resultText;
    
    if (@available(macOS 10.15, *)) {
        VNRecognizeTextRequest *request = [[VNRecognizeTextRequest alloc] initWithCompletionHandler:^(VNRequest *request, NSError *error) {
            if (error) {
                qDebug() << "OCR Error:" << QString::fromNSString(error.localizedDescription);
                return;
            }
            
            NSMutableString *fullText = [NSMutableString string];
            for (VNRecognizedTextObservation *observation in request.results) {
                NSArray<VNRecognizedText *> *topCandidates = [observation topCandidates:1];
                if (topCandidates.count > 0) {
                    [fullText appendString:topCandidates[0].string];
                    [fullText appendString:@"\n"];
                }
            }
            resultText = QString::fromNSString(fullText).trimmed();
        }];
        
        request.recognitionLevel = VNRequestTextRecognitionLevelAccurate;
        request.usesLanguageCorrection = YES;
        request.recognitionLanguages = @[@"zh-Hans", @"en-US"]; // Prioritize Chinese and English
        
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCGImage:cgImage options:@{}];
        NSError *error = nil;
        [handler performRequests:@[request] error:&error];
        
        if (error) {
            qDebug() << "OCR Handler Error:" << QString::fromNSString(error.localizedDescription);
        }
    } else {
        resultText = "OCR requires macOS 10.15 or later.";
    }
    
    return resultText;
#else
    return QString("OCR is only supported on macOS.");
#endif
}
