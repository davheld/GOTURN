/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
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

#include <stdio.h>
#include <ctype.h>

// Uncomment line below if you want to use rectangles
// #define VOT_RECTANGLE
#include "vot.h"

int main( int argc, char** argv)
{
    int f;

    // *****************************************
    // VOT: Call vot_initialize at the beginning
    // *****************************************
    vot_region* selection = vot_initialize();

    // Process the first frame
    const char* imagefile = vot_frame();
    if (!imagefile) {
        vot_quit();
        exit(0);
    }

    for(f = 1;; f++)
    {

        // *****************************************
        // VOT: Call vot_frame to get path of the 
        //      current image frame. If the result is
        //      null, the sequence is over.
        // *****************************************
        const char* imagefile = vot_frame();
        if (!imagefile) break;

        // *****************************************
        // VOT: Report the position of the object 
        //      every frame using vot_report function.
        // *****************************************
        vot_report(selection);

        usleep(10000);

    }

    vot_region_release(&selection);

    // *************************************
    // VOT: Call vot_deinitialize at the end
    // *************************************
    vot_quit();

    return 0;
}

