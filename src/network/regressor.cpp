#include "regressor.h"

#include "helper/high_res_timer.h"

// Credits:
// This file was mostly taken from:
// https://github.com/BVLC/caffe/tree/master/examples/cpp_classification

using caffe::Blob;
using caffe::Net;
using std::string;

// We need 2 inputs: one for the current frame and one for the previous frame.
const int kNumInputs = 2;

Regressor::Regressor(const string& deploy_proto,
                     const string& caffe_model,
                     const int gpu_id,
                     const int num_inputs,
                     const bool do_train)
  : num_inputs_(num_inputs),
    caffe_model_(caffe_model),
    modified_params_(false)

{
  SetupNetwork(deploy_proto, caffe_model, gpu_id, do_train);
}

Regressor::Regressor(const string& deploy_proto,
                     const string& caffe_model,
                     const int gpu_id,
                     const bool do_train)
  : num_inputs_(kNumInputs),
    caffe_model_(caffe_model),
    modified_params_(false)
{
  SetupNetwork(deploy_proto, caffe_model, gpu_id, do_train);
}

void Regressor::SetupNetwork(const string& deploy_proto,
                             const string& caffe_model,
                             const int gpu_id,
                             const bool do_train) {
#ifdef CPU_ONLY
  printf("Setting up Caffe in CPU mode\n");
  caffe::Caffe::set_mode(caffe::Caffe::CPU);
#else
  printf("Setting up Caffe in GPU mode with ID: %d\n", gpu_id);
  caffe::Caffe::SetDevice(gpu_id);
  caffe::Caffe::set_mode(caffe::Caffe::GPU);
#endif

  if (do_train) {
    printf("Setting phase to train\n");
    net_.reset(new Net<float>(deploy_proto, caffe::TRAIN));
  } else {
    printf("Setting phase to test\n");
    net_.reset(new Net<float>(deploy_proto, caffe::TEST));
  }

  if (caffe_model != "NONE") {
    net_->CopyTrainedLayersFrom(caffe_model_);
  } else {
    printf("Not initializing network from pre-trained model\n");
  }

  //CHECK_EQ(net_->num_inputs(), num_inputs_) << "Network should have exactly " << num_inputs_ << " inputs.";
  CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

  Blob<float>* input_layer = net_->input_blobs()[0];

  printf("Network image size: %d, %d\n", input_layer->width(), input_layer->height());

  num_channels_ = input_layer->channels();
  CHECK(num_channels_ == 3 || num_channels_ == 1)
    << "Input layer should have 1 or 3 channels.";
  input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

  // Load the binaryproto mean file.
  SetMean();
}

void Regressor::SetMean() {
  // Set the mean image.
  mean_ = cv::Mat(input_geometry_, CV_32FC3, cv::Scalar(104, 117, 123));
}

void Regressor::Init() {
  if (modified_params_ ) {
    printf("Reloading new params\n");
    net_->CopyTrainedLayersFrom(caffe_model_);
    modified_params_ = false;
  }
}

void Regressor::Regress(const cv::Mat& image_curr,
                        const cv::Mat& image, const cv::Mat& target,
                        BoundingBox* bbox) {
  assert(net_->phase() == caffe::TEST);

  // Estimate the bounding box location of the target object in the current image.
  std::vector<float> estimation;
  Estimate(image, target, &estimation);

  // Wrap the estimation in a bounding box object.
  *bbox = BoundingBox(estimation);
}

void Regressor::Estimate(const cv::Mat& image, const cv::Mat& target, std::vector<float>* output) {
  assert(net_->phase() == caffe::TEST);

  // Reshape the input blobs to be the appropriate size.
  Blob<float>* input_target = net_->input_blobs()[0];
  input_target->Reshape(1, num_channels_,
                       input_geometry_.height, input_geometry_.width);

  Blob<float>* input_image = net_->input_blobs()[1];
  input_image->Reshape(1, num_channels_,
                       input_geometry_.height, input_geometry_.width);

  Blob<float>* input_bbox = net_->input_blobs()[2];
  input_bbox->Reshape(1, 4, 1, 1);

  // Forward dimension change to all layers.
  net_->Reshape();

  // Process the inputs so we can set them.
  std::vector<cv::Mat> target_channels;
  std::vector<cv::Mat> image_channels;
  WrapInputLayer(&target_channels, &image_channels);

  // Set the inputs to the network.
  Preprocess(image, &image_channels);
  Preprocess(target, &target_channels);

  // Perform a forward-pass in the network.
  net_->ForwardPrefilled();

  // Get the network output.
  GetOutput(output);
}

