/*
*   Project_2_Task_2 : Histogram Matching
* 
Author: Priyanshu Ranka
Date: 07/02/2025
Subject: PRCV
Problem Statement: 
Implement a program that reads a target image and a database of images, computes a 2D color histogram for each image,
and finds the top N images that are most similar to the target image based on histogram intersection.
*/

// Include necessary libraries
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <opencv2/opencv.hpp>

// Define namespaces
using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Function to compute a 2D color histogram using rg chromaticity
vector<float> computeHistogram(const Mat& image, int bins = 16)
{
	// If image is not present, give error message
    if (image.empty())
    {
        cerr << "Error: Image is empty!" << endl;
        return {};
    }

	// Convert image to float and split into RGB channels
    Mat float_img;
    image.convertTo(float_img, CV_32F);  // Convert to float for division

    vector<Mat> channels(3);
    split(float_img, channels);

	// RGB Color space
    Mat r = channels[2];  // Red channel
    Mat g = channels[1];  // Green channel
    Mat b = channels[0];  // Blue channel

	// Compute rg chromaticity
    Mat sum_rgb = r + g + b + 1e-6;  // Avoid division by zero
    Mat r_norm = r / sum_rgb;
    Mat g_norm = g / sum_rgb;

    // Define histogram parameters
    int histSize[] = { bins, bins };
    float r_range[] = { 0, 1 };
    float g_range[] = { 0, 1 };
    const float* ranges[] = { r_range, g_range };
    int channelsArray[] = { 0, 1 };

    Mat hist;
    Mat rg_planes[] = { r_norm, g_norm };
    calcHist(rg_planes, 2, channelsArray, Mat(), hist, 2, histSize, ranges, true, false);

    // Normalize histogram (sum of all bins = 1)
    normalize(hist, hist, 1, 0, NORM_L1);

    // Flatten histogram into a vector
    vector<float> hist_vector;
    hist_vector.assign((float*)hist.datastart, (float*)hist.dataend);

    return hist_vector;
}

// Function to compute histogram intersection
float computeHistogramIntersection(const vector<float>& h1, const vector<float>& h2)
{
    float intersection = 0;
    for (size_t i = 0; i < h1.size(); ++i)
    {
        intersection += min(h1[i], h2[i]);  // Sum of min values
    }
    return intersection;  // Higher means more similar
}

// Main function
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <targetImagePath>" << endl;
        return 1;
    }

	// Target image path, database directory path and N initializations
    string targetImagePath = argv[1];
    string databaseDirectory = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus";
    int N = 3;  // Top N matches (as required in thee project)

    // Load target image
    Mat target_image = imread(targetImagePath, IMREAD_COLOR);
    if (target_image.empty())
    {
        cerr << "Error: Could not open target image." << endl;
        return 1;
    }

    // Compute histogram for the target image
    vector<float> target_histogram = computeHistogram(target_image);
    if (target_histogram.empty())
    {
        cerr << "Error: Could not compute histogram for target image." << endl;
        return 1;
    }

    // Store filenames and histograms of database images
    vector<pair<float, string>> similarityScores;

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
                    vector<float> imageHistogram = computeHistogram(image);
                    if (!imageHistogram.empty())
                    {
                        float similarity = computeHistogramIntersection(target_histogram, imageHistogram);
                        similarityScores.push_back({ similarity, filename });
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& ex)
    {
        cerr << "Error accessing database directory: " << ex.what() << endl;
        return 1;
    }

    // Sort images by highest similarity score (best matches first)
    sort(similarityScores.rbegin(), similarityScores.rend());

    // Display the target image
    namedWindow("Target Image", WINDOW_NORMAL);
    imshow("Target Image", target_image);
    waitKey(0);

    // Display the top N matched images
    for (int i = 1; i < (N +1); ++i)
    {
        string matchedImagePath = databaseDirectory + "\\" + similarityScores[i].second;
        Mat matchedImage = imread(matchedImagePath, IMREAD_COLOR);

        if (!matchedImage.empty())
        {
            string window_name = "Match " + to_string(i) + ": " + similarityScores[i].second;
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