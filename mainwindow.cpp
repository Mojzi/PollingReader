#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

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
    polling.clearAnswers();
    if(loadedFirst)
    {
        if (!polling.loadImage(firstPagePath))
            return;
        scene->clear();
        polling.normalizeImageSize();
        scene->setSceneRect(image.rect());
        scene->addPixmap(QPixmap::fromImage(polling.fromMatToQImage()));
        polling.analyzeImage(*scene, 0, 0);
        ui->graphicsView->resetTransform();
        ui->graphicsView->show();
    }
    if(loadedSecond)
    {
        if(!polling.loadImage(secondPagePath))
            return;
        //scene->clear();
        polling.normalizeImageSize();
        //scene->setSceneRect(image.rect());
        scene->setSceneRect(0,0,10000,10000);
        QGraphicsPixmapItem *second = scene->addPixmap(QPixmap::fromImage(polling.fromMatToQImage()));
        second->setOffset(2050, 0);
        polling.analyzeImage(*scene, 2050, 0);
        //ui->graphicsView->resetTransform();
        ui->graphicsView->show();
    }

}

void MainWindow::on_save_results_button_pressed()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save results"),tr(".txt"));
    polling.writeAnswersToFile(fileName);
}