void Regressor::ReshapeImageInputs(const size_t num_images) {
  // Reshape the input blobs to match the given size and geometry.
  Blob<float>* input_target = net_->input_blobs()[0];
  input_target->Reshape(num_images, num_channels_,
                       input_geometry_.height, input_geometry_.width);

  Blob<float>* input_image = net_->input_blobs()[1];
  input_image->Reshape(num_images, num_channels_,
                       input_geometry_.height, input_geometry_.width);
}

void Regressor::GetFeatures(const string& feature_name, std::vector<float>* output) const {
  //printf("Getting %s features\n", feature_name.c_str());

  // Get a pointer to the requested layer.
  const boost::shared_ptr<Blob<float> > layer = net_->blob_by_name(feature_name.c_str());

  // Compute the number of elements in this layer.
  int num_elements = 1;
  for (int i = 0; i < layer->num_axes(); ++i) {
    const int elements_in_dim = layer->shape(i);
    //printf("Layer %d: %d\n", i, elements_in_dim);
    num_elements *= elements_in_dim;
  }
  //printf("Total num elements: %d\n", num_elements);

  // Copy all elements in this layer to a vector.
  const float* begin = layer->cpu_data();
  const float* end = begin + num_elements;
  *output = std::vector<float>(begin, end);
}

void Regressor::SetImages(const std::vector<cv::Mat>& images,
                           const std::vector<cv::Mat>& targets) {
  if (images.size() != targets.size()) {
    printf("Error - %zu images but %zu targets\n", images.size(), targets.size());
  }

  const size_t num_images = images.size();

  // Set network inputs to the appropriate size and number.
  ReshapeImageInputs(num_images);

  // Wrap the network inputs with opencv objects.
  std::vector<std::vector<cv::Mat> > target_channels;
  std::vector<std::vector<cv::Mat> > image_channels;
  WrapInputLayer(num_images, &target_channels, &image_channels);

  // Set the network inputs appropriately.
  Preprocess(images, &image_channels);
  Preprocess(targets, &target_channels);
}

void Regressor::Estimate(const std::vector<cv::Mat>& images,
                        const std::vector<cv::Mat>& targets,
                        std::vector<float>* output) {
  assert(net_->phase() == caffe::TEST);

  // Set the inputs to the network.
  SetImages(images, targets);

  // Forward dimension change to all layers.
  net_->Reshape();

  // Perform a forward-pass in the network.
  net_->ForwardPrefilled();

  // Get the network output.
  GetOutput(output);
}

void Regressor::GetOutput(std::vector<float>* output) {
  // Get the fc8 output features of the network (this contains the estimated bounding box).
  GetFeatures("fc8", output);
}

// Wrap the input layer of the network in separate cv::Mat objects
// (one per channel). This way we save one memcpy operation and we
// don't need to rely on cudaMemcpy2D. The last preprocessing
// operation will write the separate channels directly to the input
// layer.
void Regressor::WrapInputLayer(std::vector<cv::Mat>* target_channels, std::vector<cv::Mat>* image_channels) {
  Blob<float>* input_layer_target = net_->input_blobs()[0];
  Blob<float>* input_layer_image = net_->input_blobs()[1];

  int target_width = input_layer_target->width();
  int target_height = input_layer_target->height();
  float* target_data = input_layer_target->mutable_cpu_data();
  for (int i = 0; i < input_layer_target->channels(); ++i) {
    cv::Mat channel(target_height, target_width, CV_32FC1, target_data);
    target_channels->push_back(channel);
    target_data += target_width * target_height;
  }

  int image_width = input_layer_image->width();
  int image_height = input_layer_image->height();
  float* image_data = input_layer_image->mutable_cpu_data();
  for (int i = 0; i < input_layer_image->channels(); ++i) {
    cv::Mat channel(image_height, image_width, CV_32FC1, image_data);
    image_channels->push_back(channel);
    image_data += image_width * image_height;
  }
}

