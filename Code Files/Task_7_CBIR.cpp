/*
Author: Priyanshu Ranka (Modified by Gemini)
Semester : Spring 2025
Subject : PRCV
Task_7: Custom Design
Description: Executing Content Based Image Recognition ie. CBIR for shoe image
*/

// Include Directories
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>

// Namespaces
using namespace std;
using namespace cv;

// Flepaths for Directory and .csv file
const string CSV_FILE_PATH = "ResNet18_olym.csv";
const string IMAGE_FOLDER = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus\\";

// Image Variable (Structure)
struct ImageData 
{
    string filename;
    vector<float> features; // DNN features
    Mat image;             // Image data for color histograms
};

// Function to calculate color histogram features
vector<float> getColorHistogram(const Mat& image) 
{
    vector<float> histogram;
    if (!image.empty()) 
    {
        Mat hsv;
        cvtColor(image, hsv, COLOR_BGR2HSV);  // Changing color spcae to HSV

        int hBins = 30, sBins = 32, vBins = 32;     // No of bins
        int histSize[] = {hBins, sBins, vBins};     // Histogram size
        float hRanges[] = {0, 180}, sRanges[] = {0, 256}, vRanges[] = {0, 256};
        const float* ranges[] = {hRanges, sRanges, vRanges};
        int channels[] = {0, 1, 2};     // 3D histogram

        Mat hist;
        calcHist(&hsv, 1, channels, Mat(), hist, 3, histSize, ranges, true, false);

        for (int i = 0; i < hist.rows; i++) 
        {
            for (int j = 0; j < hist.cols; j++) 
            {
                for(int k = 0; k < hist.channels(); k++)
                {
                    histogram.push_back(hist.at<Vec3f>(i,j)[k]);
                }
            }
        }
    }
    return histogram;
}

// Reading CSV file
vector<ImageData> readCSV() 
{
    vector<ImageData> images;
    ifstream file(CSV_FILE_PATH);
    string line;

    // Error handling
    if (!file.is_open()) 
    {
        cerr << "Error: Unable to open file " << CSV_FILE_PATH << endl;
        return images;
    }

    while (getline(file, line)) 
    {
        stringstream ss(line);
        string value, fname;
        vector<float> features;

        getline(ss, fname, ',');

        // Gettin data from file
        while (getline(ss, value, ',')) {
            features.push_back(stof(value));
        }

        if (features.size() == 512) {
            string imagePath = IMAGE_FOLDER + fname;
            Mat image = imread(imagePath);
            if (!image.empty()) {
                ImageData imageData;
                imageData.filename = fname;
                imageData.features = features;
                imageData.image = image;
                images.push_back(imageData);
            } else {
                cerr << "Error: Could not read image: " << imagePath << endl;
            }
        }
    }

    file.close();
    return images;
}

// Function to get the features
vector<float> getTargetFeatures(const vector<ImageData>& images, const string& targetFilename) 
{
    for (const auto& img : images) 
    {
        if (img.filename == targetFilename) 
        {
            vector<float> combinedFeatures = img.features;
            vector<float> colorHist = getColorHistogram(img.image);
            combinedFeatures.insert(combinedFeatures.end(), colorHist.begin(), colorHist.end()); //Append color histogram to DNN features
            return combinedFeatures;
        }
    }
    cerr << "Error: Target image " << targetFilename << " not found in database!" << endl;
    return {};
}

// Function to compute SSD
float computeSSD(const vector<float>& v1, const vector<float>& v2) 
{
    float sum = 0.0;
    size_t minSize = min(v1.size(), v2.size()); // Handle potential size differences
    for (size_t i = 0; i < minSize; i++) {
        float diff = v1[i] - v2[i];
        sum += diff * diff;
    }
    return sum;
}

// Get the most similar images
vector<pair<float, string>> findTopMatches(const vector<ImageData>& images, const vector<float>& targetFeatures, int N, const string& targetFilename) {
    vector<pair<float, string>> distances;

    // 
    for (const auto& img : images) 
    {
        // To skip the target image
        if (img.filename == targetFilename) 
        {
            continue;
        }
        vector<float> combinedFeatures = img.features;
        vector<float> colorHist = getColorHistogram(img.image);
        combinedFeatures.insert(combinedFeatures.end(), colorHist.begin(), colorHist.end());
        float ssd = computeSSD(targetFeatures, combinedFeatures);
        distances.push_back({ssd, img.filename});
    }

    // Sorting according to distance value
    sort(distances.begin(), distances.end());
    if (N > distances.size()) N = distances.size();
    return vector<pair<float, string>>(distances.begin(), distances.begin() + N);
}

// Function to Display the images
void displayImages(const string& targetImage, const vector<string>& matchImages, int N = 3) 
{
    // Target Image Display
    Mat target = imread(targetImage);
    
    namedWindow("Target Image", WINDOW_NORMAL);
    imshow("Target Image", target);
    waitKey(0);
   

    for (int i = 0; i < N && i < matchImages.size(); ++i) 
    {
        string matchedImagePath = IMAGE_FOLDER + matchImages[i];
        Mat matchedImage = imread(matchedImagePath, IMREAD_COLOR);

        if (!matchedImage.empty()) 
        {
            string windowName = "Match " + to_string(i + 1) + ": " + matchImages[i];
            namedWindow(windowName, WINDOW_NORMAL);
            imshow(windowName, matchedImage);
        }
        else 
            cerr << "Error: Could not open matched image: " << matchedImagePath << endl;
    }

    waitKey(0);
}

//Main Fucntion
int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        cerr << "Usage: " << argv[0] << " <target_image> <N>\n";
        return 1;
    }

    // Target image and no of similar images required
    string targetImage = argv[1];
    int N = stoi(argv[2]);

    vector<ImageData> images = readCSV();
    
    size_t lastSlash = targetImage.find_last_of("/\\");
    string targetFilename = (lastSlash != string::npos) ? targetImage.substr(lastSlash + 1) : targetImage;

    // Target image features
    vector<float> targetFeatures = getTargetFeatures(images, targetFilename);
    if (targetFeatures.empty()) return 1;

    // Finding top Matches
    vector<pair<float, string>> topMatches = findTopMatches(images, targetFeatures, N, targetFilename);

    // Displaying Image Number and SSD from target image
    vector<string> matchFilenames;
    cout << "Top " << N << " matches for " << targetFilename << ":\n";
    for (const auto& match : topMatches) {
        cout << match.second << " (SSD: " << match.first << ")\n";
        matchFilenames.push_back(match.second);
    }

    // To show the images
    displayImages(targetImage, matchFilenames, N);

    return 0;
}