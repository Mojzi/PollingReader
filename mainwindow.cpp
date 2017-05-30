#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>
#include "polling.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    scene = new QGraphicsScene(this);
    scene->setSceneRect(image.rect());
    ui->graphicsView->scale(.1, .1);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_p1_button_pressed()
{

    firstPagePath = QFileDialog::getOpenFileName(this).toStdString();

    if(!firstPagePath.empty())
        loadedFirst = true;
    else
        loadedFirst = false;

    ui->p1_line->setText(QString::fromStdString(firstPagePath));
}

void MainWindow::on_p2_button_pressed()
{
    secondPagePath = QFileDialog::getOpenFileName(this).toStdString();
    //polling.openPollingImage(image);
    if(!secondPagePath.empty())
        loadedSecond = true;
    else
        loadedSecond = false;
    ui->p2_line->setText(QString::fromStdString(secondPagePath));
}

void MainWindow::on_analyse_button_pressed()
{
    int offset;
    if(loadedFirst)
    {
        polling.openPollingImage(firstPagePath);
        scene->clear();
        polling.resizeImage();
        scene->setSceneRect(image.rect());
        QGraphicsPixmapItem *first = scene->addPixmap(QPixmap::fromImage(polling.getDoneImage()));
        polling.findAnswersTablePosition(*scene, 0, 0);
        ui->graphicsView->resetTransform();
        ui->graphicsView->show();
    }
    if(loadedSecond)
    {
        polling.openPollingImage(secondPagePath);
        //scene->clear();
        polling.resizeImage();
        //scene->setSceneRect(image.rect());
        scene->setSceneRect(0,0,10000,10000);
        QGraphicsPixmapItem *second = scene->addPixmap(QPixmap::fromImage(polling.getDoneImage()));
        second->setOffset(2050, 0);
        polling.findAnswersTablePosition(*scene, 2050, 0);
        //ui->graphicsView->resetTransform();
        ui->graphicsView->show();
    }
}
