// csv_utils.cpp
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"
#include "csv_utils.h"
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
        perror("Unable to open output file"); // More robust error handling
        return 1; // Indicate error
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
        perror("Unable to open feature file"); // More robust error handling
        return 1; // Indicate error
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