#ifndef EXAMPLE_GENERATOR_H
#define EXAMPLE_GENERATOR_H

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/bounding_box.h"
#include "loader/loader_imagenet_det.h"
#include "loader/video.h"

struct BBParams {
  double lambda_shift;
  double lambda_scale;
  double min_scale;
  double max_scale;
};

// Generates additional training examples by taking random crops of the target object,
// causing apparent translation and scale changes.
class ExampleGenerator
{
public:
  ExampleGenerator(const double lambda_shift, const double lambda_scale,
                   const double min_scale, const double max_scale);

  // Set up to train on the previous and current image, and the previous and current bounding boxes.
  void Reset(const BoundingBox& bbox_prev, const BoundingBox& bbox_curr,
             const cv::Mat& image_prev, const cv::Mat& image_curr);

  // Shift the whole bounding box for the current frame
  // (simulates camera motion)
  void MakeTrainingExampleBBShift(const bool visualize_example,
                                  cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;
  void MakeTrainingExampleBBShift(cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;

  // Focus the current image at the location of the previous frame's
  // bounding box (true motion).
  void MakeTrueExample(cv::Mat* image_focus, cv::Mat* target_pad,
                       BoundingBox* bbox_gt_scaled) const;

  // Make batch_size training examples according to the input parameters.
  void MakeTrainingExamples(const int num_examples, std::vector<cv::Mat>* images,
                            std::vector<cv::Mat>* targets,
                            std::vector<BoundingBox>* bboxes_gt_scaled);


  void set_indices(const int video_index, const int frame_index) {
    video_index_ = video_index; frame_index_ = frame_index;
  }

private:
  void MakeTrainingExampleBBShift(const bool visualize_example,
                                  const BBParams& bbparams,
                                  cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;

  void VisualizeExample(const cv::Mat& target_pad,
                        const cv::Mat& image_rand_focus,
                        const BoundingBox& bbox_gt_scaled) const;

  void get_default_bb_params(BBParams* default_params) const;

  // To generate synethic examples, shift the bounding box by an exponential with the given lambda parameter.
  double lambda_shift_;

  // To generate synethic examples, shift the bounding box by an exponential with the given lambda parameter.
  double lambda_scale_;

  // Do not scale the synthetic examples by more than max_scale_ shrink them by more than min_scale_.
  double min_scale_;
  double max_scale_;

  // Current training image.
  cv::Mat image_curr_;

  // Location of the target within the current and previous images.
  BoundingBox bbox_curr_gt_;
  BoundingBox bbox_prev_gt_;

  // Cropped and scaled image of the target object from the previous image.
  cv::Mat target_pad_;

  // Video and frame index from which the current example was generated.
  // These values are only used when saving images to a file, to assign them
  // a unique identifier.
  int video_index_;
  int frame_index_;
};

#endif // EXAMPLE_GENERATOR_H
