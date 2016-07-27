#ifndef IMAGE_PROC_H
#define IMAGE_PROC_H

#include "bounding_box.h"

// Functions to process images for tracking.

// Crop the image at the bounding box location, plus some additional padding.
// To account for edge effects, we use a black background for space beyond the border
// of the image.
void CropPadImage(const BoundingBox& bbox_tight, const cv::Mat& image, cv::Mat* pad_image);
void CropPadImage(const BoundingBox& bbox_tight, const cv::Mat& image, cv::Mat* pad_image,
                  BoundingBox* pad_image_location, double* edge_spacing_x, double* edge_spacing_y);

// Compute the location of the cropped image, which is centered on the bounding box center
// but has a size given by (output_width, output_height) to account for additional padding.
// The cropped image location is also limited by the edge of the image.
void ComputeCropPadImageLocation(const BoundingBox& bbox_tight, const cv::Mat& image, BoundingBox* pad_image_location);

#endif // IMAGE_PROC_H
