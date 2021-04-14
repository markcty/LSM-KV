//
// Created by markcty on 2021/4/14.
//

#ifndef LSM_KV__UTIL_H_
#define LSM_KV__UTIL_H_

#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <dirent.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <stdio.h>
#include <io.h>
#endif

namespace utils {
/**
 * Check whether directory exists
 * @param path directory to be checked.
 * @return ture if directory exists, false otherwise.
 */
bool dirExists(std::string path) {
  struct stat st;
  int ret = stat(path.c_str(), &st);
  return ret == 0 && st.st_mode & S_IFDIR;
}

/**
 * list all filename in a directory
 * @param path directory path.
 * @return name of all files.
 */
std::vector<std::string> scanDir(std::string path) {
  std::vector<std::string> ret;
  DIR *dir;
  struct dirent *rent;
  dir = opendir(path.c_str());
  char s[100];
  while ((rent = readdir(dir))) {
    strcpy(s, rent->d_name);
    if (s[0] != '.') {
      ret.push_back(s);
    }
  }
  return ret;
}

/**
 * Create directory
 * @param path directory to be created.
 * @return 0 if directory is created successfully, -1 otherwise.
 */
int _mkdir(const char *path) {
#ifdef _WIN32
  return ::_mkdir(path);
#else
  return ::mkdir(path, 0775);
#endif
}

/**
 * Create directory recursively
 * @param path directory to be created.
 * @return 0 if directory is created successfully, -1 otherwise.
 */
int mkdir(const char *path) {
  std::string currentPath = "";
  std::string dirName;
  std::stringstream ss(path);

  while (std::getline(ss, dirName, '/')) {
    currentPath += dirName;
    if (!dirExists(currentPath) && _mkdir(currentPath.c_str()) != 0) {
      return -1;
    }
    currentPath += "/";
  }
  return 0;
}

/**
 * Delete a empty directory
 * @param path directory to be deleted.
 * @return 0 if delete successfully, -1 otherwise.
 */
int rmdir(const char *path) {
#ifdef _WIN32
  return ::_rmdir(path);
#else
  return ::rmdir(path);
#endif
}

/**
 * Delete a file
 * @param path file to be deleted.
 * @return 0 if delete successfully, -1 otherwise.
 */
int rmfile(const char *path) {
#ifdef _WIN32
  return ::_unlink(path);
#else
  return ::unlink(path);
#endif
}
}
#endif //LSM_KV__UTIL_H_
