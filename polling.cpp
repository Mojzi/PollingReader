#include "polling.h"

QImage Polling::getDoneImage()
{
     cv::Mat temp; // make the same cv::Mat
     cvtColor(croppedAnswersTable, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
     resultTableImage =  QImage((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     resultTableImage.bits(); // enforce deep copy, see documentation
                          // of QImage::QImage ( const uchar * data, int width, int height, Format format )
     return resultTableImage;
}

void Polling::findAnswersTablePosition()
{
    using namespace cv;
    bool use_mask = 0;
    int match_method = 0;
    /// Source image to display
    img.copyTo( croppedAnswersTable );

    /// Create the result matrix
    int result_cols =  img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );
    /// Do the Matching and Normalize
    bool method_accepts_mask = (CV_TM_SQDIFF == match_method || match_method == CV_TM_CCORR_NORMED);
    if (use_mask && method_accepts_mask)
    { matchTemplate( img, templ, result, match_method, mask); }
    else
    { matchTemplate( img, templ, result, match_method); }

    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );


    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
    else
    { matchLoc = maxLoc; }

    cv::Rect tableBounds(matchLoc.x, matchLoc.y, templ.cols, templ.rows);
    croppedAnswersTable = croppedAnswersTable(tableBounds);

}

bool Polling::isFieldChecked(QImage &tempImage, int xPos, int yPos, int xSize, int ySize)
{
    int whiteCount = 0;
    int blackCount = 0;
    //unsigned char *imgData = tempImage.bits();
    for(int i = xPos; i < xSize; i++)
    {
        for(int j = yPos; j< ySize; j++)
        {
            //if(*(imgData + j + i*ySize) == 1)
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
    std::cout << " Black:" << blackCount << " White:" << whiteCount;
    return blackCount>whiteCount/3?true:false;
}

void Polling::readResults(QGraphicsScene &scene)
{
    QImage tempImage = resultTableImage.convertToFormat(QImage::Format_Mono);
    int imageWidth = tempImage.width();
    int imageHeight = tempImage.height();
    unsigned char *image = tempImage.bits();

    int questionsNumber = 5;
    int answersWidth = 3;
    int tutors = 18;
    int currTutors = 7;
    int answersHeight = 2;

    int squareRows = tutors*answersHeight;
    int squareColumns = questionsNumber * answersWidth;
    int squareNumber = squareRows*squareColumns;

    int squareWidth = (imageWidth / squareColumns) - 6;
    int squareHeight = imageHeight / (squareRows) - 6;
    bool quareTest = false;

    int column = 0;
    int row = 0;
    for(int j = 0, row = 0; row < tutors*2; j+=squareHeight)
    {
        if((row)%2 == 0 && row !=0  && j!=0)
        {
            std::cout << std::endl;
            j+=8;
        }
        else if(j!=0)
            j+=6;
        for(int i = 0, column = 0; column < squareColumns; i+=squareWidth)
        {
            if((column)%3 == 0 && column !=0 && i!=0)
            {
                std::cout << " ";
                i+=8;
            }
            else if(i!=0)
                i+=6;
            if(isFieldChecked(tempImage, i, j, i+squareWidth, j+squareHeight))
            {
                std::cout << "T";
                QPoint lt(i, j);
                QPoint rb(i+squareWidth, j+squareHeight);
                QRect rect(lt, rb);
                scene.addRect(rect, QColor(255, 0, 0, 255));
                quareTest = true;
            }
            else
            {
                std::cout << "N";
                QPoint lt(i, j);
                QPoint rb(i+squareWidth, j+squareHeight);
                QRect rect(lt, rb);
                scene.addRect(rect, QColor(0, 0, 255, 255));
                quareTest = true;
            }
            column++;
        }
        std::cout << std::endl;
        row++;
    }
}

Polling::Polling()
{
}

Polling::Polling(std::string tpolling, std::string tpattern)
{
    pollingToCropPath = tpolling;
    croppingPatternPath = tpattern;
}

void Polling::openPollingImage(std::string filename)
{
    img = imread( filename.c_str(), cv::IMREAD_COLOR );
}
void Polling::openTemplateImage(std::string filename)
{
    templ = imread( filename.c_str(), cv::IMREAD_COLOR );
}
Polling::Polling(const char *tpolling, const char *tpattern)
{
    pollingToCropPath = std::string(tpolling);
    croppingPatternPath = std::string(tpattern);
    img = imread( pollingToCropPath.c_str(), cv::IMREAD_COLOR );
    templ = imread( croppingPatternPath.c_str(), cv::IMREAD_COLOR );
}
