// csv_utils.h
#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

int append_image_data_csv(const char* filename, const char* image_filename, std::vector<float>& image_data, int reset_file = 0);
int read_image_data_csv(const char* filename, std::vector<std::string>& filenames, std::vector<std::vector<float>>& data, int echo_file = 0);

int sobelY3x3(cv::Mat& src, cv::Mat& dst);

int sobelX3x3(cv::Mat& src, cv::Mat& dst);


#endif