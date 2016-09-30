// Visualize the ALOV dataset.

#include <string>
#include <iostream>
#include "loader/loader_alov.h"
#include "loader/loader_vot.h"
#include "loader/loader_imagenet_det.h"

using std::string;

int main (int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " alov_videos_folder alov_annotations_folder"
              << std::endl;
    return 1;
  }

  int arg_index = 1;
  const string& alov_videos_folder      = argv[arg_index++];
  const string& alov_annotations_folder = argv[arg_index++];

  LoaderAlov video_loader(alov_videos_folder, alov_annotations_folder);

  // Different visualization options; uncomment to select the option you would like to use.
  //video_loader.ShowVideos();
  video_loader.ShowVideosShift();

  return 0;
}
