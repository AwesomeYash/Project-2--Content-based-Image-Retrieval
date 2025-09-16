/*
Author: Priyanshu Ranka
Semester : Spring 2025
Subject : PRCV
Task_5: Deep Network Embeddings
Description: This code reads the feature vectors of images from a CSV file and finds the top N closest images to a target image using the SSD distance metric.
*/

// Include directives
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>

// Namespace declarations
using namespace std;
using namespace cv;

// Hardcoded paths
const string CSV_FILE_PATH = "ResNet18_olym.csv";   // CSV File containing image filenames and feature vectors
const string IMAGE_FOLDER = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus\\";  // Folder containing images

// Structure to store image filename and feature vector
struct ImageData 
{
    string filename;
    vector<float> features;
};

// Function to read the CSV file and store image data
vector<ImageData> readCSV() 
{   // Read the CSV file and store image data in a vector
    vector<ImageData> images;
    ifstream file(CSV_FILE_PATH);
    string line;

	// Error handling: Unable to open file
    if (!file.is_open()) 
    {
        cerr << "Error: Unable to open file " << CSV_FILE_PATH << endl;
        return images;
    }

	// Read each line of the CSV file
    while (getline(file, line)) 
	{   // Read each line of the CSV file
        stringstream ss(line);
        string value, fname;
        vector<float> features;

        // Read the filename (first element)
        getline(ss, fname, ',');

        // Read the 512 feature values (remaining elements)
        while (getline(ss, value, ',')) {
            features.push_back(stof(value));
        }

		// Ensure correct feature size (Debugging)
        if (features.size() == 512) { // Ensure correct feature size
            images.push_back({ fname, features });
        }
    }

    file.close();
    return images;
}

// Function to find the feature vector of the target image
vector<float> getTargetFeatures(const vector<ImageData>& images, const string& targetFilename) 
{
    // Getting features of the target image
    for (const auto& img : images) 
    {
        if (img.filename == targetFilename) 
        {
            return img.features;
        }
    }
	// Error handling: Target image not found in database
    cerr << "Error: Target image " << targetFilename << " not found in database!" << endl;
    return {};
}

// Function to compute SSD distance between two feature vectors
float computeSSD(const vector<float>& v1, const vector<float>& v2) 
{
    float sum = 0.0;
    for (size_t i = 0; i < v1.size(); i++) 
    {
        float diff = v1[i] - v2[i];
        sum += diff * diff;
    }
    return sum;
}

// Function to find the top N closest images using SSD, excluding the target image itself
vector<pair<float, string>> findTopMatches(const vector<ImageData>& images, const vector<float>& targetFeatures, int N, const string& targetFilename) 
{
	// variables to store distances
    vector<pair<float, string>> distances;

    for (const auto& img : images) 
    {
        // To skip the target image itself
        if (img.filename == targetFilename) 
        {
            continue;
        }

		// Compute SSD distance between target and current image
        float ssd = computeSSD(targetFeatures, img.features);
        distances.push_back({ ssd, img.filename });
    }

    // Sort by ascending SSD distance
    sort(distances.begin(), distances.end());

    // Get top N matches
    if (N > distances.size()) N = distances.size();
    return vector<pair<float, string>>(distances.begin(), distances.begin() + N);
}

// Function to display the target image and top matches
void displayImages(const string& targetImage, const vector<string>& matchImages, int N = 3) 
{
    // Load the target image
    Mat target = imread(targetImage);
    
    // Display Target Image
    namedWindow("Target Image", WINDOW_NORMAL);
    imshow("Target Image", target);
    waitKey(0);  // Wait for a key press before continuing

    // Display Top N Matched Images
    for (int i = 0; i < N && i < matchImages.size(); ++i) 
    {
        string matchedImagePath = IMAGE_FOLDER + matchImages[i];  // Full path to the matched image
        Mat matchedImage = imread(matchedImagePath, IMREAD_COLOR);

        if (!matchedImage.empty()) 
        {
            string windowName = "Match " + to_string(i + 1) + ": " + matchImages[i];
            namedWindow(windowName, WINDOW_NORMAL);
            imshow(windowName, matchedImage);
        }
        else 
        {
            cerr << "Error: Could not open matched image: " << matchedImagePath << endl;
        }
    }

    waitKey(0);  // Wait for user to press a key after displaying all images
}

// Main function
int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        cerr << "Usage: " << argv[0] << " <target_image> <N>\n";
        return 1;
    }

	// Target image and number of matches as input 
    string targetImage = argv[1];
    int N = stoi(argv[2]);

    // Read CSV file
    vector<ImageData> images = readCSV();
    if (images.empty()) return 1;

    // Extract only filename from full path
    size_t lastSlash = targetImage.find_last_of("/\\");
    string targetFilename = (lastSlash != string::npos) ? targetImage.substr(lastSlash + 1) : targetImage;

    // Get target image features
    vector<float> targetFeatures = getTargetFeatures(images, targetFilename);
    if (targetFeatures.empty()) return 1;

    // Find top N matching images, excluding the target image
    vector<pair<float, string>> topMatches = findTopMatches(images, targetFeatures, N, targetFilename);

	// Debugging: Print top matches
    // Print and store results
    vector<string> matchFilenames;
    cout << "Top " << N << " matches for " << targetFilename << ":\n";
    for (const auto& match : topMatches) {
        cout << match.second << " (SSD: " << match.first << ")\n";
        matchFilenames.push_back(match.second);
    }

    // Display target image and top N matched images
    displayImages(targetImage, matchFilenames, N);

    return 0;
}