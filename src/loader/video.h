#ifndef VIDEO_H
#define VIDEO_H

#include "helper/bounding_box.h"

// An image frame and corresponding annotation.
struct Frame {
  int frame_num;
  BoundingBox bbox;
};

// Container for video data and the corresponding frame annotations.
class Video {
public:
  // For a given annotation index, get the corresponding frame number, image,
  // and bounding box.
  void LoadAnnotation(const int annotation_index, int* frame_num, cv::Mat* image,
                     BoundingBox* box) const;

  // Find and return the first frame with an annotation in this video.
  void LoadFirstAnnotation(int* first_frame, cv::Mat* image,
                          BoundingBox* box) const;

  // For a given frame num, find an annotation if it exists, and return true.
  // Otherwise return false.
  // If load_only_annotation = false, also load the image.
  // If draw_bounding_box = true, draw the annotation on the image.
  bool LoadFrame(const int frame_num,
                const bool draw_bounding_box,
                const bool load_only_annotation,
                cv::Mat* image,
                BoundingBox* box) const;

  // Show video with all annotations.
  void ShowVideo() const;

  // Path to the folder containing the image files for this video.
  std::string path;

  // Name of all image files for this video (must be appended to path).
  std::vector<std::string> all_frames;

  // Bounding box annotations for a subset of frames in this video.
  // Note that the length of this vector may be different from the length of the vector of image filenames,
  // if only a strict subset of video frames were labeled.
  std::vector<Frame> annotations;

private:
  // For a given frame num, find an annotation if it exists, and return true.
  // Otherwise return false.
  bool FindAnnotation(const int frame_num, BoundingBox* box) const;
};

// A collection of videos.
struct Category {
  std::vector<Video> videos;
};

#endif // VIDEO_H
