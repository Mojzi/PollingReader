#include "polling.h"

using namespace cv;

// For sorting contours with std::sort
bool compare_rect_h(const Rect & a, const Rect &b) {
    return a.x <= b.x;
}
bool compare_rect_v(const Rect & a, const Rect &b) {
    return a.y <= b.y;
}

QImage Polling::fromMatToQImage()
{
    cv::Mat temp; // make the same cv::Mat
    cvtColor(img, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    tableImage =  QImage((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    tableImage.bits(); // enforce deep copy, see documentation
                       // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return tableImage;
}

void Polling::analyzeImage(QGraphicsScene &scene, int xOffset, int yOffset)
{
    using namespace cv;
    Mat gray;

    // Convert to gray image
    if (img.channels() == 3)
    {
        cvtColor(img, gray, CV_BGR2GRAY);
    }
    else
    {
        gray = img;
    }

    // Apply adaptiveThreshold at the bitwise_not of gray, notice the ~ symbol
    Mat bw;
    adaptiveThreshold(~gray, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

    // Create the images that will use to extract the horizonta and vertical lines
    Mat horizontal = bw.clone();
    Mat vertical = bw.clone();

    int scale = 15; // play with this variable in order to increase/decrease the amount of lines to be detected

    // Specify size on horizontal axis
    int horizontalsize = horizontal.cols / scale;

    // Create structure element for extracting horizontal lines through morphology operations
    Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize,1));

    // Apply morphology operations
    erode(horizontal, horizontal, horizontalStructure, Point(-1, -1));
    dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1));
    //    dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1)); // expand horizontal lines

    // Specify size on vertical axis
    int verticalsize = vertical.rows / scale;

    // Create structure element for extracting vertical lines through morphology operations
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size( 1,verticalsize));

    // Apply morphology operations
    erode(vertical, vertical, verticalStructure, Point(-1, -1));
    dilate(vertical, vertical, verticalStructure, Point(-1, -1));

    // create a mask which includes the tables
    Mat mask = horizontal + vertical;

    // find the joints between the lines of the tables, we will use this information in order to descriminate tables from pictures (tables will contain more than 4 joints while a picture only 4 (i.e. at the corners))
    Mat joints;
    bitwise_and(horizontal, vertical, joints);


    Mat blobs;
    bitwise_xor(bw, mask, blobs);

    // Find external contours from the mask, which most probably will belong to tables or to images
    std::vector<Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0, 0));

    std::vector<std::vector<Point> > contours_poly( contours.size() );
    std::vector<Rect> boundRect( contours.size() );
    std::vector<Mat> rois;

    for (size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
    }

    QImage tempImage = tableImage.convertToFormat(QImage::Format_Mono);


    /*******************************
     *
     *  Read results
     *
     *******************************/
    std::vector<Rect> newRect;
    int minContourArea = 1000;
    int maxContourArea = 2500;

    // Throw away all contours that are not answer fields
    for(unsigned int i = 0; i < boundRect.size(); i++)
    {
        if(contourArea(contours[i]) < minContourArea || contourArea(contours[i]) > maxContourArea)
            continue;
        newRect.push_back(boundRect[i]);
    }

    // Dunno why, but this combination of vertical/horizontal sorts seem to work
    std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    for(unsigned int i=0; i<newRect.size(); i+=15)
    {
        std::sort(newRect.begin()+i, newRect.begin()+i+15, compare_rect_h);
    }

    // Draw found answer fields on screen and mark them if checked
    for(unsigned int i = 0; i < newRect.size(); i++)
    {
        int x1 = newRect[i].x;
        int y1 = newRect[i].y;
        int x2 = x1 + newRect[i].width;
        int y2 = y1 + newRect[i].height;
        if(isFieldChecked(tempImage, x1, y1, x2, y2))
        {
            results.push_back(true);
            QPoint lt(x2+xOffset, y1+yOffset);
            QPoint rb(x1+xOffset, y2+yOffset);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(255, 0, 0, 255));
        }
        else
        {
            results.push_back(false);
            QPoint lt(x1+xOffset, y1+yOffset);
            QPoint rb(x2+xOffset, y2+yOffset);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(0, 0, 255, 255));
        }
    }
}

bool Polling::writeAnswersToFile(QString filename)
{
   QFile output(filename);
   if(!output.open(QIODevice::WriteOnly))
       return false;
    QTextStream stream(&output);
   for(unsigned int i=0; i<results.size(); i++)
   {
       if(i%15 == 0 && i!=0)
           stream << "\n";
       stream << (results[i]?"1":"0");
   }
   output.close();
   return true;
}

void Polling::clearAnswers()
{
    results.clear();
}

bool Polling::isFieldChecked(QImage &tempImage, int xPos, int yPos, int xSize, int ySize)
{
    int whiteCount = 0;
    int blackCount = 0;
    for(int i = xPos; i < xSize; i++)
    {
        for(int j = yPos; j< ySize; j++)
        {
            if(tempImage.width() < i+1 || tempImage.height() < j+1)
                whiteCount++;
            else
            {
                QColor c = QColor::fromRgb(tempImage.pixel(i, j));
                if(c == Qt::black)
                    blackCount++;
                else
                    whiteCount++;
            }
        }
    }
    return blackCount>whiteCount/3?true:false;
}



Polling::Polling()
{
}

bool Polling::loadImage(std::string filename)
{
    img = imread( filename.c_str(), cv::IMREAD_COLOR );
    if(img.empty())
        return false;
    if(img.cols == 0 || img.rows == 0)
        return false;
    else return true;
}

void Polling::normalizeImageSize()
{
    Size size(2048, 3508); //Scale do 300DPI
    resize(img, img, size);
}
