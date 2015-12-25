#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "scanner.h"

static void drawSquare( cv::Mat& image, const std::vector<cv::Point2f>& square )
{
    std::vector<cv::Point> ptx;
    for (int i=0;i<square.size();i++)
        ptx.push_back(cv::Point(square[i].x,square[i].y));

    const cv::Point * p = &ptx[0];

    int n = (int)square.size();
    cv::polylines(image, &p, &n, 1, true, cv::Scalar(0,255,0), 3, 8);

}


int main(int argc, char** argv)
{
    for( int i = 1; i < argc; i++ )
    {
        cv::Mat image = cv::imread(argv[i], 1);
        if( image.empty() )
        {
            std::cout << "Couldn't load " << argv[i] << std::endl;
            continue;
        }

        char previewName[100];
        strcpy(previewName,argv[i]);
        strcat(previewName," (Preview)");

        // preview
        std::vector<cv::Point2f> square = scanner::find_square(image);
        cv::Mat copy = image.clone();
        drawSquare(copy, square);

        cv::namedWindow( previewName, cv::WINDOW_NORMAL );
        cv::imshow(previewName, copy);

        // process
        cv::Mat processed = scanner::process(image,1);

        cv::namedWindow( argv[i], cv::WINDOW_NORMAL );
        cv::imshow(argv[i], processed);
    }

    cv::waitKey();

    return 0;
}
