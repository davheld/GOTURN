#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H


#include <vector>
#include <string>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/bounding_box.h"
#include "loader/video.h"
#include "loader/loader_imagenet_det.h"

class VideoLoader
{
public:
  VideoLoader();

  // Show all videos with the annotations.
  // Pause after each video; press any key to continue.
  void ShowVideos() const;

  // Shift the annotations and visualize them (to visualize our
  // artificially shifted training data).
  void ShowVideosShift() const;

  std::vector<Video> get_videos() const { return videos_; }

protected:
  std::vector<Video> videos_;
};

#endif // VIDEO_LOADER_H

