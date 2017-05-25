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
    cv::Mat croppedAnswersTable;

    std::string croppingPatternPath;
    std::string pollingToCropPath;

    QImage resultTableImage;
    bool isFieldChecked(QImage &tempImage, int xPos, int yPos, int xSize, int ySize);
public:
    Polling(const char *pollingToCropPath, const char *croppingPatternPath);
    Polling(std::string tpolling, std::string tpattern);
    Polling();

    void openPollingImage(std::string filename);
    void openTemplateImage(std::string filename);
    QImage getDoneImage();
    void findAnswersTablePosition();
    void readResults(QGraphicsScene &scene);
};

#endif // POLLING_H
