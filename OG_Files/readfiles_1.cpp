/*
  Bruce A. Maxwell
  S21
  
  Sample code to identify image fils in a directory
*/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>

/*
  Given a directory on the command line, scans through the directory for image files.

  Prints out the full path name for each file.  This can be used as an argument to fopen or to cv::imread.
 */
int main(int argc, char *argv[]) {
  char dirname[256];
  char buffer[256];
  FILE *fp;
  std::filesystem::directory_iterator dirp(dirname);
  int i;

  // check for sufficient arguments
  if( argc < 2) {
    printf("usage: %s <directory path>\n", argv[0]);
    exit(-1);
  }

  // get the directory path
  strcpy(dirname, argv[1]);
  printf("Processing directory %s\n", dirname );

  // open the directory
  if (!std::filesystem::exists(dirname)) {
    printf("Cannot open directory %s\n", dirname);
    exit(-1);
  }

  // loop over all the files in the image file listing
  for (const auto& entry : std::filesystem::directory_iterator(dirname)) {
    auto dp = entry.path().filename().string();
    if (dp.find(".jpg") != std::string::npos ||
        dp.find(".png") != std::string::npos ||
        dp.find(".ppm") != std::string::npos ||
        dp.find(".tif") != std::string::npos) {

      printf("processing image file: %s\n", dp.c_str());
      // build the overall filename
      strcpy(buffer, dirname);
      strcat(buffer, "/");
      strcat(buffer, dp.c_str());
      printf("full path name: %s\n", buffer);
    }
  }
  
  printf("Terminating\n");

  return(0);
}