// Wrap the input layer of the network in separate cv::Mat objects
// (one per channel). This way we save one memcpy operation and we
// don't need to rely on cudaMemcpy2D. The last preprocessing
// operation will write the separate channels directly to the input
// layer.
void Regressor::WrapInputLayer(const size_t num_images,
                               std::vector<std::vector<cv::Mat> >* target_channels,
                               std::vector<std::vector<cv::Mat> >* image_channels) {
  Blob<float>* input_layer_target = net_->input_blobs()[0];
  Blob<float>* input_layer_image = net_->input_blobs()[1];

  image_channels->resize(num_images);
  target_channels->resize(num_images);

  int target_width = input_layer_target->width();
  int target_height = input_layer_target->height();
  float* target_data = input_layer_target->mutable_cpu_data();
  for (int n = 0; n < num_images; ++n) {
    for (int i = 0; i < input_layer_target->channels(); ++i) {
      cv::Mat channel(target_height, target_width, CV_32FC1, target_data);
      (*target_channels)[n].push_back(channel);
      target_data += target_width * target_height;
    }
  }

  int image_width = input_layer_image->width();
  int image_height = input_layer_image->height();
  float* image_data = input_layer_image->mutable_cpu_data();
  for (int n = 0; n < num_images; ++n) {
    for (int i = 0; i < input_layer_image->channels(); ++i) {
      cv::Mat channel(image_height, image_width, CV_32FC1, image_data);
      (*image_channels)[n].push_back(channel);
      image_data += image_width * image_height;
    }
  }
}

void Regressor::Preprocess(const cv::Mat& img,
                            std::vector<cv::Mat>* input_channels) {
  // Convert the input image to the input image format of the network.
  cv::Mat sample;
  if (img.channels() == 3 && num_channels_ == 1)
    cv::cvtColor(img, sample, CV_BGR2GRAY);
  else if (img.channels() == 4 && num_channels_ == 1)
    cv::cvtColor(img, sample, CV_BGRA2GRAY);
  else if (img.channels() == 4 && num_channels_ == 3)
    cv::cvtColor(img, sample, CV_BGRA2BGR);
  else if (img.channels() == 1 && num_channels_ == 3)
    cv::cvtColor(img, sample, CV_GRAY2BGR);
  else
    sample = img;

  // Convert the input image to the expected size.
  cv::Mat sample_resized;
  if (sample.size() != input_geometry_)
    cv::resize(sample, sample_resized, input_geometry_);
  else
    sample_resized = sample;

  // Convert the input image to the expected number of channels.
  cv::Mat sample_float;
  if (num_channels_ == 3)
    sample_resized.convertTo(sample_float, CV_32FC3);
  else
    sample_resized.convertTo(sample_float, CV_32FC1);

  // Subtract the image mean to try to make the input 0-mean.
  cv::Mat sample_normalized;
  cv::subtract(sample_float, mean_, sample_normalized);

  // This operation will write the separate BGR planes directly to the
  // input layer of the network because it is wrapped by the cv::Mat
  // objects in input_channels.
  cv::split(sample_normalized, *input_channels);

  /*CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
        == net_->input_blobs()[0]->cpu_data())
    << "Input channels are not wrapping the input layer of the network.";*/
}

void Regressor::Preprocess(const std::vector<cv::Mat>& images,
                           std::vector<std::vector<cv::Mat> >* input_channels) {
  for (size_t i = 0; i < images.size(); ++i) {
    const cv::Mat& img = images[i];

    // Convert the input image to the input image format of the network.
    cv::Mat sample;
    if (img.channels() == 3 && num_channels_ == 1)
      cv::cvtColor(img, sample, CV_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
      cv::cvtColor(img, sample, CV_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels_ == 3)
      cv::cvtColor(img, sample, CV_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
      cv::cvtColor(img, sample, CV_GRAY2BGR);
    else
      sample = img;

    // Convert the input image to the expected size.
    cv::Mat sample_resized;
    if (sample.size() != input_geometry_)
      cv::resize(sample, sample_resized, input_geometry_);
    else
      sample_resized = sample;

    // Convert the input image to the expected number of channels.
    cv::Mat sample_float;
    if (num_channels_ == 3)
      sample_resized.convertTo(sample_float, CV_32FC3);
    else
      sample_resized.convertTo(sample_float, CV_32FC1);

    // Subtract the image mean to try to make the input 0-mean.
    cv::Mat sample_normalized;
    cv::subtract(sample_float, mean_, sample_normalized);

    // This operation will write the separate BGR planes directly to the
    // input layer of the network because it is wrapped by the cv::Mat
    // objects in input_channels.
    cv::split(sample_normalized, (*input_channels)[i]);

    /*CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
          == net_->input_blobs()[0]->cpu_data())
      << "Input channels are not wrapping the input layer of the network.";*/
  }
}
