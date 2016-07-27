#include "loader_alov.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/helper.h"

using std::string;
using std::vector;
namespace bfs = boost::filesystem;

// If true, only load a small number of videos.
const bool kDoTest = false;

// What fraction of videos should be included in the validation set (the
// remainder are part of the training set).
// Note - after using the validation set to choose hyperparameters, set val_ratio to 0
// to train the final model on the training set + validation set (not the test set!)
const double val_ratio = 0.2;

LoaderAlov::LoaderAlov(const string& video_folder, const string& annotations_folder)
{
  if (!bfs::is_directory(annotations_folder)) {
    printf("Error - %s is not a valid directory!\n", annotations_folder.c_str());
    return;
  }

  // Find all video subcategories.
  vector<string> categories;
  find_subfolders(annotations_folder, &categories);

  const int max_categories = kDoTest ? 3 : categories.size();

  //printf("Found %zu categories...\n", categories.size());
  for (size_t i = 0; i < max_categories; ++i) {
    Category category;

    const string& category_name = categories[i];
    const string& category_path = annotations_folder + "/" + category_name;

    //printf("Loading category: %s\n", category_name.c_str());

    // Find the annotation files.
    const boost::regex annotation_filter(".*\\.ann");
    vector<string> annotation_files;
    find_matching_files(category_path, annotation_filter, &annotation_files);

    //printf("Found %zu annotations\n", annotation_files.size());

    // Iterate over all annotation files (one annotation file per video).
    for (size_t j = 0; j < annotation_files.size(); ++j) {
      const string& annotation_file = annotation_files[j];

      //printf("Processing annotation file: %s\n", annotation_file.c_str());

      // Read annotations for a given video.
      Video video;

      // Get the path to the video image files.
      const string video_path = video_folder + "/" + category_name + "/" +
          annotation_file.substr(0, annotation_file.length() - 4);
      video.path = video_path;
      //printf("Video path: %s\n", video_path.c_str());

      // Add all image files
      const boost::regex image_filter(".*\\.jpg");
      find_matching_files(video_path, image_filter, &video.all_frames);

      // Open the annotation file.
      const string& annotation_file_path = category_path + "/" + annotation_file;
      FILE* annotation_file_ptr = fopen(annotation_file_path.c_str(), "r");
      int frame_num;
      double Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;

      while (true) {
        // Read a line from the annotation file.
        const int status = fscanf(annotation_file_ptr, "%d %lf %lf %lf %lf %lf %lf %lf %lf\n",
                     &frame_num, &Ax, &Ay, &Bx, &By, &Cx, &Cy, &Dx, &Dy);
        if (status == EOF) {
          break;
        }

        // Convert the annotation data into frame and bounding box format.
        Frame frame;
        frame.frame_num = frame_num - 1; // Convert to 0-index
        BoundingBox& bbox = frame.bbox;
        bbox.x1_ = std::min(Ax, std::min(Bx, std::min(Cx, Dx))) - 1;
        bbox.y1_ = std::min(Ay, std::min(By, std::min(Cy, Dy))) - 1;
        bbox.x2_ = std::max(Ax, std::max(Bx, std::max(Cx, Dx))) - 1;
        bbox.y2_ = std::max(Ay, std::max(By, std::max(Cy, Dy))) - 1;

        // Save the annotation data.
        video.annotations.push_back(frame);
      } // Process annotation file

      // Save the video.
      fclose(annotation_file_ptr);
      videos_.push_back(video);
      category.videos.push_back(video);
    } // Process all annotation files in category

    // Save the video category.
    categories_.push_back(category);
  } // Process all categories
}

void LoaderAlov::get_videos(const bool get_train, std::vector<Video>* videos) const {
  for (size_t category_num = 0; category_num < categories_.size(); ++category_num) {
    const Category& category = categories_[category_num];

    // Number of videos in this category.
    size_t num_videos = category.videos.size();

    // Number of videos from this category to use in the validation set (the rest go into the training set).
    const int num_val = static_cast<int>(val_ratio * num_videos);

    // Number of videos from this category to use in the training set.
    const int num_train = num_videos - num_val;

    int start_num;
    int end_num;
    if (get_train) {
      // Get the start and end video num for training.
      start_num = 0;
      end_num = num_train - 1;
    } else {
      // Get the start and end video num for validation.
      start_num = num_train;
      end_num = num_videos - 1;
    }

    // Add the appropriate videos from this category to the list of videos
    // to return.
    const std::vector<Video>& category_videos = category.videos;
    for (size_t i = start_num; i <= end_num; ++i) {
      const Video& video = category_videos[i];
      videos->push_back(video);
    }
  }

  // Count the total number of labeled frames.
  size_t num_annotations = 0;
  for (size_t i = 0; i < videos->size(); ++i) {
    const Video& video = (*videos)[i];
    num_annotations += video.annotations.size();
  }
  printf("Total annotated video frames: %zu\n", num_annotations);
}

