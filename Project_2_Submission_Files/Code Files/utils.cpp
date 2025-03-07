// utils.cpp (formerly csv_utils.cpp)
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"
#include "utils.h" // Changed to utils.h
#include <cstdlib> // For atoi and atof

/*
 * reads a string from a CSV file. the 0-terminated string is returned in the char array os.
 * The function returns false if it is successfully read. It returns true if it reaches the end of the line or the file.
 */

int getstring(FILE* fp, char os[]) {
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        }
        else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }
        os[p] = ch;
        p++;
    }
    os[p] = '\0';

    return (eol); // return true if eol
}

int getint(FILE* fp, int* v) {
    char s[256];
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        }
        else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }

        s[p] = ch;
        p++;
    }
    s[p] = '\0'; // terminator
    *v = atoi(s);

    return (eol); // return true if eol
}

/*
 * Utility function for reading one float value from a CSV file
 * The value is stored in the v parameter
 * The function returns true if it reaches the end of a line or the file
 */
int getfloat(FILE* fp, float* v) {
    char s[256];
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        }
        else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }

        s[p] = ch;
        p++;
    }
    s[p] = '\0'; // terminator
    *v = atof(s);

    return (eol); // return true if eol
}

int append_image_data_csv(const char* filename, const char* image_filename, std::vector<float>& image_data, int reset_file) {
    char mode[8];
    FILE* fp;

    strcpy(mode, "a");

    if (reset_file) {
        strcpy(mode, "w");
    }

    fp = fopen(filename, mode);
    if (!fp) {
        perror("Unable to open output file");
        return 1;
    }

    fprintf(fp, "%s", image_filename);
    for (size_t i = 0; i < image_data.size(); i++) {
        fprintf(fp, ",%.4f", image_data[i]);
    }

    fprintf(fp, "\n");

    fclose(fp);

    return 0;
}

int read_image_data_csv(const char* filename, std::vector<std::string>& filenames, std::vector<std::vector<float>>& data, int echo_file) {
    FILE* fp;
    float fval;
    char img_file[256];

    fp = fopen(filename, "r");
    if (!fp) {
        perror("Unable to open feature file");
        return 1;
    }

    printf("Reading %s\n", filename);
    for (;;) {
        std::vector<float> dvec;

        if (getstring(fp, img_file)) {
            break;
        }

        filenames.push_back(std::string(img_file));

        for (;;) {
            float eol = getfloat(fp, &fval);
            dvec.push_back(fval);
            if (eol) break;
        }

        data.push_back(dvec);
    }
    fclose(fp);
    printf("Finished reading CSV file\n");

    if (echo_file) {
        for (size_t i = 0; i < data.size(); i++) {
            for (size_t j = 0; j < data[i].size(); j++) {
                printf("%.4f ", data[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }

    return 0;
}


int sobelX3x3(cv::Mat& src, cv::Mat& dst)
{
    // Define the separable sobel kernels
    int sobelHorizontal[3] = { -1, 0, 1 };
    int sobelVertical[3] = { 1, 2, 1 };

    // Create tmp frame for intermediate step
    cv::Mat tmp = cv::Mat::zeros(src.size(), CV_16SC3);
    dst = cv::Mat::zeros(src.size(), CV_16SC3);

    for (int y = 0; y < src.rows; y++) {
        for (int x = 1; x < src.cols - 1; x++) {
            cv::Vec3s gradient = { 0, 0, 0 };

            for (int k = -1; k < 2; k++) {
                cv::Vec3b pixel = src.at<cv::Vec3b>(y, x + k);

                for (int c = 0; c < 3; c++) {
                    gradient[c] += pixel[c] * sobelHorizontal[k + 1];
                }
            }

            tmp.at<cv::Vec3s>(y, x) = gradient;

        }
    }

    for (int y = 1; y < tmp.rows - 1; y++) {
        for (int x = 0; x < tmp.cols; x++) {
            cv::Vec3s gradient = { 0, 0, 0 };

            for (int k = -1; k < 2; k++) {
                cv::Vec3s pixel = tmp.at<cv::Vec3s>(y + k, x);

                for (int c = 0; c < 3; c++) {
                    gradient[c] += pixel[c] * sobelVertical[k + 1];
                }
            }

            dst.at<cv::Vec3s>(y, x) = gradient;

        }
    }

    return 0; // Success
}

int sobelY3x3(cv::Mat& src, cv::Mat& dst)
{
    // Define the sobel kernels
    int sobelVertical[3] = { -1, 0, 1 };
    int sobelHorizontal[3] = { 1, 2, 1 };

    // Create tmp frame for intermediate step
    cv::Mat tmp = cv::Mat::zeros(src.size(), CV_16SC3);
    dst = cv::Mat::zeros(src.size(), CV_16SC3);

    // Vertical gradient
    for (int y = 1; y < src.rows - 1; y++) {
        for (int x = 0; x < src.cols; x++) {
            cv::Vec3s gradient = { 0, 0, 0 };

            // Apply the filter to the pixel
            for (int k = -1; k < 2; k++) {
                cv::Vec3b pixel = src.at<cv::Vec3b>(y + k, x);

                // Consider each color channel
                for (int c = 0; c < 3; c++) {
                    gradient[c] += pixel[c] * sobelVertical[k + 1];
                }
            }

            // Set the pixel value to the gradient
            tmp.at<cv::Vec3s>(y, x) = gradient;

        }
    }

    // Horizontal gradient
    for (int y = 0; y < tmp.rows; y++) {
        for (int x = 1; x < tmp.cols - 1; x++) {
            cv::Vec3s gradient = { 0, 0, 0 };

            // Apply the filter to the pixel
            for (int k = -1; k < 2; k++) {
                cv::Vec3s pixel = tmp.at<cv::Vec3s>(y, x + k);

                // Consider each color channel
                for (int c = 0; c < 3; c++) {
                    gradient[c] += pixel[c] * sobelHorizontal[k + 1];
                }
            }

            // Set the pixel value to the gradient
            dst.at<cv::Vec3s>(y, x) = gradient;

        }
    }

    return 0; // Success
}
