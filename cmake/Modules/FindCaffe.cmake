# Caffe package
unset(Caffe_FOUND)

set(Caffe_DIR /home/kunal/ssd/caffe/distribute)


find_path(Caffe_INCLUDE_DIRS NAMES caffe/caffe.hpp caffe/common.hpp caffe/net.hpp caffe/proto/caffe.pb.h caffe/util/io.hpp caffe/vision_layers.hpp
  HINTS
  ${Caffe_DIR}/include)



find_library(Caffe_LIBRARIES NAMES caffe
  HINTS
  ${Caffe_DIR}/lib)

message("lib_dirs:${Caffe_LIBRARIES}")

if(Caffe_LIBRARIES AND Caffe_INCLUDE_DIRS)
    set(Caffe_FOUND 1)
endif()
