#ifndef LOADER_VOT_H
#define LOADER_VOT_H

#include "video_loader.h"

// Loads videos from the VOT tracking dataset.
class LoaderVOT : public VideoLoader
{
public:
  // Loads all images and annotations.
  LoaderVOT(const std::string& vot_folder);
};

#endif // LOADER_VOT_H
