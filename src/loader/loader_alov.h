#ifndef LOADER_ALOV_H
#define LOADER_ALOV_H

#include <vector>
#include <string>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/bounding_box.h"
#include "video.h"
#include "video_loader.h"

// Loads annotations from the ALOV tracking dataset.
class LoaderAlov : public VideoLoader
{
public:
  // Loads all annotations.
  LoaderAlov(const std::string& images, const std::string& annotations);

  // If get_train is true, get the videos in the training set; otherwise,
  // get the videos in the validation set.
  // The training videos (as well as the validation videos) are taken
  // with the same proportion from each category.
  void get_videos(const bool get_train, std::vector<Video>* videos) const;

private:  
  // Make this method private to ensure it doesn't get called.
  const std::vector<Video>& get_videos() const { return videos_; }

  // The videos are organized into categories.
  std::vector<Category> categories_;
};

#endif // LOADER_ALOV_H
