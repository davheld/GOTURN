/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * This header file contains C functions that can be used to quickly integrate
 * VOT challenge support into your C or C++ tracker.
 *
 * Copyright (c) 2015, VOT Committee
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 

 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

#ifndef _VOT_TOOLKIT_H
#define _VOT_TOOLKIT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VOT_RECTANGLE

#define VOT_READ_BUFFER 2024

// Newer compilers support interactive checks for headers, otherwise we have to enable TraX support manually
#ifdef __has_include
#  if __has_include("trax.h")
#    include <trax.h>
#    define VOT_TRAX
#  endif
#else
#  ifdef TRAX
#    include <trax.h>
#    define VOT_TRAX
#  endif
#endif

// Define VOT_OPENCV after including OpenCV core header to enable better OpenCV support
#ifdef __OPENCV_CORE_HPP__
#  define VOT_OPENCV
#endif

#ifndef VOT_RECTANGLE
#define VOT_POLYGON
#endif

#ifdef VOT_POLYGON

typedef struct vot_region {
    float* x;
    float* y;
    int count;
} vot_region;

#else

typedef struct vot_region {
    float x;
    float y;
    float width;
    float height;
} vot_region;

#endif // VOT_POLYGON

vot_region* _parse_region(char* buffer);
void vot_region_release(vot_region** region);
vot_region* vot_region_copy(const vot_region* region);

#ifdef VOT_POLYGON
vot_region* vot_region_create(int n);
#else
vot_region* vot_region_create();
#endif

#ifdef __cplusplus

#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class VOT;

class VOTRegion {
    friend class VOT;
public:

    ~VOTRegion() {
        vot_region_release(&_region);
    }

    VOTRegion(const vot_region* region) {
        _region = vot_region_copy(region);
    }

#ifdef VOT_POLYGON
    VOTRegion(int count) {
        _region = vot_region_create(count);
    }

    void set(int i, float x, float y) { assert(i >= 0 && i < _region->count); _region->x[i] = x; _region->y[i] = y; }
    float get_x(int i) const { assert(i >= 0 && i < _region->count); return _region->x[i]; } 
    float get_y(int i) const { assert(i >= 0 && i < _region->count); return _region->y[i]; }
    int count() const { return _region->count; }

#else

    VOTRegion() {
        _region = vot_region_create();
    }

    float get_x() const { return _region->x; }
    float get_y() const { return _region->y; }
    float get_width() const { return _region->width; }
    float get_height() const { return _region->height; }

    float set_x(float x) { _region->x = x; }
    float set_y(float y) { _region->y = y; }
    float set_width(float width) { _region->width = width; }
    float set_height(float height) { _region->height = height; }

#endif // VOT_POLYGON

    VOTRegion& operator= (const VOTRegion &source) {

        if (this == &source)
            return *this;

#ifdef VOT_POLYGON

        if (this->_region->count != source.count()) {
            vot_region_release(&(this->_region));
            this->_region = vot_region_create(source.count());
        }

        for (int i = 0; i < source.count(); i++) {
            set(i, source.get_x(i), source.get_y(i));
        }

#else

        set_x(source.get_x());
        set_y(source.get_y());
        set_width(source.get_width());
        set_height(source.get_height());

#endif

        return *this;
    }

#ifdef VOT_OPENCV

    VOTRegion(const cv::Rect& rectangle) {
#ifdef VOT_POLYGON
        _region = vot_region_create(4);
#else
        _region = vot_region_create();
#endif // VOT_POLYGON
        set(rectangle);
    }

    void set(const cv::Rect& rectangle) {

#ifdef VOT_POLYGON

        if (_region->count != 4) {
            vot_region_release(&(this->_region));
            _region = vot_region_create(4);
        }

	    set(0, rectangle.x, rectangle.y);
	    set(1, rectangle.x + rectangle.width, rectangle.y);
	    set(2, rectangle.x + rectangle.width, rectangle.y + rectangle.height);
	    set(3, rectangle.x, rectangle.y + rectangle.height);

#else

        set_x(rectangle.x);
        set_y(rectangle.y);
        set_width(rectangle.width);
        set_height(rectangle.height);

#endif // VOT_POLYGON

    }

