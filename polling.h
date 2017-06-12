#ifndef POLLING_H
#define POLLING_H

#include <QImage>

#include <string>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QGraphicsScene>
#include <QPoint>
#include <QFile>
#include <QTextStream>

class Polling
{
private:
    cv::Mat templ;
    cv::Mat img;
    cv::Mat mask;
    cv::Mat result;
    QImage tableImage;

    std::vector<bool> results;

    bool isFieldChecked(QImage &tempImage, int xPos, int yPos, int xSize, int ySize);
public:
    Polling();

    bool loadImage(std::string filename);
    void normalizeImageSize();
    QImage fromMatToQImage();
    void analyzeImage(QGraphicsScene &scene, int xOffset, int yOffset);
    bool writeAnswersToFile(QString filename,QString separator);
    void clearAnswers();
};

#endif // POLLING_H
