#ifndef REGRESSOR_TRAIN_H
#define REGRESSOR_TRAIN_H

#include "network/regressor.h"
#include "network/regressor_train_base.h"

class RegressorTrain : public Regressor, public RegressorTrainBase
{
public:
  RegressorTrain(const std::string& deploy_proto,
                 const std::string& caffe_model,
                 const int gpu_id,
                 const std::string& solver_file,
                 const bool do_train);

  RegressorTrain(const std::string& deploy_proto,
                 const std::string& caffe_model,
                 const int gpu_id,
                 const std::string& solver_file);

  // Train the tracker.
  void Train(const std::vector<cv::Mat>& images,
                             const std::vector<cv::Mat>& targets,
                             const std::vector<BoundingBox>& bboxes_gt);

  // Set up the solver with the given test file for validation testing.
  void set_test_net(const std::string& test_proto);

private:
  // Train the network.
  void Step();

  // Set the ground-truth bounding boxes (for training).
  void set_bboxes_gt(const std::vector<BoundingBox>& bboxes_gt);

  boost::shared_ptr<caffe::Net<float> > test_net_;
};

#endif // REGRESSOR_TRAIN_H
