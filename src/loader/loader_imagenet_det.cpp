#include <tinyxml.h>

#include "train/example_generator.h"
#include "loader/loader_imagenet_det.h"
#include "helper/helper.h"

using std::vector;
using std::string;

namespace bfs = boost::filesystem;

// If true, only load a small number of images.
const bool kDoTest = false;

// Max ratio of bbox size to image size that we load.
// If the ratio is too large (i.e. the object occupies almost the entire image),
// then we will not be able to simulate object motion.
const double kMaxRatio = 0.66;

LoaderImagenetDet::LoaderImagenetDet(const std::string& image_folder,
                                     const std::string& annotations_folder)
  : path_(image_folder)
{
  if (!bfs::is_directory(annotations_folder)) {
    printf("Error - %s is not a valid directory!\n", annotations_folder.c_str());
    return;
  }

  // Find all image subfolders.
  vector<string> subfolders;
  find_subfolders(annotations_folder, &subfolders);

  size_t num_annotations = 0;

  const int max_subfolders = kDoTest ? 1 : subfolders.size();

  printf("Found %zu subfolders...\n", subfolders.size());
  printf("Loading images, please wait...\n");

  // Iterate over all subfolders.
  for (size_t i = 0; i < max_subfolders; ++i) {
    // Every 100 iterations, print an update.
    if (i % 10 == 0 && i > 0) {
      printf("Loaded %zu subfolders\n", i);
    }
    const string& subfolder_name = subfolders[i];
    const string& subfolder_path = annotations_folder + "/" + subfolder_name;

    //printf("Loading subfolder: %s\n", subfolder_name.c_str());

    // Find the annotation files.
    const boost::regex annotation_filter(".*\\.xml");
    vector<string> annotation_files;
    find_matching_files(subfolder_path, annotation_filter, &annotation_files);

    //printf("Found %zu annotations\n", annotation_files.size());

    // Iterate over all annotation files.
    for (size_t j = 0; j < annotation_files.size(); ++j) {
      const string& annotation_file = annotation_files[j];

      const string& full_path = subfolder_path + "/" + annotation_file;

      // Read the annotations.
      //printf("Processing annotation file: %s\n", full_path.c_str());
      vector<Annotation> annotations;
      LoadAnnotationFile(full_path, &annotations);

      if (annotations.size() == 0) {
        continue;
      }

      // Count the number of annotations.
      //printf("Found %zu annotations\n", annotations.size());
      num_annotations += annotations.size();

      // Save the annotations.
      images_.push_back(annotations);
    } // Process all annotations in a subfolder.
  } // Process all subfolders.
  printf("Found %zu annotations from %zu images\n", num_annotations, images_.size());
}

void LoaderImagenetDet::LoadAnnotationFile(const string& annotation_file,
                                           vector<Annotation>* image_annotations) {
  // Open the annotation file.
  TiXmlDocument document(annotation_file.c_str());
  document.LoadFile();

  // Read the top-level element.
  TiXmlHandle docHandle( &document );
  TiXmlElement* annotations = docHandle.FirstChild().ToElement();
  if (!annotations) {
    printf("No annotations!\n");
    return;
  }

  // Get the folder and filename for the image corresponding to this annotation.
  const string& folder = annotations->FirstChildElement("folder")->GetText();
  //printf("Folder: %s\n", folder);

  const string& filename = annotations->FirstChildElement("filename")->GetText();
  //printf("File: %s\n", filename);

  // Get the relative image size that was displayed to the annotater (may have been downsampled).
  TiXmlNode* size = annotations->FirstChild("size");
  if (!size) {
    printf("Error - no size!\n");
    return;
  }
  const int display_width = atoi(size->FirstChildElement("width")->GetText());
  const int display_height = atoi(size->FirstChildElement("height")->GetText());
  //printf("Size: %d %d\n", display_width, display_height);

  // Get all of the bounding boxes in this image.
  for(TiXmlNode* object = annotations->FirstChild("object"); object; object = object->NextSibling("object")) {
    // Get the boudning box coordinates.
    TiXmlElement* bbox = object->FirstChildElement("bndbox");
    const int xmin = atoi(bbox->FirstChildElement("xmin")->GetText());
    const int xmax = atoi(bbox->FirstChildElement("xmax")->GetText());
    const int ymin = atoi(bbox->FirstChildElement("ymin")->GetText());
    const int ymax = atoi(bbox->FirstChildElement("ymax")->GetText());

    const double width = xmax - xmin;
    const double height = ymax - ymin;

    // If this object occupies almost the entire image, then ignore it,
    // since we will not be able to simulate object motion.
    if (width > kMaxRatio * display_width || height > kMaxRatio * display_height) {
      continue;
    }

    // Convert the annotation to bounding box format.
    Annotation annotation;
    annotation.image_path = folder + "/" + filename;
    annotation.bbox.x1_ = xmin;
    annotation.bbox.x2_ = xmax;
    annotation.bbox.y1_ = ymin;
    annotation.bbox.y2_ = ymax;
    annotation.display_width_ = display_width;
    annotation.display_height_ = display_height;

    // Check if the annotation is outside of the border of the image or otherwise invalid.
    if (xmin < 0 || ymin < 0 || xmax <= xmin || ymax <= ymin) {
      printf("Skipping invalid annotation from file: %s\n", annotation_file.c_str());
      printf("Annotation: %d, %d, %d, %d\n", xmin, xmax, ymin, ymax);
      printf("Image path: %s\n", annotation.image_path.c_str());
      printf("Display: %d, %d\n", display_width, display_height);
      continue;
    }

    // Save the annotation.
    image_annotations->push_back(annotation);

    //printf("Path: %s\n", annotation.image_path.c_str());
    //printf("bbox: %d %d %d %d\n", xmin, xmax, ymin, ymax);
  }
}

