#include <QApplication>
#include "jamshellwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    JamShellWindow w;
    w.show();
    return app.exec();
}
