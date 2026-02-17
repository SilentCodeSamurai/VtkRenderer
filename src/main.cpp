#include <QApplication>

#include "App.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    const QString initialFile = (argc >= 2) ? QString::fromLocal8Bit(argv[1]) : QString();
    App window(initialFile);
    window.show();
    return app.exec();
}
