#include <string>
#include <iostream>

#include <caffe/caffe.hpp>

#include "example_generator.h"
#include "helper/helper.h"
#include "loader/loader_imagenet_det.h"
#include "loader/rgbd_loader.h"
#include "network/regressor_train.h"
#include "train/tracker_trainer.h"
#include "tracker/tracker_manager.h"

using std::string;

namespace {

// 450,000 batches * (50 images / batch) / (10 images / example) = n examples.
const int kNumIters = 450000 * 5;

// Train on a random image.
void preTrainImage(const LoaderImagenetDet& image_loader,
           const std::vector<std::vector<Annotation> >& images,
           TrackerTrainer* tracker_trainer) {
  // Choose a random image.
  const int image_num = rand() % images.size();

  // Load the image.
  cv::Mat image;
  image_loader.getImage(image_num, &image);

  // Get a random bounding box for this image.
  BoundingBox bbox;
  bbox.rand(image);

  // Train on this example
  tracker_trainer->train(image, image, bbox, bbox);
}

} // namespace

int main (int argc, char *argv[]) {
  if (argc < 13) {
    std::cerr << "Usage: " << argv[0]
              << " videos_folder_imagenet annotations_folder_imagenet"
              << " train_deploy.prototxt network.caffemodel"
              << " mean.binaryproto solver_file"
              << " lambda_shift lambda_scale min_scale max_scale"
              << " gpu_id random_seed"
              << std::endl;
    return 1;
  }

  FLAGS_alsologtostderr = 1;

  ::google::InitGoogleLogging(argv[0]);

  int arg_index = 1;
  const string& images_folder_imagenet      = argv[arg_index++];
  const string& annotations_folder_imagenet = argv[arg_index++];
  const string& caffe_model   = argv[arg_index++];
  const string& deploy_proto   = argv[arg_index++];
  const string& test_proto = argv[arg_index++];
  const string& mean_file    = argv[arg_index++];
  const string& solver_file  = argv[arg_index++];
  const double lambda_shift        = atof(argv[arg_index++]);
  const double lambda_scale        = atof(argv[arg_index++]);
  const double min_scale           = atof(argv[arg_index++]);
  const double max_scale           = atof(argv[arg_index++]);
  const int gpu_id          = atoi(argv[arg_index++]);
  const int random_seed          = atoi(argv[arg_index++]);

  caffe::Caffe::set_random_seed(random_seed); 
  printf("Using random seed: %d\n", random_seed);

#ifdef CPU_ONLY
  printf("Setting up Caffe in CPU mode\n");
  caffe::Caffe::set_mode(caffe::Caffe::CPU);
#else
  printf("Setting up Caffe in GPU mode with ID: %d\n", gpu_id);
  caffe::Caffe::set_mode(caffe::Caffe::GPU);
  caffe::Caffe::SetDevice(gpu_id);
#endif

  // Load the ImageNet data.
  LoaderImagenetDet image_loader(images_folder_imagenet, annotations_folder_imagenet);
  const std::vector<std::vector<Annotation> >& train_images = image_loader.get_images();

  // Create an example generator.
  ExampleGenerator example_generator(lambda_shift, lambda_scale,
                                     min_scale, max_scale);

  printf("Setting up training objects\n");
  RegressorTrain regressor_train(deploy_proto, caffe_model, mean_file,
                                 gpu_id, solver_file);
  regressor_train.set_test_net(test_proto);

  TrackerTrainer tracker_trainer(&example_generator, &regressor_train);
  //TrackerManagerTrainer track_manager(train_videos, &regressor, &tracker_trainer);
  //track_manager.set_use_gt_target(true);

  for (int i = 0; i < kNumIters; ++i) {
    preTrainImage(image_loader, train_images, &tracker_trainer);
  }

  return 0;
}