void LoaderImagenetDet::ShowImages() const {
  // Iterate over all images.
  for (size_t image_index = 0; image_index < images_.size(); ++image_index) {
    // Load the image.
    cv::Mat image;
    LoadImage(image_index, &image);

#ifndef NO_DISPLAY
    // Display the image and wait for a keystroke to continue.
    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow( "Display window", image );                   // Show our image inside it.
    cv::waitKey(0);                                          // Wait for a keystroke in the window
#endif
  }
}

void LoaderImagenetDet::ComputeStatistics() const {
  bool first_time = true;

  // Variables to store various image statistics.
  double min_width;
  double min_height;
  double max_width;

  double max_height;
  double mean_width;
  double mean_height;

  double min_width_frac;
  double min_height_frac;
  double max_width_frac;

  double max_height_frac;
  double mean_width_frac;
  double mean_height_frac;

  int n = 0;

  // Iterate over all images.
  for (size_t i = 0; i < images_.size(); ++i) {

    // Iterate over all annotations.
    const std::vector<Annotation>& annotations = images_[i];
    for (size_t j = 0; j < annotations.size(); ++j) {

      // Load the annotation information.
      const Annotation& annotation = annotations[j];
      const double width = annotation.bbox.get_width();
      const double height = annotation.bbox.get_height();
      const double image_width = annotation.display_width_;
      const double image_height = annotation.display_height_;

      // Compute the fraction of the image that this bounding box occupies.
      const double width_frac = width / image_width;
      const double height_frac = height / image_height;

      // If this is the first annotation, then save the information for it.
      if (first_time) {
        min_width = width;
        min_height = height;
        max_width = width;
        max_height = height;

        min_width_frac = width_frac;
        min_height_frac = height_frac;
        max_width_frac = width_frac;
        max_height_frac = height_frac;

        first_time = false;
      } else {
        // Update our estimates of the smallest / largest for various measures:
        // bounding box width / height,
        // fraction of the image width / height occupied by the bounding box.
        min_width = std::min(min_width, width);
        min_height = std::min(min_height, height);
        max_width = std::max(max_width, width);
        max_height = std::max(max_height, height);

        min_width_frac = std::min(min_width_frac, width_frac);
        min_height_frac = std::min(min_height_frac, height_frac);
        max_width_frac = std::max(max_width_frac, width_frac);
        max_height_frac = std::max(max_height_frac, height_frac);
      }
      // Update our mean estimates.
      mean_width = (static_cast<double>(n) * mean_width + width) / static_cast<double>(n + 1);
      mean_height = (static_cast<double>(n) * mean_height + height) / static_cast<double>(n + 1);
      mean_width_frac = (static_cast<double>(n) * mean_width_frac + width_frac) / static_cast<double>(n + 1);
      mean_height_frac = (static_cast<double>(n) * mean_height_frac + height_frac) / static_cast<double>(n + 1);
    }
    n++;
  }
  // Print the image statistics for this dataset.
  printf("Width: %lf %lf %lf\n", min_width, max_width, mean_width);
  printf("Height: %lf %lf %lf\n", min_height, max_height, mean_height);
  printf("Width frac: %lf %lf %lf\n", min_width_frac, max_width_frac, mean_width_frac);
  printf("Height frac: %lf %lf %lf\n", min_height_frac, max_height_frac, mean_height_frac);
  printf("Total: %d\n", n);
}

void LoaderImagenetDet::ShowAnnotations() const {
  // Iterate over all images.
  for (size_t i = 0; i < images_.size(); ++i) {

    // Iterate over all annotations.
    const std::vector<Annotation>& annotations = images_[i];
    for (size_t j = 0; j < annotations.size(); ++j) {

      // Load the image and annotation.
      cv::Mat image;
      BoundingBox bbox;
      LoadAnnotation(i, j, &image, &bbox);
      printf("Width: %lf, height: %lf\n", bbox.get_width(), bbox.get_height());

      // Draw the annotation on the image.
      bbox.DrawBoundingBox(&image);

#ifndef NO_DISPLAY
      // Display the image with the annotation.
      cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
      cv::imshow( "Display window", image );                   // Show our image inside it.
      cv::waitKey(0);                                          // Wait for a keystroke in the window
#endif
    }
  }
}

