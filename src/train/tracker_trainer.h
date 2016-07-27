#ifndef TRACKER_TRAINER_H
#define TRACKER_TRAINER_H


#include <vector>
#include <opencv/cv.h>

#include "helper/bounding_box.h"
#include "tracker/tracker.h"
#include "network/regressor_train_base.h"

class TrackerTrainer
{
public:
  TrackerTrainer(ExampleGenerator* example_generator);

  TrackerTrainer(ExampleGenerator* example_generator,
                 RegressorTrainBase* regressor_train);

  // Train from this example.
  // Inputs: previous image, current image, previous image's bounding box, current image's bounding box.
  void Train(const cv::Mat& image_prev, const cv::Mat& image_curr,
             const BoundingBox& bbox_prev, const BoundingBox& bbox_curr);

  // Number of total batches trained on so far.
  int get_num_batches() { return num_batches_; }

private:
  // Generate training examples and return them.
  // Note that we do not clear the input variables, so if they already contain
  // some examples then we will append to them.
  virtual void MakeTrainingExamples(std::vector<cv::Mat>* images,
                            std::vector<cv::Mat>* targets,
                            std::vector<BoundingBox>* bboxes_gt_scaled);

  // Train on the batch.
  virtual void ProcessBatch();

  // Data in the current training batch.
  std::vector<cv::Mat> images_batch_;
  std::vector<cv::Mat> targets_batch_;
  std::vector<BoundingBox> bboxes_gt_scaled_batch_;

  // Used to generate additional training examples through synthetic transformations.
  ExampleGenerator* example_generator_;

  // Neural network.
  RegressorTrainBase* regressor_train_;

  // Number of total batches trained on so far.
  int num_batches_;
};

#endif // TRACKER_TRAINER_H
