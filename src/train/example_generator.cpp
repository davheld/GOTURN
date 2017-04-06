#include "example_generator.h"

#include <string>

#include "helper/bounding_box.h"

#include "loader/loader_imagenet_det.h"
#include "helper/high_res_timer.h"
#include "helper/helper.h"
#include "helper/image_proc.h"

using std::string;

// Choose whether to shift boxes using the motion model or using a uniform distribution.
const bool shift_motion_model = true;

ExampleGenerator::ExampleGenerator(const double lambda_shift,
                                   const double lambda_scale,
                                   const double min_scale,
                                   const double max_scale)
  : lambda_shift_(lambda_shift),
    lambda_scale_(lambda_scale),
    min_scale_(min_scale),
    max_scale_(max_scale)
{
}

void ExampleGenerator::Reset(const BoundingBox& bbox_prev,
                             const BoundingBox& bbox_curr,
                             const cv::Mat& image_prev,
                             const cv::Mat& image_curr) {
  // Get padded target from previous image to feed the network.
  CropPadImage(bbox_prev, image_prev, &target_pad_);

  // Save the current image.
  image_curr_ = image_curr;

  // Save the current ground-truth bounding box.
  bbox_curr_gt_ = bbox_curr;

  // Save the previous ground-truth bounding box.
  bbox_prev_gt_ = bbox_prev;
}

void ExampleGenerator::MakeTrainingExamples(const int num_examples,
                                            std::vector<cv::Mat>* images,
                                            std::vector<cv::Mat>* targets,
                                            std::vector<BoundingBox>* bboxes_gt_scaled) {
  for (int i = 0; i < num_examples; ++i) {
    cv::Mat image_rand_focus;
    cv::Mat target_pad;
    BoundingBox bbox_gt_scaled;

    // Make training example by synthetically shifting and scaling the image,
    // creating an apparent translation and scale change of the target object.
    MakeTrainingExampleBBShift(&image_rand_focus, &target_pad, &bbox_gt_scaled);

    images->push_back(image_rand_focus);
    targets->push_back(target_pad);
    bboxes_gt_scaled->push_back(bbox_gt_scaled);
  }
}

void ExampleGenerator::MakeTrueExample(cv::Mat* curr_search_region,
                                       cv::Mat* target_pad,
                                       BoundingBox* bbox_gt_scaled) const {
  *target_pad = target_pad_;

  // Get a tight prior prediction of the target's location in the current image.
  // For simplicity, we use the object's previous location as our guess.
  // TODO - use a motion model?
  const BoundingBox& curr_prior_tight = bbox_prev_gt_;

  // Crop the current image based on the prior estimate, with some padding
  // to define a search region within the current image.
  BoundingBox curr_search_location;
  double edge_spacing_x, edge_spacing_y;
  CropPadImage(curr_prior_tight, image_curr_, curr_search_region, &curr_search_location, &edge_spacing_x, &edge_spacing_y);

  // Recenter the ground-truth bbox relative to the search location.
  BoundingBox bbox_gt_recentered;
  bbox_curr_gt_.Recenter(curr_search_location, edge_spacing_x, edge_spacing_y, &bbox_gt_recentered);

  // Scale the bounding box relative to current crop.
  bbox_gt_recentered.Scale(*curr_search_region, bbox_gt_scaled);
}

void ExampleGenerator::get_default_bb_params(BBParams* default_params) const {
  default_params->lambda_scale = lambda_scale_;
  default_params->lambda_shift = lambda_shift_;
  default_params->min_scale = min_scale_;
  default_params->max_scale = max_scale_;
}

void ExampleGenerator::MakeTrainingExampleBBShift(cv::Mat* image_rand_focus,
                                                  cv::Mat* target_pad,
                                                  BoundingBox* bbox_gt_scaled) const {

  // Get default parameters for how much translation and scale change to apply to the
  // training example.
  BBParams default_bb_params;
  get_default_bb_params(&default_bb_params);

  // Generate training examples.
  const bool visualize_example = false;
  MakeTrainingExampleBBShift(visualize_example, default_bb_params,
                             image_rand_focus, target_pad, bbox_gt_scaled);

}