void LoaderImagenetDet::LoadImage(const size_t image_num,
                                  cv::Mat* image) const {
  // Load the specified image's annotations.
  const std::vector<Annotation>& annotations = images_[image_num];

  // Load the first annotation for this image.
  const int annotation_num = 0;
  const Annotation& annotation = annotations[annotation_num];

  // Load the specified image (using the file-path contained within the annotation).
  const string& image_file = path_ + "/" + annotation.image_path + ".JPEG";
  *image = cv::imread(image_file.c_str());

  // Check that we were able to load the image.
  if (!image->data) {
    printf("Could not open or find image %s\n", image_file.c_str());
    return;
  }
}

void LoaderImagenetDet::LoadAnnotation(const size_t image_num,
                                       const size_t annotation_num,
                                       cv::Mat* image,
                                       BoundingBox* bbox) const {
  // Load the specified image's annotations.
  const std::vector<Annotation>& annotations = images_[image_num];
  const Annotation& annotation = annotations[annotation_num];

  // Load the specified image.
  const string& image_file = path_ + "/" + annotation.image_path + ".JPEG";
  *image = cv::imread(image_file.c_str());

  // Check that we were able to load the image.
  if (!image->data) {
    printf("Could not open or find image %s\n", image_file.c_str());
    return;
  }

  // Check if the dispay width / height differs from the image width / height (the image may have been
  // downsampled for visualization).  Usually this value will be 1.
  double factor = 1;
  if (image->rows != annotation.display_height_ || image->cols != annotation.display_width_) {
    printf("Image: %zu %zu %s\n", image_num, annotation_num, image_file.c_str());
    printf("Image size: %d %d\n", image->rows, image->cols);
    printf("Display size: %d %d\n", annotation.display_height_,
           annotation.display_width_);

    // Check that the aspect ratio was preserved for annotation.
    factor = static_cast<double>(image->rows) / static_cast<double>(annotation.display_height_);
    const double factor2 = static_cast<double>(image->cols) / static_cast<double>(annotation.display_width_);
    printf("Factor: %lf %lf\n", factor, factor2);
  }

  // Scale the bounding box by the ratio of the the image size to the display size.
  *bbox = annotation.bbox;
  bbox->x1_ *= factor;
  bbox->x2_ *= factor;
  bbox->y1_ *= factor;
  bbox->y2_ *= factor;
}

void LoaderImagenetDet::ShowAnnotationsRand() const {
  while (true) {
    // Choose a random image.
    const int image_num = rand() % images_.size();

    // Get the annotations for this image.
    const std::vector<Annotation>& annotations = images_[image_num];

    // Choose a random annotation.
    const int annotation_num = rand() % annotations.size();

    // Load the image and annotation.
    cv::Mat image;
    BoundingBox bbox;
    LoadAnnotation(image_num, annotation_num, &image, &bbox);

    // Show the full image with the bounding box.
    cv::Mat image_copy;
    image.copyTo(image_copy);
    bbox.DrawBoundingBox(&image_copy);
#ifndef NO_DISPLAY
    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow( "Display window", image_copy );                   // Show our image inside it.
    cv::waitKey(0);                                          // Wait for a keystroke in the window
#endif
  }
}

void LoaderImagenetDet::ShowAnnotationsShift() const {
  // Create an example generator with parameters for a large shift, so that this will be noticable.
  // This will be used to artificially shift the crops around the annotations, creating an
  // apparent motion (via translation and scale change).
  ExampleGenerator example_generator(5, 5, -0.4, 0.4);

  const bool save_images = false;

  // Iterate over all images.
  for (size_t i = 0; i < images_.size(); ++i) {

    // Iterate over all images.
    const std::vector<Annotation>& annotations = images_[i];
    for (size_t j = 0; j < annotations.size(); ++j) {
      // Load the image and its annotation.
      cv::Mat image;
      BoundingBox bbox;
      LoadAnnotation(i, j, &image, &bbox);

      // Show the full image with the bounding box annotation.
      cv::Mat image_copy;
      image.copyTo(image_copy);
      bbox.Draw(0, 255, 0, &image_copy);
#ifndef NO_DISPLAY
      cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
      cv::imshow( "Display window", image_copy );                   // Show our image inside it.
#endif

      // If desired, save this image with its annotation.
      if (save_images) {
        const string image_name = "Image" + num2str(i) + "_" + num2str(j) + "full.jpg";
        cv::imwrite(image_name, image_copy);
      }

      // Set up the example generator with this annotation.
      example_generator.Reset(bbox, bbox, image, image);

      // Save the image and annotation index so we can give this image a simple identifier.
      example_generator.set_indices(i, j);

      // Make a training example from randomly shifting the annotation and visualize it.
      cv::Mat image_rand_focus;
      cv::Mat target_pad;
      BoundingBox bbox_gt_scaled;
      const bool visualize = true;
      const int kNumShifts = 1;
      for (int k = 0; k < kNumShifts; ++k) {
        example_generator.MakeTrainingExampleBBShift(visualize, &image_rand_focus,
                                                     &target_pad, &bbox_gt_scaled);
      }
    }
  }
}

