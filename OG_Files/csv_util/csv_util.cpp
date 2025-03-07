#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem> // Include for filesystem functions
#include "csv_util.h"
#include <opencv2/opencv.hpp>
#include <algorithm>

// ... (rest of the functions as you provided them)
std::vector<float> extractCenterPatch(const cv::Mat& img) {
    std::vector<float> features;
    cv::Mat gray;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = img.clone();
    }
    int startY = (gray.rows - 7) / 2;
    int startX = (gray.cols - 7) / 2;
    for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 7; x++) {
            features.push_back(gray.at<uchar>(startY + y, startX + x));
        }
    }
    return features;
}



void process_images(const std::string& dir_path, const std::string& csv_path) {
    std::ofstream file_out(csv_path);
    if (!file_out.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) { // Use std::filesystem
        if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
            std::string full_path = entry.path().string();
            cv::Mat img = cv::imread(full_path);

            if (img.empty()) {
                std::cerr << "Error reading image: " << full_path << std::endl;
                continue;
            }

            std::vector<float> features = extractCenterPatch(img); // Make sure this is defined
            file_out << entry.path().filename().string(); // Use filename() for just the name

            for (float f : features) {
                file_out << "," << f;
            }
            file_out << "\n";
        }
    }
    file_out.close();
}