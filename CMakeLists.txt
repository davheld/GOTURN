cmake_minimum_required(VERSION 2.8)
project(GOTURN)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "No build type selected, default to ${CMAKE_BUILD_TYPE}")
endif()

find_package(Boost COMPONENTS system filesystem regex REQUIRED)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(TinyXML REQUIRED)


find_package( OpenCV REQUIRED )
message("Open CV version is ${OpenCV_VERSION}")
# Uncomment for no display (e.g. AWS instance)
# set(Opencv_DEFINITIONS -DNO_DISPLAY)
add_definitions(${Opencv_DEFINITIONS})
message("Opencv_DEFINITIONS is ${Opencv_DEFINITIONS}")

find_package(CUDA REQUIRED)
# Note: If can't find CUDA, please uncomment the below line and set the path manually
# set(CUDA_INCLUDE_DIRS /path_to_cuda/include)
include_directories(${CUDA_INCLUDE_DIRS})
message("CUDA_INCLUDE_DIRS is ${CUDA_INCLUDE_DIRS}")


find_package(Caffe REQUIRED)
# If Caffe not found, configure line 5 of cmake/Modules/FindCaffe.cmake
# If that fails uncomment the two lines below and set paths manually
# set(Caffe_DIR /path_to_caffe/build/install)
# set(Caffe_INCLUDE_DIRS /path_to_caffe/build/install/include)
include_directories(${Caffe_INCLUDE_DIRS})
# Uncomment for CPU only:
# set(Caffe_DEFINITIONS -DCPU_ONLY)
add_definitions(${Caffe_DEFINITIONS})
message("Caffe_DEFINITIONS is ${Caffe_DEFINITIONS}")
message("Caffe_DIR is ${Caffe_DIR}")
message("Caffe_INCLUDE_DIRS is ${Caffe_INCLUDE_DIRS}")


set(GLOG_LIB glog)

add_library (${PROJECT_NAME}
src/helper/bounding_box.cpp
src/train/example_generator.cpp
src/helper/helper.cpp
src/helper/high_res_timer.cpp
src/helper/image_proc.cpp
src/loader/loader_alov.cpp
src/loader/loader_imagenet_det.cpp
src/loader/loader_vot.cpp
src/network/regressor.cpp
src/network/regressor_base.cpp
src/network/regressor_train.cpp
src/network/regressor_train_base.cpp
src/tracker/tracker.cpp
src/tracker/tracker_manager.cpp
src/train/tracker_trainer.cpp
src/loader/video.cpp
src/loader/video_loader.cpp
src/native/vot.cpp

src/helper/bounding_box.h
src/train/example_generator.h
src/helper/helper.h
src/helper/high_res_timer.h
src/helper/image_proc.h
src/loader/loader_alov.h
src/loader/loader_imagenet_det.h
src/loader/loader_vot.h
src/network/regressor.h
src/network/regressor_base.h
src/network/regressor_train.h
src/network/regressor_train_base.h
src/tracker/tracker.h
src/tracker/tracker_manager.h
src/train/tracker_trainer.h
src/loader/video.h
src/loader/video_loader.h
src/native/vot.h
)

# Add src to include directories.
include_directories(src)
include_directories(src/native)

# Add all files.
#file(GLOB_RECURSE hdrs src/*.h*)
#file(GLOB_RECURSE srcs src/*.cpp)
#add_library (${PROJECT_NAME} ${srcs} ${hdrs})

add_executable (test_tracker_vot src/test/test_tracker_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${GLOG_LIB})
target_link_libraries (test_tracker_vot ${PROJECT_NAME})
# Note: If can't find trax, please download trax and build it, then uncomment the below line and set the path manually
# target_link_libraries(${PROJECT_NAME} /path_to_trax/build/libtrax.so)

add_executable (test_tracker_alov src/test/test_tracker_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${GLOG_LIB})
target_link_libraries (test_tracker_alov ${PROJECT_NAME})

add_executable (save_videos_vot src/test/save_videos_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${GLOG_LIB})
target_link_libraries (save_videos_vot ${PROJECT_NAME})

add_executable (train src/train/train.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${Caffe_LIBRARIES} ${TinyXML_LIBRARIES} ${GLOG_LIB})
target_link_libraries (train ${PROJECT_NAME})

add_executable (show_tracker_vot src/visualizer/show_tracker_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${Boost_LIBRARIES} ${GLOG_LIB})
target_link_libraries (show_tracker_vot ${PROJECT_NAME})

add_executable (show_tracker_alov src/visualizer/show_tracker_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${Boost_LIBRARIES} ${GLOG_LIB})
target_link_libraries (show_tracker_alov ${PROJECT_NAME})

add_executable (show_imagenet src/visualizer/show_imagenet.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TinyXML_LIBRARIES})
target_link_libraries (show_imagenet ${PROJECT_NAME})

add_executable (show_alov src/visualizer/show_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries (show_alov ${PROJECT_NAME})


