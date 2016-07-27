// This file was mostly taken from the example given here:
// http://www.votchallenge.net/howto/integration.html

// Uncomment line below if you want to use rectangles
#define VOT_RECTANGLE
#include "native/vot.h"

#include "tracker/tracker.h"
#include "network/regressor_train.h"

//using std::string;

int main (int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " deploy.prototxt network.caffemodel"
              << " [gpu_id]" << std::endl;
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  const string& model_file   = argv[1];
  const string& trained_file = argv[2];

  int gpu_id = 0;
  if (argc >= 4) {
    gpu_id = atoi(argv[3]);
  }

  const bool do_train = false;
  Regressor regressor(model_file, trained_file, gpu_id, do_train);

  // Ensuring randomness for fairness.
  srandom(time(NULL));

  // Create a tracker object.
  const bool show_intermediate_output = false;
  Tracker tracker(show_intermediate_output);

  VOT vot; // Initialize the communcation

  // Get region and first frame
  VOTRegion region = vot.region();
  string path = vot.frame();

  // Load the first frame and use the initialization region to initialize the tracker.
  tracker.Init(path, region, &regressor);

  //track
  while (true) {
      path = vot.frame(); // Get the next frame
      if (path.empty()) break; // Are we done?

      // Load current image.
      const cv::Mat& image = cv::imread(path);

      // Track and estimate the bounding box location.
      BoundingBox bbox_estimate;
      tracker.Track(image, &regressor, &bbox_estimate);

      bbox_estimate.GetRegion(&region);

      vot.report(region); // Report the position of the tracker
  }

  // Finishing the communication is completed automatically with the destruction
  // of the communication object (if you are using pointers you have to explicitly
  // delete the object).

  return 0;
}
