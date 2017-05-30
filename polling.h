#ifndef POLLING_H
#define POLLING_H

#include <QImage>

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QGraphicsScene>
#include <QPoint>

class Polling
{
private:
    cv::Mat templ;
    cv::Mat img;
    cv::Mat mask;
    cv::Mat result;
    std::string pollingToCropPath;
    QImage tableImage;

    std::vector<std::vector<bool>> results;

    bool isFieldChecked(QImage &tempImage, int xPos, int yPos, int xSize, int ySize);
public:
    Polling(const char *pollingToCropPath);
    Polling(std::string tpolling);
    Polling();

    void openPollingImage(std::string filename);
    void resizeImage();
    QImage getDoneImage();
    void findAnswersTablePosition(QGraphicsScene &scene, int xOffset, int yOffset);
};

#endif // POLLING_H
