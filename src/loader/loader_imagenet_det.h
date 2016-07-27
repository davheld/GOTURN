#ifndef LOADER_IMAGENET_DET_H
#define LOADER_IMAGENET_DET_H

#include "helper/bounding_box.h"

// An image annotation.
struct Annotation {
  // Relative path of the image files (must be appended to path).
  std::string image_path;

  // Bounding box annotation for an object in this image.
  BoundingBox bbox;

  // Size of image when the annotation was performed.
  int display_width_;
  int display_height_;
};

// Loads images from the ImageNet object detection challenge.
class LoaderImagenetDet
{
public:
  // Load all annotations.
  LoaderImagenetDet(const std::string& image_folder,
                    const std::string& annotations_folder);

  // Load the specified image.
  void LoadImage(const size_t image_num, cv::Mat* image) const;

  // Get the annotation and the image.
  void LoadAnnotation(const size_t image_num,
                      const size_t annotation_num,
                      cv::Mat* image,
                      BoundingBox* bbox) const;

  // Show just the images (no annotations).
  void ShowImages() const;

  // Show all of the annotations on the images.
  void ShowAnnotations() const;

  // Show random images with their annotations.
  void ShowAnnotationsRand() const;

  // Shift the annotations and visualize them (to visualize our
  // artificially shifted training data).
  void ShowAnnotationsShift() const;

  // Compute statistics over bounding box sizes on this dataset.
  void ComputeStatistics() const;

  const std::vector<std::vector<Annotation> >& get_images() {
    return images_;
  }

private:
  // Read the annotation file, convert to bounding box format, and save.
  void LoadAnnotationFile(const std::string& annotation_file,
                          std::vector<Annotation>* image_annotations);

  // Path to the folder containing the image files.
  std::string path_;

  // All annotations for all images.
  std::vector<std::vector<Annotation> > images_;
};

#endif // LOADER_IMAGENET_DET_H
