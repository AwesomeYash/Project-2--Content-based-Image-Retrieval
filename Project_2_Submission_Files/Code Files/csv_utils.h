// csv_utils.h
#ifndef CSV_UTIL_H
#define CSV_UTIL_H

#include <vector>
#include <string>

int append_image_data_csv(const char* filename, const char* image_filename, std::vector<float>& image_data, int reset_file = 0);
int read_image_data_csv(const char* filename, std::vector<std::string>& filenames, std::vector<std::vector<float>>& data, int echo_file = 0);


#endif
