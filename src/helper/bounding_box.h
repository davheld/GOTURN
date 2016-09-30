#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <vector>

#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class VOTRegion;

// Represents a bounding box on an image, with some additional functionality.
class BoundingBox
{
public:
  BoundingBox();
  BoundingBox(const std::vector<float>& bounding_box);
  BoundingBox(const VOTRegion& region);

  // Convert bounding box into a vector format.
  void GetVector(std::vector<float>* bounding_box) const;

  // Convert bounding box into VOTRegion format.
  void GetRegion(VOTRegion* region);

  // Print the bounding box coordinates.
  void Print() const;

  // Draw a rectangle corresponding to this bbox with the given color.
  void Draw(const int r, const int g, const int b, cv::Mat* image) const;

  // Draw a white rectangle corresponding to this bbox.
  void DrawBoundingBox(cv::Mat* figure_ptr) const;

  // Normalize the size of the bounding box based on the size of the image.
  void Scale(const cv::Mat& image, BoundingBox* bbox_scaled) const;

  // Unnormalize the size of the bounding box based on the size of the image.
  // (Undoes the effect of Scale).
  void Unscale(const cv::Mat& image, BoundingBox* bbox_unscaled) const;

  // Compute location of bounding box relative to search region
  // edge_spacing_x and edge_spacing_y is the spaving of the image within the search region to account for edge effects.
  // *this should be the ground-truth bbox.
  void Recenter(const BoundingBox& search_location,
                const double edge_spacing_x, const double edge_spacing_y,
                BoundingBox* bbox_recentered) const;

  // Undo the effect of Recenter.
  void Uncenter(const cv::Mat& raw_image, const BoundingBox& search_location,
                const double edge_spacing_x, const double edge_spacing_y,
                BoundingBox* bbox_uncentered) const;

  // Shift the cropped region of the image to generate a new random training example.
  void Shift(const cv::Mat& image,
             const double lambda_scale_frac, const double lambda_shift_frac,
             const double min_scale, const double max_scale,
             const bool shift_motion_model,
             BoundingBox* bbox_rand) const;

  double get_scale_factor() const { return scale_factor_; }
  double get_width() const { return x2_ - x1_;  }
  double get_height() const { return y2_ - y1_; }

  // Compute the bounding box center x and y coordinates.
  double get_center_x() const;
  double get_center_y() const;

  // Get the size of the output image, which is equal to the bounding box with some padding.
  double compute_output_height() const;
  double compute_output_width() const;

  // Get the amount that the output "sticks out" beyond the left and bottom edges of the image.
  // This might be 0, but it might be > 0 if the output is near the edge of the image.
  double edge_spacing_x() const;
  double edge_spacing_y() const;

  // Area enclosed by the bounding box.
  double compute_area() const;

  // Area of intersection between two bounding boxes.
  double compute_intersection(const BoundingBox& bbox) const;

  // Bounding box coordiantes: top left, bottom right.
  double x1_, y1_, x2_, y2_;

  // Factor to scale the bounding box coordinates before inputting into the neural net.
  double scale_factor_;
};

#endif // BOUNDING_BOX_H
