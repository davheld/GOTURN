#include "video.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

void Video::ShowVideo() const {
  const string& video_path = path;

  const vector<string>& image_files = all_frames;

  int annotated_frame_index = 0;

  // For the 0th annotation in this video, get the start and end frames.
  const int start_frame = annotations[0].frame_num;
  const int end_frame = annotations[annotations.size() - 1].frame_num;

  // Iterate over all frames in this video.
  for (size_t image_frame_num = start_frame; image_frame_num <= end_frame; ++image_frame_num) {
    // Load the image.
    const string& image_file = video_path + "/" + image_files[image_frame_num];
    cv::Mat image = cv::imread(image_file);

    // Get the frame number for the next annotation.
    const int annotated_frame_num = annotations[annotated_frame_index].frame_num;

    bool has_bounding_box = false;

    // Check if the annotation frame number corresponds to the image frame number.
    if (annotated_frame_num == image_frame_num) {
      // Draw the annotation on the image.
      const BoundingBox& box = annotations[annotated_frame_index].bbox;
      box.DrawBoundingBox(&image);
      has_bounding_box = true;

      // Incremrent the annotation index.
      if (annotated_frame_index < annotations.size() - 1) {
        annotated_frame_index++;
      }
    }

    // Show the image with the annotation.
    if(!image.data ) {
      printf("Could not open or find image %s\n", image_file.c_str());
    } else {
#ifndef NO_DISPLAY
      cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
      cv::imshow( "Display window", image );                   // Show our image inside it.
      cv::waitKey(1);                                          // Wait for a keystroke in the window
#endif
    }
  } // For all frames in video
}

void Video::LoadFirstAnnotation(int* first_frame, cv::Mat* image,
                               BoundingBox* box) const {
  LoadAnnotation(0, first_frame, image, box);
}

void Video::LoadAnnotation(const int annotation_index,
                          int* frame_num,
                          cv::Mat* image,
                          BoundingBox* box) const {
  // Get the annotation corresponding to this index.
  const Frame& annotated_frame = annotations[annotation_index];

  // Get the frame number corresponding to this annotation.
  *frame_num = annotated_frame.frame_num;

  *box = annotated_frame.bbox;

  const string& video_path = path;
  const vector<string>& image_files = all_frames;

  if (image_files.empty()) {
    printf("Error - no image files for video at path: %s\n", path.c_str());
    return;
  } else if (*frame_num >= image_files.size()) {
    printf("Cannot find frame: %d; only %zu image files were found at %s\n", *frame_num, image_files.size(), path.c_str());
    return;
  }

  // Load the image corresponding to this annotation.
  const string& image_file = video_path + "/" + image_files[*frame_num];
  *image = cv::imread(image_file);

  if (!image->data) {
    printf("Could not find file: %s\n", image_file.c_str());
  }
}

bool Video::FindAnnotation(const int frame_num, BoundingBox* box) const {
  // Iterate over all annotations.
  for (size_t i = 0; i < annotations.size(); ++i) {
    const Frame& frame = annotations[i];

    // Check if the annotation frame matches the desired frame.
    if (frame.frame_num == frame_num) {
      // If we found a match, return the corresponding annotation.
      *box = frame.bbox;
      return true;
    }
  }

  return false;
}

bool Video::LoadFrame(const int frame_num, const bool draw_bounding_box,
                     const bool load_only_annotation, cv::Mat* image,
                     BoundingBox* box) const {
  const string& video_path = path;
  const vector<string>& image_files = all_frames;

  // Load the image for this frame.
  if (!load_only_annotation) {
    const string& image_file = video_path + "/" + image_files[frame_num];
    *image = cv::imread(image_file);
  }

  // Find the annotation (if it exists) for the desired frame_num.
  const bool has_annotation = FindAnnotation(frame_num, box);

  // Draw the annotation (if it exists) on the image.
  if (!load_only_annotation && has_annotation && draw_bounding_box) {
    box->DrawBoundingBox(image);
  }

  // Return whether we found an annotation for this frame.
  return has_annotation;
}
