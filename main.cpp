#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序名称
    a.setApplicationName("ScreenSniper");
    a.setOrganizationName("ScreenSniper");

    MainWindow w;
    // 使用国际化的显示名称
    a.setApplicationDisplayName(w.getText("app_display_name", "屏幕截图工具"));
    w.show();

    return a.exec();
}
