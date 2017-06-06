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
#include <QGraphicsPixmapItem>
#include <QFileDialog>

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
    void on_p1_button_pressed();

    void on_p2_button_pressed();

    void on_analyse_button_pressed();

    void on_save_results_button_pressed();

private:
    std::string firstPagePath;
    std::string secondPagePath;
    bool loadedFirst = false;
    bool loadedSecond = false;
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QImage image;
    QPixmap pixmap;
    QGraphicsEllipseItem *ellipse;
    Polling polling;
};

#endif // MAINWINDOW_H
