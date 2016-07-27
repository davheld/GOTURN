#ifndef REGRESSOR_H
#define REGRESSOR_H

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "helper/bounding_box.h"
#include "network/regressor_base.h"

class Regressor : public RegressorBase {
 public:
  // Set up a network with the architecture specified in deploy_proto,
  // with the model weights saved in caffe_model.
  // If we are using a model with a
  Regressor(const std::string& train_deploy_proto,
            const std::string& caffe_model,
            const int gpu_id,
            const int num_inputs,
            const bool do_train);

  Regressor(const std::string& train_deploy_proto,
            const std::string& caffe_model,
            const int gpu_id,
            const bool do_train);

  // Estimate the location of the target object in the current image.
  // image_curr is the entire current image.
  // image is the best guess as to a crop of the current image that likely contains the target object.
  // target is an image of the target object from the previous frame.
  // Returns: bbox, an estimated location of the target object in the current image.
  virtual void Regress(const cv::Mat& image_curr, const cv::Mat& image, const cv::Mat& target, BoundingBox* bbox);

protected:
  // Set the network inputs.
  void SetImages(const std::vector<cv::Mat>& images,
                 const std::vector<cv::Mat>& targets);

  // Get the features corresponding to the output of the network.
  virtual void GetOutput(std::vector<float>* output);

  // Reshape the image inputs to the network to match the expected size and number of images.
  virtual void ReshapeImageInputs(const size_t num_images);

  // Get the features in the network with the given name, and copy their values to the output.
  void GetFeatures(const std::string& feature_name, std::vector<float>* output) const;

  // Pass the image and the target to the network; estimate the location of the target in the current image.
  void Estimate(const cv::Mat& image, const cv::Mat& target, std::vector<float>* output);

  // Batch estimation, for tracking multiple targets.
  void Estimate(const std::vector<cv::Mat>& images,
                             const std::vector<cv::Mat>& targets,
                             std::vector<float>* output);

  // Wrap the input layer of the network in separate cv::Mat objects
  // (one per channel).
  void WrapInputLayer(std::vector<cv::Mat>* target_channels, std::vector<cv::Mat>* image_channels);

  // Wrap the input layer of the network in separate cv::Mat objects
  // (one per channel per image, for num_images images).
  void WrapInputLayer(const size_t num_images,
                      std::vector<std::vector<cv::Mat> >* target_channels,
                      std::vector<std::vector<cv::Mat> >* image_channels);

  // Set the inputs to the network.
  void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);
  void Preprocess(const std::vector<cv::Mat>& images,
                  std::vector<std::vector<cv::Mat> >* input_channels);

  // If the parameters of the network have been modified, reinitialize the parameters to their original values.
  virtual void Init();

 private:
  // Set up a network with the architecture specified in deploy_proto,
  // with the model weights saved in caffe_model.
  void SetupNetwork(const std::string& deploy_proto,
                    const std::string& caffe_model,
                    const int gpu_id,
                    const bool do_train);

  // Set the mean input (used to normalize the inputs to be 0-mean).
  void SetMean();

 private:
  // Number of inputs expected by the network.
  int num_inputs_;

  // Size of the input images.
  cv::Size input_geometry_;

  // Number of image channels: normally either 1 (black and white) or 3 (color).
  int num_channels_;

  // Mean image, used to make the input 0-mean.
  cv::Mat mean_;

  // Folder containing the model parameters.
  std::string caffe_model_;

  // Whether the model weights has been modified.
  bool modified_params_;
};

#endif // REGRESSOR_H
