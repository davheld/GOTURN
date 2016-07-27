// Train the neural network tracker.

#include <string>
#include <iostream>

#include <caffe/caffe.hpp>

#include "example_generator.h"
#include "helper/helper.h"
#include "loader/loader_imagenet_det.h"
#include "loader/loader_alov.h"
#include "network/regressor_train.h"
#include "train/tracker_trainer.h"
#include "tracker/tracker_manager.h"
#include "loader/video.h"
#include "loader/video_loader.h"

using std::string;

// Desired number of training batches.
const int kNumBatches = 500000;

namespace {

// Train on a random image.
void train_image(const LoaderImagenetDet& image_loader,
           const std::vector<std::vector<Annotation> >& images,
           TrackerTrainer* tracker_trainer) {
  // Get a random image.
  const int image_num = rand() % images.size();
  const std::vector<Annotation>& annotations = images[image_num];

  // Choose a random annotation.
  const int annotation_num = rand() % annotations.size();

  // Load the image with its ground-truth bounding box.
  cv::Mat image;
  BoundingBox bbox;
  image_loader.LoadAnnotation(image_num, annotation_num, &image, &bbox);

  // Train on this example
  tracker_trainer->Train(image, image, bbox, bbox);
}

// Train on all annotated frames in the set of videos.
void train_video(const std::vector<Video>& videos, TrackerTrainer* tracker_trainer) {
  // Get a random video.
  const int video_num = rand() % videos.size();
  const Video& video = videos[video_num];

  // Get the video's annotations.
  const std::vector<Frame>& annotations = video.annotations;

  // We need at least 2 annotations in this video for this to be useful.
  if (annotations.size() < 2) {
    printf("Error - video %s has only %zu annotations\n", video.path.c_str(),
           annotations.size());
    return;
  }

  // Choose a random annotation.
  const int annotation_index = rand() % (annotations.size() - 1);

  // Load the frame's annotation.
  int frame_num_prev;
  cv::Mat image_prev;
  BoundingBox bbox_prev;
  video.LoadAnnotation(annotation_index, &frame_num_prev, &image_prev, &bbox_prev);

  // Load the next frame's annotation.
  int frame_num_curr;
  cv::Mat image_curr;
  BoundingBox bbox_curr;
  video.LoadAnnotation(annotation_index + 1, &frame_num_curr, &image_curr, &bbox_curr);

  // Train on this example
  tracker_trainer->Train(image_prev, image_curr, bbox_prev, bbox_curr);

  // Save
  frame_num_prev = frame_num_curr;
  image_prev = image_curr;
  bbox_prev = bbox_curr;
}

} // namespace

int main (int argc, char *argv[]) {
  if (argc < 14) {
    std::cerr << "Usage: " << argv[0]
              << " videos_folder_imagenet annotations_folder_imagenet"
              << " alov_videos_folder alov_annotations_folder"
              << " network.caffemodel train.prototxt val.prototxt"
              << " solver_file"
              << " lambda_shift lambda_scale min_scale max_scale"
              << " gpu_id"
              << std::endl;
    return 1;
  }

  FLAGS_alsologtostderr = 1;

  ::google::InitGoogleLogging(argv[0]);

  int arg_index = 1;
  const string& videos_folder_imagenet      = argv[arg_index++];
  const string& annotations_folder_imagenet = argv[arg_index++];
  const string& alov_videos_folder      = argv[arg_index++];
  const string& alov_annotations_folder = argv[arg_index++];
  const string& caffe_model   = argv[arg_index++];
  const string& train_proto   = argv[arg_index++];
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

  // Load the image data.
  LoaderImagenetDet image_loader(videos_folder_imagenet, annotations_folder_imagenet);
  const std::vector<std::vector<Annotation> >& train_images = image_loader.get_images();
  printf("Total training images: %zu\n", train_images.size());

  // Load the video data.
  LoaderAlov alov_video_loader(alov_videos_folder, alov_annotations_folder);
  const bool get_train = true;
  std::vector<Video> train_videos;
  alov_video_loader.get_videos(get_train, &train_videos);
  printf("Total training videos: %zu\n", train_videos.size());

  // Create an ExampleGenerator to generate training examples.
  ExampleGenerator example_generator(lambda_shift, lambda_scale,
                                     min_scale, max_scale);

  // Set up network.
  RegressorTrain regressor_train(train_proto, caffe_model,
                                 gpu_id, solver_file);

  // Set up trainer.
  TrackerTrainer tracker_trainer(&example_generator, &regressor_train);

  // Train tracker.
  while (tracker_trainer.get_num_batches() < kNumBatches) {
    // Train on an image example.
    train_image(image_loader, train_images, &tracker_trainer);

    // Train on a video example.
    train_video(train_videos, &tracker_trainer);
  }

  return 0;
}

