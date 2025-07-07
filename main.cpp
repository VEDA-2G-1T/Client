#include <QApplication>
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("QtClientSSN Camera Monitoring System");
    app.setApplicationVersion("1.0");

    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
