#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>
#include <QPixmap>
#include <QImage>
#include <pollingview.h>
#include "polling.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pushButton_2_pressed();

    void on_pushButton_pressed();

    void on_pushButton_3_pressed();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QImage image;
    QPixmap pixmap;
    QGraphicsEllipseItem *ellipse;
    Polling polling;
};

#endif // MAINWINDOW_H
