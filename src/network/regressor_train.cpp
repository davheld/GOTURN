#include "regressor_train.h"

const int kNumInputs = 3;
const bool kDoTrain = true;

using std::string;
using std::vector;
using caffe::Blob;

RegressorTrain::RegressorTrain(const std::string& deploy_proto,
                               const std::string& caffe_model,
                               const int gpu_id,
                               const string& solver_file,
                               const bool do_train)
  : Regressor(deploy_proto, caffe_model, gpu_id, kNumInputs, do_train),
    RegressorTrainBase(solver_file)
{
  solver_.set_net(net_);
}

RegressorTrain::RegressorTrain(const std::string& deploy_proto,
                               const std::string& caffe_model,
                               const int gpu_id,
                               const string& solver_file)
  : Regressor(deploy_proto, caffe_model, gpu_id, kNumInputs, kDoTrain),
    RegressorTrainBase(solver_file)
{
  solver_.set_net(net_);
}

void RegressorTrain::set_test_net(const std::string& test_proto) {
  printf("Setting test net to: %s\n", test_proto.c_str());
  test_net_.reset(new caffe::Net<float>(test_proto, caffe::TEST));
  solver_.set_test_net(test_net_);
}

void RegressorTrain::set_bboxes_gt(const std::vector<BoundingBox>& bboxes_gt) {
  assert(net_->phase() == caffe::TRAIN);

  // Reshape the bbox.
  Blob<float>* input_bbox = net_->input_blobs()[2];
  const size_t num_images = bboxes_gt.size();
  const int bbox_dims = 4;
  vector<int> shape;
  shape.push_back(num_images);
  shape.push_back(bbox_dims);
  input_bbox->Reshape(shape);

  // Get a pointer to the bbox memory.
  float* input_bbox_data = input_bbox->mutable_cpu_data();

  int input_bbox_data_counter = 0;
  for (size_t i = 0; i < bboxes_gt.size(); ++i) {
    const BoundingBox& bbox_gt = bboxes_gt[i];

    // Set the bbox data to the ground-truth bbox.
    std::vector<float> bbox_vect;
    bbox_gt.GetVector(&bbox_vect);
    for (size_t j = 0; j < 4; ++j) {
      input_bbox_data[input_bbox_data_counter] = bbox_vect[j];
      input_bbox_data_counter++;
    }
  }
}

void RegressorTrain::Train(const std::vector<cv::Mat>& images,
                           const std::vector<cv::Mat>& targets,
                           const std::vector<BoundingBox>& bboxes_gt) {
  assert(net_->phase() == caffe::TRAIN);

  if (images.size() != targets.size()) {
    printf("Error - %zu images but %zu targets\n", images.size(), targets.size());
  }

  if (images.size() != bboxes_gt.size()) {
    printf("Error - %zu images but %zu bboxes_gt", images.size(), bboxes_gt.size());
  }

  // Normally to track we just estimate the bbox location; if we need to backprop,
  // we also need to input the ground-truth bounding boxes.
  set_bboxes_gt(bboxes_gt);

  // Set the image and target.
  SetImages(images, targets);

  // Train the network.
  Step();
}

void RegressorTrain::Step() {
  assert(net_->phase() == caffe::TRAIN);

  // Train the network.
  solver_.Step(1);
}

