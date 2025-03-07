/*
Author: Priyanshu Ranka
Semester : Spring 2025
Subject : PRCV
Task_3: Multi-Histogram Matching
Description: Uses multiple histograms per image and a custom weighted similarity metric.
*/

// Header files
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <opencv2/opencv.hpp>

// Namespace
using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Function to compute a 2D RGB histogram
Mat computeHistogram(const Mat& image, Rect region, int bins = 8) 
{
	// Variables
    Mat hist;
    Mat roi = image(region);  // Extract region of interest (ROI)

	int histSize[] = { bins, bins, bins };   // Number of bins
	float range[] = { 0, 256 };              // Range of pixel values
    const float* histRange[] = { range, range, range };
	int channels[] = { 0, 1, 2 };			 // Channels

    calcHist(&roi, 1, channels, Mat(), hist, 3, histSize, histRange, true, false);
    normalize(hist, hist, 1, 0, NORM_L1);    // Normalize the histogram (for histogram intersection)

    return hist;
}

// Function to compute histogram intersection between two histograms
double histogramIntersection(const Mat& hist1, const Mat& hist2) 
{
    return sum(min(hist1, hist2))[0]; // Sum of minimum bin values
}

// Function to compute a weighted similarity score using two histograms
double computeMultiHistogramSimilarity(const Mat& hist1a, const Mat& hist1b, const Mat& hist2a, const Mat& hist2b, double weight1 = 0.5, double weight2 = 0.5) 
{
	double score1 = histogramIntersection(hist1a, hist2a); // Histogram intersection score (Lower 2/3rd part)
	double score2 = histogramIntersection(hist1b, hist2b); // Histogram intersection score (Upper 2/3rd part)
	return (weight1 * score1 + weight2 * score2);          // Weighted average (Equal)
}

// Main function
int main(int argc, char* argv[]) 
{
    if (argc != 2) 
    {
        cerr << "Usage: " << argv[0] << " <imagePath>" << endl;
        return 1;
    }

    string targetImagePath = argv[1];
	string databaseDirectory = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus"; // Hardcoded database directory
    int N = 3;  // Top N matches (3 required)

    // Load target image
    Mat target_image = imread(targetImagePath, IMREAD_COLOR);
    if (target_image.empty()) {
        cerr << "Error: Could not open target image." << endl;
        return 1;
    }

	// Get dimensions of target image for histogram computation
    int height = target_image.rows;
    int width = target_image.cols;

    // Compute histograms for Upper 2/3 and Lower 2/3
    Mat targetHistUpper = computeHistogram(target_image, Rect(0, 0, width, (2 * height) / 3));
    Mat targetHistLower = computeHistogram(target_image, Rect(0, height / 3, width, (2 * height) / 3));

	vector<pair<double, string>> similarities;  // Vector to store similarity scores

    // Iterate through database images
    try 
    {
        for (const auto& entry : fs::directory_iterator(databaseDirectory)) 
        {
            if (entry.is_regular_file()) 
            {
                string filename = entry.path().filename().string();
                string imagePath = entry.path().string();

                Mat image = imread(imagePath, IMREAD_COLOR);
                if (!image.empty()) 
                {
                    // Compute histograms for database image (Upper 2/3 and Lower 2/3)
                    Mat hist_upper = computeHistogram(image, Rect(0, 0, width, (2 * height) / 3));
                    Mat hist_lower = computeHistogram(image, Rect(0, height / 3, width, (2 * height) / 3));

                    // Compute similarity score (weighted average of histogram intersection)
                    double similarity = computeMultiHistogramSimilarity(targetHistUpper, targetHistLower, hist_upper, hist_lower);
                    similarities.push_back({ similarity, filename });
                }
            }
        }
    }

	// Error handling
    catch (const fs::filesystem_error& ex) 
    {
        cerr << "Error accessing database directory: " << ex.what() << endl;
        return 1;
    }

    // Sort images based on similarity (higher is better)
    sort(similarities.rbegin(), similarities.rend());

    // Display Target Image
    namedWindow("Target Image", WINDOW_NORMAL);
    imshow("Target Image", target_image);
    waitKey(0);

    // Display Top N Matched Images
    for (int i = 1; i < (N+1); ++i) 
    {
        string matchedImagePath = databaseDirectory + "\\" + similarities[i].second;
        Mat matchedImage = imread(matchedImagePath, IMREAD_COLOR);

        if (!matchedImage.empty()) 
        {
            string window_name = "Match " + to_string(i ) + ": " + similarities[i].second;
            namedWindow(window_name, WINDOW_NORMAL);
            imshow(window_name, matchedImage);
        }
        else 
        {
            cerr << "Error: Could not open matched image: " << matchedImagePath << endl;
        }
    }

    waitKey(0);
    destroyAllWindows();
    return 0;
}