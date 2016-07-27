#ifndef TRACKER_MANAGER_H
#define TRACKER_MANAGER_H

#include "network/regressor.h"
#include "tracker/tracker.h"
#include "loader/video.h"
#include "helper/high_res_timer.h"

// Manage the iteration over all videos and tracking the objects inside.
class TrackerManager
{
public:
  TrackerManager(const std::vector<Video>& videos,
                 RegressorBase* regressor, Tracker* tracker);

  // Iterate over all videos and track the target object in each.
  void TrackAll() ;

  // Iterate over all videos and track the target object in each.
  // Start tracking the video with index start_video_num (for debugging).
  // pause_val is normally ignored.
  void TrackAll(const size_t start_video_num, const int pause_val);

  // Functions for subclasses that get called at appropriate times.
  virtual void VideoInit(const Video& video, const size_t video_num) {}

  // Called immediately before estimating the current location of the target object.
  virtual void SetupEstimate() {}

  // Called immediately after estimating the current location of the target object.
  virtual void ProcessTrackOutput(
      const size_t frame_num, const cv::Mat& image_curr, const bool has_annotation,
      const BoundingBox& bbox_gt, const BoundingBox& bbox_estimate_uncentered,
      const int pause_val) {}

  // Called after finishing tracking a video. (Used by subclasses)
  virtual void PostProcessVideo() {}

  // Called after finishing tracking all videos. (Used by subclasses)
  virtual void PostProcessAll() {}

protected:
  // Videos to track.
  const std::vector<Video>& videos_;

  // Neural network tracker.
  RegressorBase* regressor_;

  // Tracker.
  Tracker* tracker_;
};

// Track objects and visualize the tracker output.
class TrackerVisualizer : public TrackerManager
{
public:
  TrackerVisualizer(const std::vector<Video>& videos,
                    RegressorBase* regressor, Tracker* tracker);

  // Print which video is being visualized.
  virtual void VideoInit(const Video& video, const size_t video_num);

  // Show the tracking estimate and the ground-truth target location.
  virtual void ProcessTrackOutput(
      const size_t frame_num, const cv::Mat& image_curr, const bool has_annotation,
      const BoundingBox& bbox_gt, const BoundingBox& bbox_estimate,
      const int pause_val);
};

// Save tracking output and video; record timing.
class TrackerTesterAlov : public TrackerManager
{
public:
  TrackerTesterAlov(const std::vector<Video>& videos,
                    const bool save_videos,
                    RegressorBase* regressor, Tracker* tracker,
                    const std::string& output_folder);

  // Set up folder to save tracking output to a video.
  virtual void VideoInit(const Video& video, const size_t video_num);

  // Record the time before starting to track.
  virtual void SetupEstimate();

  // Record timing info and save the tracking output.
  virtual void ProcessTrackOutput(
      const size_t frame_num, const cv::Mat& image_curr, const bool has_annotation,
      const BoundingBox& bbox_gt, const BoundingBox& bbox_estimate,
      const int pause_val);

  // Close the file that saves the tracking data.
  virtual void PostProcessVideo();

  virtual void PostProcessAll();

private:
  // Folder to save all tracking output.
  std::string output_folder_;

  // File for saving tracking output coordinates (for evaluation).
  FILE* output_file_ptr_;

  // Timer.
  HighResTimer hrt_;

  // Total time used for tracking (Other time is used to save the tracking
  // output to a video and to write tracking data to a file for evaluation purposes).
  double total_ms_;

  // Number of frames tracked.
  int num_frames_;

  // Used to save tracking visualization data.
  cv::VideoWriter video_writer_;

  // Whether to save tracking videos.  Videos take up a lot of space, so use this only when needed.
  bool save_videos_;
};


#endif // TRACKER_MANAGER_H
