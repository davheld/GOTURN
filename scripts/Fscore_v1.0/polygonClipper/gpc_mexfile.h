#ifndef __gpc_mexfile_h
#define __gpc_mexfile_h

#include <stdio.h>
#include "mex.h"
#include "gpc.h"


/* ===========================================================================
	                      Public Function Prototypes
	===========================================================================*/

void gpc_read_polygon_MATLAB (const mxArray	 *prhs,
							  gpc_polygon    *polygon);

void gpc_write_polygon_MATLAB(mxArray	     *plhs,
							  gpc_polygon    *polygon);


#endif