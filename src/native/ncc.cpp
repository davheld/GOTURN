/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * This is an example of a stationary tracker. It only reports the initial 
 * position for all frames and is used for testing purposes.
 * The main function of this example is to show the developers how to modify
 * their trackers to work with the evaluation environment.
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
 *
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>

#define VOT_RECTANGLE
#include "vot.h"
 
class NCCTracker
{
public:
    inline void init(cv::Mat & img, cv::Rect rect)
    {
        p_window = MAX(rect.width, rect.height) * 2;

        cv::Mat gray;
        cv::cvtColor(img, gray, CV_BGR2GRAY);

        int left = MAX(rect.x, 0);
        int top = MAX(rect.y, 0);

        int right = MIN(rect.x + rect.width, gray.cols - 1);
        int bottom = MIN(rect.y + rect.height, gray.rows - 1);

        cv::Rect roi(left, top, right - left, bottom - top);

        gray(roi).copyTo(p_template);

        p_position.x = (float)rect.x + (float)rect.width / 2;
        p_position.y = (float)rect.y + (float)rect.height / 2;

        p_size = cv::Size2f(rect.width, rect.height);

    }
    inline cv::Rect track(cv::Mat img)
    {

        cv::Mat gray;
        cv::cvtColor(img, gray, CV_BGR2GRAY);

        float left = MAX(round(p_position.x - (float)p_window / 2), 0);
        float top = MAX(round(p_position.y - (float)p_window / 2), 0);

        float right = MIN(round(p_position.x + (float)p_window / 2), gray.cols - 1);
        float bottom = MIN(round(p_position.y + (float)p_window / 2), gray.rows - 1);

        cv::Rect roi((int) left, (int) top, (int) (right - left), (int) (bottom - top));

        if (roi.width < p_template.cols || roi.height < p_template.rows) {
            cv::Rect result;

            result.x = p_position.x - p_size.width / 2;
            result.y = p_position.y - p_size.height / 2;
            result.width = p_size.width;
            result.height = p_size.height;
            return result;

        }

        cv::Mat matches;
        cv::Mat cut = gray(roi);

        cv::matchTemplate(cut, p_template, matches, CV_TM_CCOEFF_NORMED);

        cv::Point matchLoc;
        cv::minMaxLoc(matches, NULL, NULL, NULL, &matchLoc, cv::Mat());

        cv::Rect result;

        p_position.x = left + matchLoc.x + (float)p_size.width / 2;
        p_position.y = top + matchLoc.y + (float)p_size.height / 2;

        result.x = left + matchLoc.x;
        result.y = top + matchLoc.y;
        result.width = p_size.width;
        result.height = p_size.height;

        return result;
    }

private:
    cv::Point2f p_position;

    cv::Size p_size;

    float p_window;

    cv::Mat p_template;
};

int main( int argc, char** argv) {

    NCCTracker tracker;
    VOT vot;

    cv::Rect initialization;
    initialization << vot.region();
    cv::Mat image = cv::imread(vot.frame());
    tracker.init(image, initialization);

    while (!vot.end()) {

        string imagepath = vot.frame();

        if (imagepath.empty()) break;

        cv::Mat image = cv::imread(imagepath);

        cv::Rect rect = tracker.track(image);

        vot.report(tracker.track(image));

    }

}