    void get(cv::Rect& rectangle) const {

#ifdef VOT_POLYGON

	    float top = FLT_MAX;
	    float bottom = FLT_MIN;
	    float left = FLT_MAX;
	    float right = FLT_MIN;

	    for (int j = 0; j < _region->count; j++) {
		    top = MIN(top, _region->y[j]);
		    bottom = MAX(bottom, _region->y[j]);
		    left = MIN(left, _region->x[j]);
		    right = MAX(right, _region->x[j]);
	    }

        rectangle.x = left;
        rectangle.y = top;
        rectangle.width = right - left;
        rectangle.height = bottom - top;
#else

        rectangle.x = get_x();
        rectangle.y = get_y();
        rectangle.width = get_width();
        rectangle.height = get_height();

#endif // VOT_POLYGON

    }

    void operator= (cv::Rect& rectangle) {
        this->get(rectangle);
    }

#endif // VOT_OPENCV

protected:

    vot_region* _region;

};

#ifdef VOT_OPENCV

void operator<< (VOTRegion &source, const cv::Rect& rectangle) {
    source.set(rectangle);
}

void operator>> (const VOTRegion &source, cv::Rect& rectangle) {
    source.get(rectangle);
}

void operator<< (cv::Rect& rectangle, const VOTRegion &source) {
    source.get(rectangle);
}

void operator>> (const cv::Rect& rectangle, VOTRegion &source) {
    source.set(rectangle);
}


#endif // VOT_OPENCV

class VOT {
public:
    VOT() {
        _region = vot_initialize(); 
    }

    ~VOT() {
        vot_quit();
    }

    const VOTRegion region() { 
        return VOTRegion(_region);
    }

    void report(const VOTRegion& region) {

        vot_report(region._region);

    }

    const string frame() {

        const char* result = vot_frame();

        if (!result)
            return string();

        return string(result);
    }

    bool end() {
        return vot_end() != 0;
    }


private:

    vot_region* vot_initialize();

    void vot_quit();

    const char* vot_frame();

    void vot_report(vot_region* region);

    int vot_end();

    vot_region* _region;

#endif // __cplusplus

    // Current position in the sequence
    int _vot_sequence_position;
    // Size of the sequence
    int _vot_sequence_size;
    // List of image file names
    //char** _vot_sequence = NULL;
    char** _vot_sequence; //DH
    // List of results
    //vot_region** _vot_result = NULL;
    vot_region** _vot_result; //DH

#ifdef VOT_TRAX

    trax_handle* _trax_handle = NULL;
    char _trax_image_buffer[VOT_READ_BUFFER];

#ifdef VOT_POLYGON

vot_region* _trax_to_region(const trax_region* _trax_region) {
    int i;
    int count = trax_region_get_polygon_count(_trax_region);
    vot_region* region = vot_region_create(count);
    for (i = 0; i < count; i++)
        trax_region_get_polygon_point(_trax_region, i, &(region->x[i]), &(region->y[i]));
    return region;
}
trax_region* _region_to_trax(const vot_region* region) {
    int i;
    trax_region* _trax_region = trax_region_create_polygon(region->count);
    assert(trax_region_get_type(_trax_region) == TRAX_REGION_POLYGON);
    for (i = 0; i < region->count; i++)
        trax_region_set_polygon_point(_trax_region, i, region->x[i], region->y[i]);
    return _trax_region;
}
#else

vot_region* _trax_to_region(const trax_region* _trax_region) {
    vot_region* region = vot_region_create();
    assert(trax_region_get_type(_trax_region) == TRAX_REGION_RECTANGLE);
    trax_region_get_rectangle(_trax_region, &(region->x), &(region->y), &(region->width), &(region->height));
    return region;
}
trax_region* _region_to_trax(const vot_region* region) {
    return trax_region_create_rectangle(region->x, region->y, region->width, region->height);
}

#endif // VOT_POLYGON
#endif // VOT_TRAX

#ifdef __cplusplus

};

#endif // __cplusplus

#ifdef __cplusplus
#  define VOT_PREFIX(FUN) VOT::FUN
#else
#  define VOT_PREFIX(FUN) FUN
#endif

#endif // _VOT_TOOLKIT_H
