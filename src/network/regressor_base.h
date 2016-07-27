#ifndef REGRESSOR_BASE_H
#define REGRESSOR_BASE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/shared_ptr.hpp>

#include <caffe/caffe.hpp>

class BoundingBox;

// A neural network for the tracker must inherit from this class.
class RegressorBase
{
public:
  RegressorBase();

  // Predict the bounding box.
  // image_curr is the entire current image.
  // image is the best guess as to a crop of the current image that likely contains the target object.
  // target is an image of the target object from the previous frame.
  // Returns: bbox, an estimated location of the target object in the current image.
  virtual void Regress(const cv::Mat& image_curr, const cv::Mat& image, const cv::Mat& target, BoundingBox* bbox) = 0;

  // Called at the beginning of tracking a new object to initialize the network.
  virtual void Init() { }

  //virtual boost::shared_ptr<caffe::Net<float> > get_net() { return net_; }

protected:
  boost::shared_ptr<caffe::Net<float> > net_;

};

#endif // REGRESSOR_BASE_H
