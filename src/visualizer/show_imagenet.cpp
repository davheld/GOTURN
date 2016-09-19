// Visualize the ImageNet dataset.

#include "loader/loader_imagenet_det.h"
#include <iostream>
#include <string>

using std::string;

int main (int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " alov_videos_folder alov_annotations_folder"
              << std::endl;
    return 1;
  }

  int arg_index = 1;
  const string& videos_folder      = argv[arg_index++];
  const string& annotations = argv[arg_index++];

  LoaderImagenetDet loader(videos_folder, annotations);

  // Different visualization options; uncomment to select the option you would like to use.
  //loader.ShowImages();
  //loader.ShowAnnotations();
  //loader.ShowAnnotationsRand();
  loader.ShowAnnotationsShift();

  return 0;
}
