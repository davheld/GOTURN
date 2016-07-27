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
  if (argc < 6) {
    std::cerr << "Usage: " << argv[0]
              << " videos_folder deploy.prototxt network.caffemodel"
              << " output_folder gpu_id" << std::endl;
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  string videos_folder          = argv[1];
  string test_proto             = argv[2];
  string caffe_model           = argv[3];
  string output_folder          = argv[4];
  int gpu_id                    = atoi(argv[5]);

  boost::filesystem::create_directories(output_folder);

  const bool do_train = false;
  Regressor regressor(test_proto, caffe_model, gpu_id, do_train);

  // Time how long tracking takes.
  HighResTimer hrt_total("Total evaluation (including loading videos)");
  hrt_total.start();

  // Get videos.
  std::vector<Video> videos;
  LoaderVOT loader(videos_folder);
  videos = loader.get_videos();

  // Create a tracker object.
  const bool show_intermediate_output = false;
  Tracker tracker(show_intermediate_output);

  // Track all objects in all videos and save the output.
  const bool save_videos = true;
  TrackerTesterAlov tracker_tester(videos, save_videos, &regressor, &tracker, output_folder);
  tracker_tester.TrackAll();

  // Print the timing information.
  hrt_total.stop();
  hrt_total.print();

  return 0;
}
