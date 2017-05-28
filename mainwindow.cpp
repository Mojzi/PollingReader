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

    std::string ptemp = "ankietawzor.png";
    polling.openTemplateImage(ptemp);

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

void MainWindow::on_actionOpen_triggered()
{
    //std::string name2 = QFileDialog::getOpenFileName(this).toStdString();
    //polling.getTableCoords();
    //scene->clear();
    //scene->addPixmap(QPixmap::fromImage(polling.getDoneImage()));
    //scene->setSceneRect(image.rect());
    //ui->graphicsView->resetTransform();
    //ui->graphicsView->show();
}

void MainWindow::on_pushButton_2_pressed()
{
    std::string image = QFileDialog::getOpenFileName(this).toStdString();
    polling.openPollingImage(image);
    ui->lineEdit->setText(QString::fromStdString(image));
}

void MainWindow::on_pushButton_pressed()
{
    std::string image = QFileDialog::getOpenFileName(this).toStdString();
    polling.openTemplateImage(image);
}

void MainWindow::on_pushButton_3_pressed()
{
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(polling.getDoneImage()));
    polling.findAnswersTablePosition(*scene);
    scene->setSceneRect(image.rect());
    //polling.readResults(*scene);
    ui->graphicsView->resetTransform();
    ui->graphicsView->show();
}
