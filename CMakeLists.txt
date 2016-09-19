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

if(CUDA_FOUND)
include_directories(${CUDA_INCLUDE_DIRS})
endif()

find_package(Caffe REQUIRED)
include_directories(${Caffe_INCLUDE_DIRS})
add_definitions(${Caffe_DEFINITIONS})    # ex. -DCPU_ONLY
message("Caffe_DIR is ${Caffe_DIR}")

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

# Add all files.
#file(GLOB_RECURSE hdrs src/*.h*)
#file(GLOB_RECURSE srcs src/*.cpp)
#add_library (${PROJECT_NAME} ${srcs} ${hdrs})

add_executable (test_tracker_vot src/test/test_tracker_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES})
target_link_libraries (test_tracker_vot ${PROJECT_NAME})

add_executable (test_tracker_alov src/test/test_tracker_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES})
target_link_libraries (test_tracker_alov ${PROJECT_NAME})

add_executable (save_videos_vot src/test/save_videos_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES})
target_link_libraries (save_videos_vot ${PROJECT_NAME})

add_executable (train src/train/train.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${Caffe_LIBRARIES} ${TinyXML_LIBRARIES})
target_link_libraries (train ${PROJECT_NAME})

add_executable (show_tracker_vot src/visualizer/show_tracker_vot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries (show_tracker_vot ${PROJECT_NAME})

add_executable (show_tracker_alov src/visualizer/show_tracker_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries (show_tracker_alov ${PROJECT_NAME})

add_executable (show_imagenet src/visualizer/show_imagenet.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TinyXML_LIBRARIES})
target_link_libraries (show_imagenet ${PROJECT_NAME})

add_executable (show_alov src/visualizer/show_alov.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries (show_alov ${PROJECT_NAME})


