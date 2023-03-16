#include "mainwindow.h"
#include <QApplication>
/*
这个代码是用来模拟chrome里的线程消息通信

*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
