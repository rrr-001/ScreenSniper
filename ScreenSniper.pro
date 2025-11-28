QT       += core gui widgets


win32 {
    LIBS += -lPsapi -lDwmapi
}
macx {
    LIBS += -framework CoreGraphics
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# OpenCV 配置 - 跨平台版本
win32 {
    # Windows 配置
    INCLUDEPATH += "D:/C++/opencv/build/include"
    DEPENDPATH += "D:/C++/opencv/build/include"

    # 显式添加 opencv2 子目录
    INCLUDEPATH += "D:/C++/opencv/build/include/opencv2"
    DEPENDPATH += "D:/C++/opencv/build/include/opencv2"

    LIBS += -lPsapi -lDwmapi
    LIBS += -L"D:/C++/opencv/build/x64/vc16/lib"

    CONFIG(debug, debug|release) {
        LIBS += -lopencv_world4120d
        DEFINES += OPENCV_DEBUG
    } else {
        LIBS += -lopencv_world4120
    }
}

macx {
    # macOS 配置 - 请根据您的实际路径修改
    # 通常使用 Homebrew 安装的 OpenCV 路径
    INCLUDEPATH += /usr/local/include/opencv4
    DEPENDPATH += /usr/local/include/opencv4

    # 显式添加 opencv2 子目录
    INCLUDEPATH += /usr/local/include/opencv4/opencv2
    DEPENDPATH += /usr/local/include/opencv4/opencv2

    LIBS += -framework CoreGraphics
    LIBS += -L/usr/local/lib

    CONFIG(debug, debug|release) {
        LIBS += -lopencv_world4120d
        DEFINES += OPENCV_DEBUG
    } else {
        LIBS += -lopencv_world4120
    }

    # 或者如果使用 pkg-config（推荐）
    # CONFIG += link_pkgconfig
    # PKGCONFIG += opencv4
}



# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    pinwidget.cpp \
    screenshotwidget.cpp \
    watermark_robust.cpp

HEADERS += \
    mainwindow.h \
    pinwidget.h \
    screenshotwidget.h \
    watermark_robust.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
