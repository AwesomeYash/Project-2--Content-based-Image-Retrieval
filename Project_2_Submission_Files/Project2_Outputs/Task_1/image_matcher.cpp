/* Author: Priyanshu Ranka
// Date : 07/02/2025
// Subject : PRCV
// Problem Statement : Implement a program that reads a target image and a database of images, computes a feature vector for each image,and finds 
the top N images that are most similar to the target image based on the Sum of Squared Differences (SSD) matching method. The feature vector is 
a 7x7 square centered at the center of the image. The program should display the target image and the top N matched images in separate windows.
*/

// image_matcher.cpp

// Include necessary headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "csv_utils.h"

// Use the cv and std namespaces so that we don't have to prefix cv:: and std:: everywhere
using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Function to compute the 7x7 square feature vector
vector<float> computeFeature(const Mat& image) 
{
	int row_start = image.rows / 2 - 3;  // This is the center row of the image
	int col_start = image.cols / 2 - 3;  // This is the center column of the image
    
    vector<float> features;
    for (int i = row_start; i < row_start + 7; ++i) 
    {
        for (int j = col_start; j < col_start + 7; ++j) 
        {
            features.push_back(image.at<uchar>(i, j));
        }
    }
	return features; // Returns the 7x7 feature vector
}

// Function to compute Sum of Squared Differences (SSD)
float compute_ssd(const vector<float>& f1, const vector<float>& f2) 
{
    float ssd = 0;
    for (size_t i = 0; i < f1.size(); ++i) {
		ssd += (f1[i] - f2[i]) * (f1[i] - f2[i]); // Squared difference as discussed in the lecture
    }
    return ssd;
}

int main(int argc, char* argv[]) // Main function taking the target image path as input arguement
{
    if (argc != 2) 
    {  // Check for exactly 1 argument (plus the program name)
        cerr << "Usage: " << argv[0] << " <image_path>" << endl;
        return 1;
    }

	// Defining the variables
    string target_image_path = argv[1]; // Get image path from command-line argument
	string database_directory = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus"; // Hardcoded database directory
	string csv_filename = "image_features.csv"; // Name of the CSV file
	string csv_filepath = database_directory + "\\" + csv_filename; // Defing the full path to the CSV file
    string feature_type = "7x7"; 
    string matching_method = "SSD"; 
	int N = 3; // Number of matched images to display (3 as per Task_1)

	// Read the feature vectors from the CSV file
    Mat target_image = imread(target_image_path, IMREAD_COLOR);
    if (target_image.empty()) 
    {
        cerr << "Error: Could not open target image." << endl;
        return 1;
    }

	// Compute the feature vector for the target image
    vector<float> target_features;
    if (feature_type == "7x7") 
    {
        target_features = computeFeature(target_image);
    }
    else 
    {
        cerr << "Error: Unknown feature type." << endl;
        return 1;
    }

	// Variables to store image filenames and feature vectors
    vector<string> image_filenames;
    vector<vector<float>> image_features;

    try 
    {
		for (const auto& entry : fs::directory_iterator(database_directory)) // Iterate over all files in the database directory
        {
			if (entry.is_regular_file()) //`Check if the entry is fine or not
            {
                string filename = entry.path().filename().string();
                string image_path = entry.path().string();

				Mat image = imread(image_path, IMREAD_COLOR); // Read the image from the directory
                if (!image.empty()) 
                {
                    vector<float> features;
                    if (feature_type == "7x7") 
                    {
                        features = computeFeature(image);
                    }
                    else 
                    {
						cerr << "Error: Unknown feature type." << endl; // Error message if the feature not found
                        return 1;
                    }
                    
                    image_filenames.push_back(filename);
                    image_features.push_back(features);
                }
            }
        }
    }
    
    
	// Catch block to handle the exception
    catch (const fs::filesystem_error& ex) 
    {
        cerr << "Error accessing database directory: " << ex.what() << endl;
        return 1;
    }
    
    vector<pair<float, string>> distances;
    for (size_t i = 0; i < image_features.size(); ++i) {
        float distance = 0;
        if (matching_method == "SSD") {
            distance = compute_ssd(target_features, image_features[i]);
        }
        else {
            cerr << "Error: Unknown matching method." << endl;
            return 1;
        }
        distances.push_back({ distance, image_filenames[i] });
    }

    sort(distances.begin(), distances.end());
    
    // Display the target image
    namedWindow("Target Image", WINDOW_NORMAL);
    imshow("Target Image", target_image);
    waitKey(0);

    // Display the top N matched images
    for (int i = 1; i < min((int)distances.size(), N+1); ++i)     
    {
        string matched_image_path = database_directory + "\\" + distances[i].second; // Construct full path
        Mat matched_image = imread(matched_image_path, IMREAD_COLOR);

        if (!matched_image.empty()) 
        {
            string window_name = "Match " + to_string(i) + ": " + distances[i].second;
            namedWindow(window_name, WINDOW_NORMAL);
            imshow(window_name, matched_image);
        }
        else 
        {
            cerr << "Error: Could not open matched image: " << matched_image_path << endl;
        }
    }

	waitKey(0);
	destroyAllWindows();
    return 0;
}