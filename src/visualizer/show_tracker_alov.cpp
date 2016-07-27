// Visualize the tracker performance.

#include <string>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "network/regressor.h"
#include "loader/loader_alov.h"
#include "loader/loader_vot.h"
#include "tracker/tracker.h"
#include "tracker/tracker_manager.h"

using std::string;

// Set to true to show more detailed tracking visualizations.
const bool show_intermediate_output = false;

int main (int argc, char *argv[]) {
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0]
              << " deploy.prototxt network.caffemodel videos_folder annotations_folder"
              << " [video_num] [pauseval] [gpu_id]" << std::endl;
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  const string& model_file   = argv[1];
  const string& trained_file = argv[2];
  const string& videos_folder = argv[3];
  const string& annotations_folder = argv[4];

  int gpu_id = 0;
  if (argc >= 6) {
    gpu_id = atoi(argv[5]);
  }

  int start_video_num = 0;
  if (argc >= 7) {
    start_video_num = atoi(argv[6]);
  }

  int pause_val = 1;
  if (argc >= 8) {
    pause_val = atoi(argv[7]);
  }

  // Set up the neural network.
  const bool do_train = false;
  Regressor regressor(model_file, trained_file, gpu_id, do_train);

  Tracker tracker(show_intermediate_output);

  // Get videos.
  std::vector<Video> videos;
  LoaderAlov loader(videos_folder, annotations_folder);
  const bool get_train = false;
  loader.get_videos(get_train, &videos);

  // Visualize the tracker performance.
  TrackerVisualizer tracker_visualizer(videos, &regressor, &tracker);
  tracker_visualizer.TrackAll(start_video_num, pause_val);

  return 0;
}
