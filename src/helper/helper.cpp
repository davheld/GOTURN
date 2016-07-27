/*
 * helper.cpp
 *
 *  Created on: Jul 11, 2011
 *      Author: davheld
 */

#include "helper.h"

#include <string>
#include <cstdio>
#include <vector>

namespace bfs = boost::filesystem;

using std::string;
using std::sprintf;
using std::vector;

// Maximum length of a string created from a number.
const int kMaxNum2StringSize = 50;

// Default number of decimal places when converting a double to a string.
const int kDefaultDecimalPlaces = 6;


string int2str(const int num) {
 char num_buffer[kMaxNum2StringSize];
 sprintf(num_buffer, "%d", num);
 return num_buffer;
}

string num2str(const int num) {
  return int2str(num);
}

string double2str(const double num, const int decimal_places) {
  const string format = "%." + num2str(decimal_places) + "lf";
  char num_buffer[kMaxNum2StringSize];
  sprintf(num_buffer, format.c_str(), num);
  return num_buffer;
}

string float2str(const float num) {
 char num_buffer[kMaxNum2StringSize];
 sprintf(num_buffer, "%f", num);
 return num_buffer;
}

string unsignedint2str(const unsigned int num) {
  char num_buffer[kMaxNum2StringSize];
  sprintf(num_buffer, "%u", num);
  return num_buffer;
}

string num2str(const double num) {
  return double2str(num, kDefaultDecimalPlaces);
}

string num2str(const double num, int decimal_places) {
  return double2str(num, decimal_places);
}

string num2str(const float num) {
  return float2str(num);
}

string num2str(const unsigned int num) {
  return unsignedint2str(num);
}

string num2str(const size_t num) {
  char num_buffer[50];
  sprintf(num_buffer, "%zu", num);
  return num_buffer;
}

// *******File IO *************

void find_subfolders(const bfs::path& folder, vector<string>* sub_folders) {
  if (!bfs::is_directory(folder)) {
    printf("Error - %s is not a valid directory!\n", folder.c_str());
    return;
  }

  bfs::directory_iterator end_itr; // default construction yields past-the-end
  for (bfs::directory_iterator itr(folder); itr != end_itr; ++itr) {
    if (bfs::is_directory(itr->status())) {
      string filename = itr->path().filename().string();
      sub_folders->push_back(filename);
    }
  }

  // Sort the files by name.
  std::sort(sub_folders->begin(), sub_folders->end());
}

void find_matching_files(const bfs::path& folder, const boost::regex filter,
                         vector<string>* files) {
  if (!bfs::is_directory(folder)) {
    printf("Error - %s is not a valid directory!\n", folder.c_str());
    return;
  }

  bfs::directory_iterator end_itr; // default construction yields past-the-end
  for (bfs::directory_iterator itr(folder); itr != end_itr; ++itr) {
    if (bfs::is_regular_file(itr->status())) {
      string filename = itr->path().filename().string();

      boost::smatch what;
      if(boost::regex_match(filename, what, filter) ) {
        files->push_back(filename);
      }
    }
  }

  // Sort the files by name.
  std::sort(files->begin(), files->end());
}

double sample_rand_uniform() {
  // Generate a random number in (0,1)
  // http://www.cplusplus.com/forum/beginner/7445/
  return (rand() + 1) / (static_cast<double>(RAND_MAX) + 2);
}

double sample_exp(const double lambda) {
  // Sample from an exponential - http://stackoverflow.com/questions/11491458/how-to-generate-random-numbers-with-exponential-distribution-with-mean
  const double rand_uniform = sample_rand_uniform();
  return -log(rand_uniform) / lambda;
}

double sample_exp_two_sided(const double lambda) {
  // Determine which side of the two-sided exponential we are sampling from.
  const double pos_or_neg = (rand() % 2 == 0) ? 1 : -1;

  // Sample from an exponential - http://stackoverflow.com/questions/11491458/how-to-generate-random-numbers-with-exponential-distribution-with-mean
  const double rand_uniform = sample_rand_uniform();
  return log(rand_uniform) / lambda * pos_or_neg;
}
