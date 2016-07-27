#include "image_proc.h"

void ComputeCropPadImageLocation(const BoundingBox& bbox_tight, const cv::Mat& image, BoundingBox* pad_image_location) {
  // Get the bounding box center.
  const double bbox_center_x = bbox_tight.get_center_x();
  const double bbox_center_y = bbox_tight.get_center_y();

  // Get the image size.
  const double image_width = image.cols;
  const double image_height = image.rows;

  // Get size of output image, which is given by the bounding box + some padding.
  const double output_width = bbox_tight.compute_output_width();
  const double output_height = bbox_tight.compute_output_height();

  // The output image is centered on the bounding box center but has a size given by (output_width, output_height)
  // to account for additional padding.
  // The output image location is also limited by the edge of the image.
  // Get the output image corner.
  const double roi_left = std::max(0.0, bbox_center_x - output_width / 2);
  const double roi_bottom = std::max(0.0, bbox_center_y - output_height / 2);

  // Compute the output image size, limiting the output to within the borders of the original image.

  // The left half of the output has a width of output_width / 2, unless it is bounded by the left image border,
  // in which case it has a width of bbox_center_x.
  const double left_half = std::min(output_width / 2, bbox_center_x);

  // The right half of the output has a width of output_width / 2, unless it is bounded by the right image border,
  // in which case it has a width of (image_width - bbox_center_x).
  const double right_half = std::min(output_width / 2, image_width - bbox_center_x);

  // The total width of the output is the sum of the widths of the two halves, but cannot be smaller than 1 pixel.
  const double roi_width =  std::max(1.0, left_half + right_half);

  // The top half of the output has a height of output_height / 2, unless it is bounded by the top image border.
  const double top_half = std::min(output_height / 2, bbox_center_y);

  // The bottom half of the output has a height of output_height / 2, unless it is bounded by the bottom image border.
  const double bottom_half = std::min(output_height / 2, image_height - bbox_center_y);

  // The total height  of the output is the sum of the heights of the two halves, but cannot be smaller than 1 pixel.
  const double roi_height = std::max(1.0, top_half + bottom_half);

  // Set the output image coordinates.
  pad_image_location->x1_ = roi_left;
  pad_image_location->y1_ = roi_bottom;
  pad_image_location->x2_ = roi_left + roi_width;
  pad_image_location->y2_ = roi_bottom + roi_height;
}

void CropPadImage(const BoundingBox& bbox_tight, const cv::Mat& image, cv::Mat* pad_image) {
  BoundingBox pad_image_location;
  double edge_spacing_x, edge_spacing_y;
  CropPadImage(bbox_tight, image, pad_image, &pad_image_location, &edge_spacing_x, &edge_spacing_y);
}

void CropPadImage(const BoundingBox& bbox_tight, const cv::Mat& image, cv::Mat* pad_image,
                  BoundingBox* pad_image_location, double* edge_spacing_x, double* edge_spacing_y) {
  // Crop the image based on the bounding box location, adding some padding.

  // Get the location of the cropped and padded image.
  ComputeCropPadImageLocation(bbox_tight, image, pad_image_location);

  // Compute the ROI, ensuring that the crop stays within the boundaries of the image.
  const double roi_left = std::min(pad_image_location->x1_, static_cast<double>(image.cols - 1));
  const double roi_bottom = std::min(pad_image_location->y1_, static_cast<double>(image.rows - 1));
  const double roi_width = std::min(static_cast<double>(image.cols), std::max(1.0, ceil(pad_image_location->x2_ - pad_image_location->x1_)));
  const double roi_height = std::min(static_cast<double>(image.rows), std::max(1.0, ceil(pad_image_location->y2_ - pad_image_location->y1_)));

  // Crop the image based on the ROI.
  cv::Rect myROI(roi_left, roi_bottom, roi_width, roi_height);
  cv::Mat cropped_image = image(myROI);

  // Now we need to place the crop in a new image of the appropriate size,
  // adding a black border where necessary to account for edge effects.

  // Make a new image to store the output.
  // The new image should have size: get_output_width(), get_output_height(), but
  // to be safe we ensure that the output is not smaller than roi_width, roi_height.
  const double output_width = std::max(ceil(bbox_tight.compute_output_width()), roi_width);
  const double output_height = std::max(ceil(bbox_tight.compute_output_height()), roi_height);
  cv::Mat output_image = cv::Mat(output_height, output_width, image.type(), cv::Scalar(0, 0, 0));

  // Compute the location to place the crop so that it will be centered at the
  // center of the bounding box (accounting for edge effects).

  // Get the amount that the output "sticks out" beyond the left and bottom edges of the image.
  // This might be 0, but it might be > 0 if the output is near the edge of the image.
  *edge_spacing_x = std::min(bbox_tight.edge_spacing_x(), static_cast<double>(output_image.cols - 1));
  *edge_spacing_y = std::min(bbox_tight.edge_spacing_y(), static_cast<double>(output_image.rows - 1));

  // Get the location within the output to put the cropped image (accounting for edge effects).
  cv::Rect output_rect(*edge_spacing_x, *edge_spacing_y, roi_width, roi_height);
  cv::Mat output_image_roi = output_image(output_rect);

  // Copy the cropped image to the specified location within the output.
  // Without edge effects, this will fill the output.
  // With edge effects, this will comprise a subset of the output, with black
  // being placed around the crop to account for edge effects.
  cropped_image.copyTo(output_image_roi);

  // Set the output.
  *pad_image = output_image;
}

