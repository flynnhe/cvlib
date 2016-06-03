#include "io.h"
#ifdef _WIN32
#include <Windows.h>
#endif

bool cv::base::readFilesInDirectory(const char* path, std::vector<std::string>* files, const char* ext)
{
  bool success = false;
#ifdef _WIN32
  files->clear();
  HANDLE hFind;
  WIN32_FIND_DATA data;

  char wildcard[256];
  sprintf(wildcard, "%s\\*.%s", path, ext);
  hFind = FindFirstFile(wildcard, &data);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      files->push_back(data.cFileName);
    } while (FindNextFile(hFind, &data));
    FindClose(hFind);
  }
#endif
  return success;
}

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
