#include "mainwindow.h"
#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::cout << QCoreApplication::applicationDirPath().toStdString() << std::endl;
    MainWindow w;
    w.show();

    return a.exec();
}
