#include "tracker_trainer.h"

#include "caffe/caffe.hpp"

#include "network/regressor.h"

// Number of images in each batch.
const int kBatchSize = 50;

// Number of examples that we generate (by applying synthetic transformations)
// to each image.
const int kGeneratedExamplesPerImage = 10;

TrackerTrainer::TrackerTrainer(ExampleGenerator* example_generator)
  : example_generator_(example_generator),
    num_batches_(0)
{
}

TrackerTrainer::TrackerTrainer(ExampleGenerator* example_generator,
                               RegressorTrainBase* regressor_train)
  : example_generator_(example_generator),
    regressor_train_(regressor_train),
    num_batches_(0)
{
}

void TrackerTrainer::MakeTrainingExamples(std::vector<cv::Mat>* images,
                                          std::vector<cv::Mat>* targets,
                                          std::vector<BoundingBox>* bboxes_gt_scaled) {
  // Generate true example.
  cv::Mat image;
  cv::Mat target;
  BoundingBox bbox_gt_scaled;
  example_generator_->MakeTrueExample(&image, &target, &bbox_gt_scaled);
  images->push_back(image);
  targets->push_back(target);
  bboxes_gt_scaled->push_back(bbox_gt_scaled);

  // Generate additional training examples through synthetic transformations.
  example_generator_->MakeTrainingExamples(kGeneratedExamplesPerImage, images,
                                           targets, bboxes_gt_scaled);
}

void TrackerTrainer::ProcessBatch() {
  // Train the neural network tracker with these examples.
  regressor_train_->Train(images_batch_, targets_batch_,
                          bboxes_gt_scaled_batch_);
}

void TrackerTrainer::Train(const cv::Mat& image_prev, const cv::Mat& image_curr,
                           const BoundingBox& bbox_prev, const BoundingBox& bbox_curr) {
  // Check that the saved batches are of appropriate dimensions.
  CHECK_EQ(images_batch_.size(), targets_batch_.size())
      << " images_batch: " << images_batch_.size() <<
         " targets_batch: " << targets_batch_.size();

  CHECK_EQ(images_batch_.size(), bboxes_gt_scaled_batch_.size())
      << " images_batch: " << images_batch_.size() <<
         " bboxes_gt_scaled_batch_: " << bboxes_gt_scaled_batch_.size();

  // Set up example generator.
  example_generator_->Reset(bbox_prev,
                           bbox_curr,
                           image_prev,
                           image_curr);

  // Make training examples.
  std::vector<cv::Mat> images;
  std::vector<cv::Mat> targets;
  std::vector<BoundingBox> bboxes_gt_scaled;
  MakeTrainingExamples(&images, &targets, &bboxes_gt_scaled);

  while (images.size() > 0) {
    // Compute the number of images left to complete the batch.
    const int num_in_batch = images_batch_.size();
    const int num_left_in_batch = kBatchSize - num_in_batch;

    // The number of images to use is upper-bounded by the number left in the batch.
    // The rest go into the next batch.
    const int num_use = std::min(static_cast<int>(images.size()), num_left_in_batch);

    if (num_use < 0) {
      printf("Error: num_use: %d\n", num_use);
    }

    // Add the approrpriate number of images to the batch.
    images_batch_.insert(images_batch_.end(),
                         images.begin(), images.begin() + num_use);
    targets_batch_.insert(targets_batch_.end(),
                          targets.begin(), targets.begin() + num_use);
    bboxes_gt_scaled_batch_.insert(bboxes_gt_scaled_batch_.end(),
                                   bboxes_gt_scaled.begin(),
                                   bboxes_gt_scaled.begin() + num_use);

    // If we have a full batch, then train!  Otherwise, save this batch for later.
    if (images_batch_.size() == kBatchSize) {
      // Increment the batch count.
      num_batches_++;

      // We have filled up a complete batch, so we should train.
      ProcessBatch();

      // After training, clear the batch.
      images_batch_.clear();
      targets_batch_.clear();
      bboxes_gt_scaled_batch_.clear();

      // Reserve the appropriate amount of space for the next batch.
      images_batch_.reserve(kBatchSize);
      targets_batch_.reserve(kBatchSize);
      bboxes_gt_scaled_batch_.reserve(kBatchSize);
    }

    // Remove the images that were used.
    images.erase(images.begin(), images.begin() + num_use);
    targets.erase(targets.begin(), targets.begin() + num_use);
    bboxes_gt_scaled.erase(bboxes_gt_scaled.begin(), bboxes_gt_scaled.begin() + num_use);
  }
}
