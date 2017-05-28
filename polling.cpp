#include "polling.h"

using namespace std;
using namespace cv;
struct contour_sorter // 'less' for contours
{
    bool operator ()( const vector<Point>& a, const vector<Point> & b )
    {
        Rect ra(boundingRect(a));
        Rect rb(boundingRect(b));
        // scale factor for y should be larger than img.width
        return ( (ra.x + 10000*ra.y) < (rb.x + 10000*rb.y) );
    }
};

bool compare_rect_h(const Rect & a, const Rect &b) {
    return a.x <= b.x;
}
bool compare_rect_v(const Rect & a, const Rect &b) {
    return a.y <= b.y;
}

QImage Polling::getDoneImage()
{
    cv::Mat temp; // make the same cv::Mat
    cvtColor(img, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    tableImage =  QImage((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    tableImage.bits(); // enforce deep copy, see documentation
    // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return tableImage;
}

void Polling::findAnswersTablePosition(QGraphicsScene &scene)
{
    using namespace cv;
    Size size(2048, 3508); //Scale do 300DPI
    resize(img, img, size);
    Mat gray;

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

    // Show extracted horizontal lines
    imwrite("horizontal.jpg", horizontal);
    // Specify size on vertical axis
    int verticalsize = vertical.rows / scale;

    // Create structure element for extracting vertical lines through morphology operations
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size( 1,verticalsize));

    // Apply morphology operations
    erode(vertical, vertical, verticalStructure, Point(-1, -1));
    dilate(vertical, vertical, verticalStructure, Point(-1, -1));
    //    dilate(vertical, vertical, verticalStructure, Point(-1, -1)); // expand vertical lines

    // Show extracted vertical lines
    imwrite("vertical.jpg", vertical);

    // create a mask which includes the tables
    Mat mask = horizontal + vertical;
    imwrite("mask.jpg", mask);

    // find the joints between the lines of the tables, we will use this information in order to descriminate tables from pictures (tables will contain more than 4 joints while a picture only 4 (i.e. at the corners))
    Mat joints;
    bitwise_and(horizontal, vertical, joints);
    imwrite("joints.jpg", joints);


    Mat blobs;
    bitwise_xor(bw, mask, blobs);
    imwrite("blobs.jpg", blobs);

    // Find external contours from the mask, which most probably will belong to tables or to images
    std::vector<Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0, 0));
    //cv::findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    std::vector<std::vector<Point> > contours_poly( contours.size() );
    std::vector<Rect> boundRect( contours.size() );
    std::vector<Mat> rois;

    for (size_t i = 0; i < contours.size(); i++)
    {

        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );

        // find the number of joints that each table has
        Mat roi = joints(boundRect[i]);

        std::vector<std::vector<Point> > joints_contours;
        findContours(roi, joints_contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

        // if the number is not more than 5 then most likely it not a table
        if(joints_contours.size() <= 4)
            continue;

        rois.push_back(img(boundRect[i]).clone());

        //        drawContours( img, contours, i, Scalar(0, 0, 255), CV_FILLED, 8, std::vector<Vec4i>(), 0, Point() );
        rectangle( img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, i==0?0:255, i==0?255:0), 3, 8, 0 );

        if(i == 0)
        {
            cv::Rect tableBounds(boundRect[i].tl(), boundRect[i].br());
            //croppedAnswersTable = croppedAnswersTable(tableBounds);
        }
    }

    imwrite("contours.jpg", img);

    QImage tempImage = tableImage.convertToFormat(QImage::Format_Mono);


    /*******************************
     *
     *  Read results
     *
     *******************************/
    std::vector<char> wynik;
    std::vector<Rect> newRect;
    int contoursNmb = 0;
    for(unsigned int i = 0; i < boundRect.size(); i++)
    {
        if(contourArea(contours[i]) < 1000 || contourArea(contours[i]) > 2500)
            continue;
        newRect.push_back(boundRect[i]);
    }
    std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    for(unsigned int i=0; i<newRect.size(); i+=15)
    {
        std::sort(newRect.begin()+i, newRect.begin()+i+15, compare_rect_h);
    }
    cout << "Contour: " << endl;
    for(unsigned int i = 0; i < newRect.size(); i++)
    {
        cout << newRect[i];
        int x1 = newRect[i].x;
        int y1 = newRect[i].y;
        int x2 = x1 + newRect[i].width;
        int y2 = y1 + newRect[i].height;
        if(isFieldChecked(tempImage, x1, y1, x2, y2))
        {
            wynik.push_back('T');
            QPoint lt(x2, y1);
            QPoint rb(x1, y2);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(255, 0, 0, 255));
        }
        else
        {
            wynik.push_back('N');
            QPoint lt(x1, y1);
            QPoint rb(x2, y2);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(0, 0, 255, 255));
        }
        contoursNmb++;
    }
    cout << endl << endl;
    for(int i=0; i<contoursNmb; i++)
    {
        if((i)%15 == 0 && i!=0)
        {
            std::cout << std::endl;
        }
        else if((i)%3 == 0 && i!=0)
        {
            std::cout << " ";
        }
        std::cout << wynik[i];
    }
    std::cout << "/////// Liczba kwadratow: " << contoursNmb << " ///////////" << std::endl;
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
    //std::cout << " Black:" << blackCount << " White:" << whiteCount;
    return blackCount>whiteCount/3?true:false;
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
    Size size(2048, 3508);
    resize(img, img, size);
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
