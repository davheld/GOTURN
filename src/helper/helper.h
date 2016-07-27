/*
 * helper.h
 *
 *  Created on: Jul 11, 2011
 *      Author: davheld
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Convenience helper functions.

// *******Number / string conversions*************

// Conversions from number into a string.
std::string num2str(const int num);
std::string num2str(const float num);
std::string num2str(const double num);
std::string num2str(const double num, const int decimal_places);
std::string num2str(const unsigned int num);
std::string num2str(const size_t num);

// Conversions from string into a number.
template<class T>
  T str2num(const std::string& s);


// Template implementation
template<class T>
    T str2num(const std::string& s)
{
     std::istringstream stream (s);
     T t;
     stream >> t;
     return t;
}

// *******File IO *************

// Find all subfolder of the given folder.
void find_subfolders(const boost::filesystem::path& folder, std::vector<std::string>* sub_folders);

// Find all files within a given folder that match a given regex filter.
void find_matching_files(const boost::filesystem::path& folder, const boost::regex filter,
                         std::vector<std::string>* files);

// *******Probability*************
// Generate a random number in (0,1)
double sample_rand_uniform();

// Sample from an exponential distribution.
double sample_exp(const double lambda);

// Sample from a Laplacian distribution, aka two-sided exponential.
double sample_exp_two_sided(const double lambda);

#endif /* HELPER_H_ */

