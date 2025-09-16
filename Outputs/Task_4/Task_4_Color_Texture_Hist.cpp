#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const string IMAGE_FOLDER = "C:\\Users\\yashr\\Desktop\\NEU\\Semester 2\\PRCV\\Projects\\Project_2\\olympus\\";

struct ImageData {
    string filename;
    Mat image;
    vector<float> colorHistogram;
    vector<float> textureHistogram; // Now for Sobel magnitude
};

vector<float> getColorHistogram(const Mat& image) {
    vector<float> histogram;
    Mat hsv;
    cvtColor(image, hsv, COLOR_BGR2HSV);

    int hBins = 30, sBins = 32, vBins = 32;
    int histSize[] = { hBins, sBins, vBins };
    float hRanges[] = { 0, 180 }, sRanges[] = { 0, 256 }, vRanges[] = { 0, 256 };
    const float* ranges[] = { hRanges, sRanges, vRanges };
    int channels[] = { 0, 1, 2 };

    Mat hist;
    calcHist(&hsv, 1, channels, Mat(), hist, 3, histSize, ranges, true, false);

    double histSum = 0;
    for (int i = 0; i < hist.rows; i++) {
        for (int j = 0; j < hist.cols; j++) {
            for (int k = 0; k < hist.channels(); k++) {
                histSum += hist.at<Vec3f>(i, j)[k];
            }
        }
    }

    for (int i = 0; i < hist.rows; i++) {
        for (int j = 0; j < hist.cols; j++) {
            for (int k = 0; k < hist.channels(); k++) {
                histogram.push_back(hist.at<Vec3f>(i, j)[k] / histSum);
            }
        }
    }
    return histogram;
}

vector<float> getTextureHistogram(const Mat& image) {
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);

    Mat sobelX, sobelY;
    Sobel(gray, sobelX, CV_32F, 1, 0, 3);
    Sobel(gray, sobelY, CV_32F, 0, 1, 3);

    Mat magnitude;
    magnitude = Mat::zeros(sobelX.size(), CV_32F);
    for (int i = 0; i < sobelX.rows; i++) {
        for (int j = 0; j < sobelX.cols; j++) {
            magnitude.at<float>(i, j) = sqrt(sobelX.at<float>(i, j) * sobelX.at<float>(i, j) + sobelY.at<float>(i, j) * sobelY.at<float>(i, j));
        }
    }

    vector<float> histogram;
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };

    Mat hist;
    calcHist(&magnitude, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

    double histSum = 0;
    for (int i = 0; i < hist.rows; i++) {
        histSum += hist.at<float>(i);
    }
    for (int i = 0; i < hist.rows; i++) {
        histogram.push_back(hist.at<float>(i) / histSum);
    }

    return histogram;
}


vector<ImageData> readImagesFromFolder(const string& folder) {
    vector<ImageData> images;
    vector<String> filenames;
    glob(folder + "*", filenames);

    for (const auto& filename : filenames) {
        Mat image = imread(filename);
        if (!image.empty()) {
            ImageData imageData;
            imageData.filename = filename.substr(folder.length());
            imageData.image = image;
            imageData.colorHistogram = getColorHistogram(image);
            imageData.textureHistogram = getTextureHistogram(image); // Use Sobel magnitude histogram
            images.push_back(imageData);
        }
        else {
            cerr << "Error reading image: " << filename << endl;
        }
    }
    return images;
}


float computeSSD(const vector<float>& hist1, const vector<float>& hist2) {
    float sum = 0.0;
    size_t minSize = min(hist1.size(), hist2.size());
    for (size_t i = 0; i < minSize; i++) {
        float diff = hist1[i] - hist2[i];
        sum += diff * diff;
    }
    return sum;
}

vector<pair<float, string>> findTopMatches(const vector<ImageData>& images, const Mat& targetImage, int N, const string& targetFilename) {
    vector<pair<float, string>> distances;

    vector<float> targetColorHist = getColorHistogram(targetImage);
    vector<float> targetTextureHist = getTextureHistogram(targetImage); // Sobel magnitude

    for (const auto& imgData : images) {
        if (imgData.filename == targetFilename) continue;

        float colorDistance = computeSSD(targetColorHist, imgData.colorHistogram);
        float textureDistance = computeSSD(targetTextureHist, imgData.textureHistogram); // Sobel magnitude

        float distance = colorDistance + textureDistance; // Equal weighting
        distances.push_back({ distance, imgData.filename });
    }

    sort(distances.begin(), distances.end());
    if (N > distances.size()) N = distances.size();
    return vector<pair<float, string>>(distances.begin(), distances.begin() + N);
}



void displayImages(const string& targetImage, const vector<string>& matchImages, int N = 3) {
    Mat target = imread(targetImage);
    if (!target.empty()) {
        namedWindow("Target Image", WINDOW_NORMAL);
        imshow("Target Image", target);
        waitKey(0);
    }
    else {
        cerr << "Error: Could not open target image: " << targetImage << endl;
    }

    for (int i = 0; i < N && i < matchImages.size(); ++i) {
        string matchedImagePath = IMAGE_FOLDER + matchImages[i];
        Mat matchedImage = imread(matchedImagePath, IMREAD_COLOR);

        if (!matchedImage.empty()) {
            string windowName = "Match " + to_string(i + 1) + ": " + matchImages[i];
            namedWindow(windowName, WINDOW_NORMAL);
            imshow(windowName, matchedImage);
        }
        else {
            cerr << "Error: Could not open matched image: " << matchedImagePath << endl;
        }
    }

    waitKey(0);
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <target_image> <N>\n";
        return 1;
    }

    string targetImage = argv[1];
    int N = stoi(argv[2]);

    Mat target = imread(targetImage);
    if (target.empty()) {
        cerr << "Error reading target image: " << targetImage << endl;
        return 1;
    }

    vector<ImageData> images = readImagesFromFolder(IMAGE_FOLDER);
    if (images.empty()) return 1;

    size_t lastSlash = targetImage.find_last_of("/\\");
    string targetFilename = (lastSlash != string::npos) ? targetImage.substr(lastSlash + 1) : targetImage;

    vector<pair<float, string>> topMatches = findTopMatches(images, target, N, targetFilename);

    vector<string> matchFilenames;
    cout << "Top " << N << " matches for " << targetFilename << ":\n";
    for (const auto& match : topMatches) {
        cout << match.second << " (Distance: " << match.first << ")\n";
        matchFilenames.push_back(match.second);
    }

    displayImages(targetImage, matchFilenames, N);

    return 0;
}