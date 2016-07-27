#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/high_res_timer.h"
#include "network/regressor.h"
#include "loader/loader_alov.h"
#include "loader/loader_vot.h"
#include "tracker/tracker.h"
#include "tracker/tracker_manager.h"

using std::string;

int main (int argc, char *argv[]) {
  if (argc < 9) {
    std::cerr << "Usage: " << argv[0]
              << " videos_folder annotations_folder deploy.prototxt network.caffemodel"
              << " outputfolder use_train save_videos gpu_id" << std::endl;
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  string videos_folder          = argv[1];
  string annotations_folder     = argv[2];
  string test_proto             = argv[3];
  string caffe_model           = argv[4];
  string output_folder          = argv[5];
  const bool use_train          = atoi(argv[6]);
  const bool save_videos        = atoi(argv[7]);
  int gpu_id                    = atoi(argv[8]);

  boost::filesystem::create_directories(output_folder);

  const bool do_train = false;
  Regressor regressor(test_proto, caffe_model, gpu_id, do_train);

  // Time how long tracking takes.
  HighResTimer hrt_total("Total evaluation (including loading videos)");
  hrt_total.start();

  // Get videos.
  std::vector<Video> videos;
  LoaderAlov loader(videos_folder, annotations_folder);
  loader.get_videos(use_train, &videos);

  // Create a tracker object.
  const bool show_intermediate_output = false;
  Tracker tracker(show_intermediate_output);

  // Track all objects in all videos.
  TrackerTesterAlov tracker_tester(videos, save_videos, &regressor, &tracker, output_folder);
  tracker_tester.TrackAll();

  // Print the timing information.
  hrt_total.stop();
  hrt_total.print();

  return 0;
}
