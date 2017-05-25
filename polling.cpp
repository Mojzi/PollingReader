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
    /*bool use_mask = 0;
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
    croppedAnswersTable = croppedAnswersTable(tableBounds);*/

    //Size size(img.cols/1, img.rows/1);
    Size size(2048, 3508);
    resize(img, img, size);
    img.copyTo( croppedAnswersTable );
    Mat gray;

    if (img.channels() == 3)
    {
        cvtColor(img, gray, CV_BGR2GRAY);
    }
    else
    {
        gray = img;
    }

    // Show gray image
    //imwrite("gray.jpg", gray);

    // Apply adaptiveThreshold at the bitwise_not of gray, notice the ~ symbol
    Mat bw;
    adaptiveThreshold(~gray, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

    // Show binary image
    //imwrite("binary.jpg", bw);

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
    cv::findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    std::vector<std::vector<Point> > contours_poly( contours.size() );
    std::vector<Rect> boundRect( contours.size() );
    std::vector<Mat> rois;

    for (size_t i = 0; i < contours.size(); i++)
    {
        // find the area of each contour
        double area = contourArea(contours[i]);

        //        // filter individual lines of blobs that might exist and they do not represent a table
        if(area < 50) // value is randomly chosen, you will need to find that by yourself with trial and error procedure
            continue;

        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );

        // find the number of joints that each table has
        Mat roi = joints(boundRect[i]);

        //std::string nazwa;
        //nazwa.append("test");
        //nazwa.append(std::to_string(i));
        //nazwa.append(".jpg");
        //imwrite(nazwa.c_str(), roi);

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

    for(size_t i = 0; i < rois.size(); ++i)
    {
        /* Now you can do whatever post process you want
              * with the data within the rectangles/tables. */
        std::string nazwa;
        nazwa.append("roi");
        nazwa.append(std::to_string(i));
        nazwa.append(".jpg");
        cv::Mat detected_edges;
        Canny( rois[i], detected_edges, 0, 0*3, 3 );
        cv::Mat dst;
        detected_edges.convertTo(dst, CV_8U);
        //rois[i].copyTo(dst, detected_edges);
        imwrite(nazwa.c_str(), rois[i]);
        if(i ==0) rois[i].copyTo(croppedAnswersTable);
        waitKey();
    }
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

void Polling::readResults(QGraphicsScene &scene)
{
    using namespace cv;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Mat gray;

    if (croppedAnswersTable.channels() == 3)
    {
        cvtColor(croppedAnswersTable, gray, CV_BGR2GRAY);
    }
    else
    {
        gray = croppedAnswersTable;
    }

    // Apply adaptiveThreshold at the bitwise_not of gray, notice the ~ symbol
    Mat bw;
    adaptiveThreshold(~gray, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

    // Show binary image
    //imwrite("binary.jpg", bw);

    // Create the images that will use to extract the horizonta and vertical lines
    Mat horizontal = bw.clone();
    Mat vertical = bw.clone();

    int scale = 25; // play with this variable in order to increase/decrease the amount of lines to be detected

    // Specify size on horizontal axis
    int horizontalsize = horizontal.cols / scale;

    // Create structure element for extracting horizontal lines through morphology operations
    Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize,1));

    // Apply morphology operations
    erode(horizontal, horizontal, horizontalStructure, Point(-1, -1));
    dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1));
    dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1)); // expand horizontal lines

    // Show extracted horizontal lines
    imwrite("horizontal.jpg", horizontal);
    // Specify size on vertical axis
    int verticalsize = vertical.rows / scale;

    // Create structure element for extracting vertical lines through morphology operations
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size( 1,verticalsize));

    // Apply morphology operations
    erode(vertical, vertical, verticalStructure, Point(-1, -1));
    dilate(vertical, vertical, verticalStructure, Point(-1, -1));
    dilate(vertical, vertical, verticalStructure, Point(-1, -1)); // expand vertical lines

    // Show extracted vertical lines
    imwrite("vertical.jpg", vertical);

    // create a mask which includes the tables
    Mat mask = horizontal + vertical;
    imwrite("mask.jpg", mask);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0, 0));
    std::cout << "Contour size:" << contours.size();
    //for(int i=0; i<contours.size(); i++)
    //{
    //    if(contourArea(contours[i]) < 250 || contourArea(contours[i]) > 750)
    //        contours.erase(contours.begin() + i);
    //}
    //for(std::vector<std::vector<cv::Point> >::iterator it = contours.begin(); it != contours.end(); it++)
    //{
    //    if(contourArea(*it) < 100 || contourArea(*it) > 150)
    //        it = contours.erase(it);
    //}
    std::cout << "\n After Contour size:" << contours.size() << std::endl;
    //std::sort(contours.begin(), contours.end(), contour_sorter());
    //drawContours( croppedAnswersTable, contours, -1, Scalar(0, 0, 255), CV_FILLED, 1, hierarchy);

    //drawContours( img, contours, i, Scalar(0, 0, 255), CV_FILLED, 8, std::vector<Vec4i>(), 0, Point() );
    QImage tempImage = resultTableImage.convertToFormat(QImage::Format_Mono);

    std::vector<std::vector<Point> > contours_poly( contours.size() );
    std::vector<Rect> boundRect( contours.size() );

    for(int i=0; i<contours.size(); i++)
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
    }
    //std::sort(boundRect.begin(), boundRect.end(), compare_rect);
    //int medx = 0;
    //int medy = 0;
    //int s = boundRect.size();
    //if(boundRect.size()%2 == 0)
    //{
    //    medx = (boundRect[s/2].x + boundRect[s/2+1].x)/2;
    //    medy = (boundRect[s/2].y + boundRect[s/2+1].y)/2;
    //}
    //else
    //{
    //    medx = boundRect[s/2+1].x;
    //    medy = boundRect[s/2+1].y;
    //}
    //for(int i = 0; i < boundRect.size(); i++)
    //{
    //    //if(contourArea(contours[i]) < 2500 || contourArea(contours[i]) > 5000)
    //    //if(contourArea(contours[i]) < 250 || contourArea(contours[i]) > 750)
    //    //if(boundRect[i].area() < 250 || boundRect[i].area() > 1000)
    //    if(boundRect[i].x < medx*0.9 || boundRect[i].x > medx*1.1 || boundRect[i].y < medy*0.9 || boundRect[i].y > medy*1.1)
    //        boundRect.erase(boundRect.begin() + i);
    //}
    //std::sort(boundRect.begin(), boundRect.end(), compare_rect);
    //drawContours( croppedAnswersTable, contours, -1, Scalar(0, 0, 255), CV_FILLED, 1, hierarchy);
    imwrite("testcntrs.jpg", croppedAnswersTable);
    std::vector<char> wynik;
    std::vector<Rect> newRect;
    int contoursNmb = 0;
    for(int i = 0; i < boundRect.size(); i++)
    {
        if(contourArea(contours[i]) < 1000 || contourArea(contours[i]) > 2500)
        //if(contourArea(contours[i]) < 250 || contourArea(contours[i]) > 750)
        //if(boundRect[i].area() < 250 || boundRect[i].area() > 1000)
            continue;
        //int x1 = boundRect[i].x;
        //int y1 = boundRect[i].y;
        //int x2 = x1 + boundRect[i].width;
        //int y2 = y1 + boundRect[i].height;

        //if(isFieldChecked(tempImage, x1, y1, x2, y2))
        //{
        //    //std::cout << "T";
        //    wynik.push_back('T');
        //    QPoint lt(x2, y1);
        //    QPoint rb(x1, y2);
        //    QRect rect(lt, rb);
        //    scene.addRect(rect, QColor(255, 0, 0, 255));
        //    //rectangle(croppedAnswersTable, boundRect[i], Scalar(0,0,255), 3);
        //}
        //else
        //{
        //    //std::cout << "N";
        //    wynik.push_back('N');
        //    QPoint lt(x1, y1);
        //    QPoint rb(x2, y2);
        //    QRect rect(lt, rb);
        //    scene.addRect(rect, QColor(0, 0, 255, 255));
        //    //rectangle(croppedAnswersTable, boundRect[i], Scalar(255,0,0), 3);
        //}
        //contoursNmb++;
        newRect.push_back(boundRect[i]);
    }
    //std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    //std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    //std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    //std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    std::sort(newRect.begin(), newRect.end(), compare_rect_v);
    for(int i=0; i<newRect.size(); i+=15)
    {
        std::sort(newRect.begin()+i, newRect.begin()+i+15, compare_rect_h);
    }
    cout << "Contour: " << endl;
    for(int i = 0; i < newRect.size(); i++)
    {
        cout << newRect[i];
        //if(contourArea(contours[i]) < 250 || contourArea(contours[i]) > 750)
        //if(boundRect[i].area() < 250 || boundRect[i].area() > 1000)
        int x1 = newRect[i].x;
        int y1 = newRect[i].y;
        int x2 = x1 + newRect[i].width;
        int y2 = y1 + newRect[i].height;
        if(isFieldChecked(tempImage, x1, y1, x2, y2))
        {
            //std::cout << "T";
            wynik.push_back('T');
            QPoint lt(x2, y1);
            QPoint rb(x1, y2);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(255, 0, 0, 255));
            //rectangle(croppedAnswersTable, boundRect[i], Scalar(0,0,255), 3);
        }
        else
        {
            //std::cout << "N";
            wynik.push_back('N');
            QPoint lt(x1, y1);
            QPoint rb(x2, y2);
            QRect rect(lt, rb);
            scene.addRect(rect, QColor(0, 0, 255, 255));
            //rectangle(croppedAnswersTable, boundRect[i], Scalar(255,0,0), 3);
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
    /*int imageWidth = tempImage.width();
    int imageHeight = tempImage.height();
    unsigned char *image = tempImage.bits();

    int questionsNumber = 5;
    int answersWidth = 3;
    //int tutors = 18;
    int tutors = imageHeight/200;
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
    }*/
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