void ExampleGenerator::MakeTrainingExampleBBShift(
    const bool visualize_example, cv::Mat* image_rand_focus,
    cv::Mat* target_pad, BoundingBox* bbox_gt_scaled) const {
  // Get default parameters for how much translation and scale change to apply to the
  // training example.
  BBParams default_bb_params;
  get_default_bb_params(&default_bb_params);

  // Generate training examples.
  MakeTrainingExampleBBShift(visualize_example, default_bb_params,
                             image_rand_focus, target_pad, bbox_gt_scaled);

}

void ExampleGenerator::MakeTrainingExampleBBShift(const bool visualize_example,
                                                  const BBParams& bbparams,
                                                  cv::Mat* rand_search_region,
                                                  cv::Mat* target_pad,
                                                  BoundingBox* bbox_gt_scaled) const {
  *target_pad = target_pad_;

  // Randomly transform the current image (translation and scale changes).
  BoundingBox bbox_curr_shift;
  bbox_curr_gt_.Shift(image_curr_, bbparams.lambda_scale, bbparams.lambda_shift,
                      bbparams.min_scale, bbparams.max_scale,
                      shift_motion_model,
                      &bbox_curr_shift);

  // Crop the image based at the new location (after applying translation and scale changes).
  double edge_spacing_x, edge_spacing_y;
  BoundingBox rand_search_location;
  CropPadImage(bbox_curr_shift, image_curr_, rand_search_region, &rand_search_location,
               &edge_spacing_x, &edge_spacing_y);

  // Find the shifted ground-truth bounding box location relative to the image crop.
  BoundingBox bbox_gt_recentered;
  bbox_curr_gt_.Recenter(rand_search_location, edge_spacing_x, edge_spacing_y, &bbox_gt_recentered);

  // Scale the ground-truth bounding box relative to the random transformation.
  bbox_gt_recentered.Scale(*rand_search_region, bbox_gt_scaled);

  if (visualize_example) {
    VisualizeExample(*target_pad, *rand_search_region, *bbox_gt_scaled);
  }
}

void ExampleGenerator::VisualizeExample(const cv::Mat& target_pad,
                                        const cv::Mat& image_rand_focus,
                                        const BoundingBox& bbox_gt_scaled) const {
  const bool save_images = false;

  // Show resized target.
  cv::Mat target_resize;
  cv::resize(target_pad, target_resize, cv::Size(227,227));
#ifndef NO_DISPLAY
  cv::namedWindow("Target object", cv::WINDOW_AUTOSIZE );// Create a window for display.
  cv::imshow("Target object", target_resize);                   // Show our image inside it.
#endif
  if (save_images) {
    const string target_name = "Image" + num2str(video_index_) + "_" + num2str(frame_index_) + "target.jpg";
    cv::imwrite(target_name, target_resize);
  }

  // Resize the image.
  cv::Mat image_resize;
  cv::resize(image_rand_focus, image_resize, cv::Size(227, 227));

  // Draw gt bbox.
  BoundingBox bbox_gt_unscaled;
  bbox_gt_scaled.Unscale(image_resize, &bbox_gt_unscaled);
  bbox_gt_unscaled.Draw(0, 255, 0, &image_resize);

  // Show image with bbox.
#ifndef NO_DISPLAY
  cv::namedWindow("Search_region+gt", cv::WINDOW_AUTOSIZE );// Create a window for display.
  cv::imshow("Search_region+gt", image_resize );                   // Show our image inside it.
  cv::waitKey(0);
#endif

  if (save_images) {
    const string image_name = "Image" + num2str(video_index_) + "_" + num2str(frame_index_) + "image.jpg";
    cv::imwrite(image_name, image_resize);
  }
}
