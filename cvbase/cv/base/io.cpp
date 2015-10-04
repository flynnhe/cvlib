#include "io.h"

bool cv::base::readHomography(const char* filename, cv::Mat* H)
{
  std::ifstream in;
  in.open(filename);
  if (!in.is_open()) {
    return false;
  }

  cv::Mat_<float> res(3, 3, CV_32FC1);
  char str[256];
  for (int j = 0; j < 3; ++j) {
    in.getline(str, 256);
    char* tokens = strtok(str, " ");
    for (int i = 0; i < 3; ++i) {
      if (tokens == NULL) {
        return false;
      }
      res(j, i) = (float)atof(tokens);
      tokens = strtok(NULL, " ");
    }
  }
  *H = res;
  return true;
}