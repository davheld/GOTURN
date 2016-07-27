#include "loader_vot.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/helper.h"

using std::string;
using std::vector;
namespace bfs = boost::filesystem;

const bool kDoTest = false;

LoaderVOT::LoaderVOT(const std::string& vot_folder)
{
  if (!bfs::is_directory(vot_folder)) {
    printf("Error - %s is not a valid directory!\n", vot_folder.c_str());
    return;
  }

  // Find all video subcategories.
  vector<string> videos;
  find_subfolders(vot_folder, &videos);

  printf("Found %zu videos...\n", videos.size());
  for (size_t i = 0; i < videos.size(); ++i) {
    const string& video_name = videos[i];
    const string& video_path = vot_folder + "/" + video_name;

    printf("Loading video: %s\n", video_name.c_str());

    Video video;
    video.path = video_path;

    // Find all image files
    const boost::regex image_filter(".*\\.jpg");
    find_matching_files(video_path, image_filter, &video.all_frames);

    // Open the annotation file.
    const string& bbox_groundtruth_path = video_path + "/groundtruth.txt";
    FILE* bbox_groundtruth_file_ptr = fopen(bbox_groundtruth_path.c_str(), "r");
    int frame_num = 0;
    double Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;

    while (true) {
      // Read the annotation data.
      const int status = fscanf(bbox_groundtruth_file_ptr, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                   &Ax, &Ay, &Bx, &By, &Cx, &Cy, &Dx, &Dy);
      if (status == EOF) {
        break;
      }

      // Convert to bounding box format.
      Frame frame;
      frame.frame_num = frame_num;
      BoundingBox& bbox = frame.bbox;
      bbox.x1_ = std::min(Ax, std::min(Bx, std::min(Cx, Dx))) - 1;
      bbox.y1_ = std::min(Ay, std::min(By, std::min(Cy, Dy))) - 1;
      bbox.x2_ = std::max(Ax, std::max(Bx, std::max(Cx, Dx))) - 1;
      bbox.y2_ = std::max(Ay, std::max(By, std::max(Cy, Dy))) - 1;

      // Increment the frame number.
      frame_num++;

      video.annotations.push_back(frame);
    } // Process annotation file
    fclose(bbox_groundtruth_file_ptr);
    videos_.push_back(video);
  } // Process all videos
}
