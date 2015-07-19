#ifndef SCANNER_H
#define	SCANNER_H

#include <vector>
#include <opencv2/opencv.hpp>


namespace scanner {
    std::vector<cv::Point2f> find_square(const cv::Mat& image);
    cv::Mat transform(const cv::Mat& image, const std::vector<cv::Point2f>&);
    void adjust(cv::Mat& image, int mode);
    
    cv::Mat process(const cv::Mat& image, int mode);
};

#endif	/* SCANNER_H */

