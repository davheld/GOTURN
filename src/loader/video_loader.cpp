#include "video_loader.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/helper.h"
#include "helper/high_res_timer.h"
#include "helper/image_proc.h"
#include "loader/loader_alov.h"
#include "loader/loader_vot.h"
#include "loader/loader_imagenet_det.h"
#include "train/example_generator.h"

using std::string;
using std::vector;
namespace bfs = boost::filesystem;

VideoLoader::VideoLoader() {
}

void VideoLoader::ShowVideos() const {
  // Iterate over all videos.
  printf("Showing %zu videos\n", videos_.size());
  for (size_t i = 0; i < videos_.size(); ++i) {
    const Video& video = videos_[i];
    printf("Showing video %zu: %s\n", i, video.path.c_str());

#ifndef NO_DISPLAY
    // Show the video with the annotations.
    video.ShowVideo();

    // Wait for a keystroke.
    cv::waitKey(0);
#endif
  } // For each video
}

void VideoLoader::ShowVideosShift() const {
  // Iterate over all videos.
  printf("Showing %zu videos\n", videos_.size());
  for (size_t video_index = 0; video_index < videos_.size(); ++video_index) {
    const Video& video = videos_[video_index];
    const string& video_path = video.path;
    printf("Showing video %zu: %s\n", video_index, video_path.c_str());

    const std::vector<Frame>& annotations = video.annotations;

    BoundingBox bbox_prev;
    cv::Mat image_prev;

    // Create an example generator with parameters for a large shift, so that this will be noticable.
    // This will be used to artificially shift the crops around the annotations, creating an
    // apparent motion (via translation and scale change).
    ExampleGenerator example_generator(1, 5, -0.4, 0.4);

    // Iterate over all annotations.
    for (size_t frame_index = 0; frame_index < annotations.size(); ++frame_index) {
      const Frame& frame = annotations[frame_index];

      // Load image and bounding box.
      cv::Mat raw_image;
      BoundingBox bbox;
      const bool draw_bounding_box = false;
      const bool load_only_annotation = false;
      video.LoadFrame(frame.frame_num, draw_bounding_box, load_only_annotation,
                     &raw_image, &bbox);

      // If this is the second (or later) bounding box, show the previous target and the current
      // search region (based on the object's previous location), with corresponding annotations.
      if (frame_index > 0) {
        // Draw the annotation on the full image and display it.
        cv::Mat full_image_with_bbox;
        raw_image.copyTo(full_image_with_bbox);
        bbox.DrawBoundingBox(&full_image_with_bbox);

#ifndef NO_DISPLAY
        cv::namedWindow("Raw image", cv::WINDOW_AUTOSIZE);// Create a window for display.
        cv::imshow("Raw image", full_image_with_bbox);                   // Show our image inside it.
#endif
          
        example_generator.Reset(bbox_prev, bbox, image_prev, raw_image);
        example_generator.set_indices(video_index, frame_index);

        cv::Mat image_rand_focus;
        cv::Mat target_pad;
        BoundingBox bbox_gt_scaled;
        const bool visualize = true;
        const int kNumGeneratedExamples = 1;

        for (int k = 0; k < kNumGeneratedExamples; ++k) {
          // Shift the cropped region to generate an apparent motion (translation and scale change) and visualize.
          example_generator.MakeTrainingExampleBBShift(visualize, &image_rand_focus,
                                                       &target_pad, &bbox_gt_scaled);
        }
      }

      bbox_prev = bbox;
      image_prev = raw_image;
    }
  }
}
