#include "scanner.h"

#include <math.h>

namespace scanner {

    static double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 )
    {
        double dx1 = pt1.x - pt0.x;
        double dy1 = pt1.y - pt0.y;
        double dx2 = pt2.x - pt0.x;
        double dy2 = pt2.y - pt0.y;
        return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
    }

    static double compare_squares(std::vector<cv::Point> sq1, std::vector<cv::Point> sq2)
    {
        // very simple comparision

        if (sq1.size()<4) return false;
        if (sq2.size()<4) return true;

        int deltaX1 = std::max(sq1[0].x,std::max(sq1[1].x,std::max(sq1[2].x,sq1[3].x))) - std::min(sq1[0].x,std::min(sq1[1].x,std::min(sq1[2].x,sq1[3].x)));
        int deltaY1 = std::max(sq1[0].y,std::max(sq1[1].y,std::max(sq1[2].y,sq1[3].y))) - std::min(sq1[0].y,std::min(sq1[1].y,std::min(sq1[2].y,sq1[3].y)));

        int deltaX2 = std::max(sq2[0].x,std::max(sq2[1].x,std::max(sq2[2].x,sq2[3].x))) - std::min(sq2[0].x,std::min(sq2[1].x,std::min(sq2[2].x,sq2[3].x)));
        int deltaY2 = std::max(sq2[0].y,std::max(sq2[1].y,std::max(sq2[2].y,sq2[3].y))) - std::min(sq2[0].y,std::min(sq2[1].y,std::min(sq2[2].y,sq2[3].y)));

        return deltaX1*deltaY1 > deltaX2*deltaY2;
    }
    
    static std::vector<cv::Point2f> convert_to_float(const std::vector<cv::Point>& sq) {
        std::vector<cv::Point2f> fSq;
        
        for(int i=0; i<sq.size();i++)
        {
            fSq.push_back(cv::Point2f(sq[i].x,sq[i].y));
        }
        
        return fSq;
    }
    
    
    // credit for this method goes to http://stackoverflow.com/questions/13269432/perspective-transform-crop-in-ios-with-opencv
    std::vector<cv::Point2f> find_square(const cv::Mat& image)
    {
        
        std::vector<cv::Point> largestSquare;

        // blur will enhance edge detection
        cv::Mat blurred;
        cv::medianBlur(image, blurred, 9);

        cv::Mat gray0(blurred.size(), CV_8U), gray;
        std::vector<std::vector<cv::Point> > contours;

        // find squares in every color plane of the image
        for (int c = 0; c < 3; c++)
        {
            int ch[] = {c, 0};
            cv::mixChannels(&blurred, 1, &gray0, 1, ch, 1);

            // try several threshold levels
            const int threshold_level = 2;
            for (int l = 0; l < threshold_level; l++)
            {
                // Use Canny instead of zero threshold level!
                // Canny helps to catch squares with gradient shading
                if (l == 0)
                {
                    cv::Canny(gray0, gray, 10, 20, 3);

                    // Dilate helps to remove potential holes between edge segments
                    cv::dilate(gray, gray, cv::Mat(), cv::Point(-1, -1));
                }
                else
                {
                    gray = gray0 >= (l + 1) * 255 / threshold_level;
                }

                // Find contours and store them in a list
                cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

                // Test contours
                std::vector<cv::Point> approx ;
                for (size_t i = 0; i < contours.size(); i++)
                {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                    if (approx.size() == 4 && fabs(cv::contourArea(cv::Mat(approx))) > 1000 && cv::isContourConvex(cv::Mat(approx)))
                    {
                        double maxCosine = 0;

                        for (int j = 2; j < 5; j++)
                        {
                            double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                            maxCosine = MAX(maxCosine, cosine);
                        }

                        if (maxCosine < 0.3)
                        {
                            // found a candidate

                            if (compare_squares(approx,largestSquare))
                            {
                                // the current square is the greatest one
                                largestSquare = approx;
                            }
                        }

                    }
                }
            }
        }

        return convert_to_float(largestSquare);
    }
    
    static std::vector<cv::Point2f> sort_corners(const std::vector<cv::Point2f>& corners)
    {
        cv::Point2f center(0,0);
        for (int i = 0; i < corners.size(); i++)
            center += corners[i];
            
        center *= (1. / corners.size());


        std::vector<cv::Point2f> top, bot;

        for (int i = 0; i < corners.size(); i++)
        {
            if (corners[i].y < center.y)
                top.push_back(corners[i]);
            else
                bot.push_back(corners[i]);
        }

        cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
        cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
        cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
        cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

        std::vector<cv::Point2f> res;
        res.push_back(tl);
        res.push_back(tr);
        res.push_back(br);
        res.push_back(bl);
        
        return res;
    }
    
    cv::Mat transform(const cv::Mat& image, const std::vector<cv::Point2f>& rect)
    {
        // source corners
        std::vector<cv::Point2f> src = sort_corners(rect);
        
        // destination image
        float w1 = sqrt( pow(src[2].x - src[3].x , 2) + pow(src[2].x - src[3].x, 2));
        float w2 = sqrt( pow(src[1].x - src[0].x , 2) + pow(src[1].x - src[0].x, 2));

        float h1 = sqrt( pow(src[1].y - src[2].y , 2) + pow(src[1].y - src[2].y, 2));
        float h2 = sqrt( pow(src[0].y - src[3].y , 2) + pow(src[0].y - src[3].y, 2));

        float maxWidth = (w1 < w2) ? w1 : w2;
        float maxHeight = (h1 < h2) ? h1 : h2;
        
        cv::Mat result = cv::Mat::zeros(maxHeight,maxWidth, CV_8UC3);
        
        // destination corners
        std::vector<cv::Point2f> dest;
        dest.push_back(cv::Point2f(0, 0));
        dest.push_back(cv::Point2f(result.cols, 0));
        dest.push_back(cv::Point2f(result.cols, result.rows));
        dest.push_back(cv::Point2f(0, result.rows));
                
        // transformation matrix
        cv::Mat transmtx = cv::getPerspectiveTransform(src, dest);
                
        // perspective transformation
        cv::warpPerspective(image, result, transmtx, result.size());
        
        return result;
    }
    
    void adjust(cv::Mat& image, int mode)
    {
        /*
         * 0 no filter
         * 1 text only
         * 2 photo and text
         * 3 photo only
         */
        
        if (mode==1) {
            cv::cvtColor( image, image, CV_RGB2GRAY );
            cv::GaussianBlur(image, image, cvSize(11,11), 0);
            cv::adaptiveThreshold(image, image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 7, 1.2); // 5,2; 7, 1.2
        }
        else if (mode==2) {
            cv::cvtColor( image, image, CV_RGB2GRAY );
                            
            image.convertTo(image, -1, 1.4, -50);
        }
        else if (mode==3) {
            image.convertTo(image, -1, 1.9, -80);
        }
        
        
        // TODO
        /*
        cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size2i(5,5));
        cv::morphologyEx(image,image,cv::MORPH_CLOSE,kernel1);
        //cv::normalize(image,image,0,255,cv::NORM_MINMAX);
         */
        
        // TODO shading korrektur
        // TODO threshold
        
        /*
        
        double alpha = 1.2; // contrast (1.0-3.0)
        int beta = 35;    // brightness (0-100)
        
        
        // apply image = alpha*image + beta
        for (int y = 0; y < image.rows; y++)
        {
            for (int x = 0; x < image.cols; x++)
            {
                for (int c = 0; c < 3; c++)
                {
                    image.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(alpha * (image.at<cv::Vec3b>(y, x)[c]) + beta);
                }
            }
        }
        */
    }
    
    cv::Mat process(const cv::Mat& image, int mode)
    {
        std::vector<cv::Point2f> square = find_square(image);
        
        if (square.size()<4) return image;
        
        cv::Mat transformed = transform(image,square);
        adjust(transformed,mode);
        
        return transformed;
    }

}