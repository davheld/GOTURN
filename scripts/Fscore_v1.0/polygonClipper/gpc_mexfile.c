/* ===========================================================================
	                               Includes
	===========================================================================*/

#include "mex.h"
#include "gpc_mexfile.h"
#include "gpc.h"
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

/* ===============================
	    Constants
	===============================*/

#ifndef TRUE
#define FALSE              0
#define TRUE               1
#endif

/*	=================================
		GATEWAY ROUTINE TO MATLAB
	=================================*/

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
	gpc_polygon subject, clip, result;
    int dims[2], GPC_ARG;
	const char *field_names[] = {"x","y","hole"};
    
	if (nrhs == 0) {
        mexPrintf("\nOutPol = PolygonClip(RefPol, ClipPol, [type]);\n\n");
        mexPrintf("All polygons are structures with the fields ...\n");
        mexPrintf("  .x:    x-coordinates of contour\n");
        mexPrintf("  .y:    y-coordinates of contour\n");
        mexPrintf("  .hole: hole flag\n");
        mexPrintf("\nEvery polygon may contain several contours (s.example). Optional type parameter selects method:\n");
        mexPrintf("0 - Diff\n");
        mexPrintf("1 - And (Standard)\n");
        mexPrintf("2 - Xor\n");
        mexPrintf("3 - Union\n");
        mexPrintf("\nPolygon Clipping Routine based on gpc2.32. Credit goes to ...\n");
        mexPrintf("Alan Murta, Advanced Interfaces Group, Department of Computer Science, University of Manchester\n");
        mexPrintf("http://www.cs.man.ac.uk/~toby/alan/software//\n\n");
        return;}
      
    /*  Check number of arguments */
	if (nrhs < 2 || nrhs > 3)
        mexErrMsgTxt("Two or three inputs required.");
	if (nlhs != 1)
        mexErrMsgTxt("One output required.");

	/* Import polygons to structures */
	gpc_read_polygon_MATLAB(prhs[0], &subject);
    gpc_read_polygon_MATLAB(prhs[1], &clip);  

	/* Calling computational routine */
    if (nrhs==2 || !mxIsDouble(prhs[2]) || mxGetM(prhs[2])!=1 || mxGetN(prhs[2])!=1)
        GPC_ARG = GPC_INT;
    else
        GPC_ARG = mxGetScalar(prhs[2]);
    if (GPC_ARG!=GPC_DIFF && GPC_ARG!=GPC_INT && GPC_ARG!=GPC_XOR && GPC_ARG!=GPC_UNION)
        GPC_ARG = GPC_INT;
    
	gpc_polygon_clip(GPC_ARG, &subject, &clip, &result);
    
	/* Output Data to Matlab */
    dims[0] = 1;
    dims[1] = result.num_contours;
    plhs[0] = mxCreateStructArray(2, dims, 3, field_names);
	gpc_write_polygon_MATLAB(plhs[0], &result);

}

/* =================================
	END GATEWAY 
	=================================*/


/*	=================================
	Routines for data-IO (Matlab)
	=================================*/

int Min(const int *Numbers, const int Count)
{	int Minimum = Numbers[0];
	int i;
	for(i = 0; i < Count; i++)
		if( Minimum > Numbers[i] )
			Minimum = Numbers[i];
	return Minimum;}

int Max(const int *Numbers, const int Count)
{	int Maximum = Numbers[0];
	int i;
	for(i = 0; i < Count; i++)
		if( Maximum < Numbers[i] )
			Maximum = Numbers[i];
	return Maximum;}

void gpc_read_polygon_MATLAB(const mxArray *prhs, gpc_polygon *p)
{

	int c, v;
	int read_hole_flag;
	int id_x, id_y, id_hole;
	int nx[2], ny[2];
	double *x, *y;
	const mxArray *field_x, *field_y, *field_hole;

/*  Checking if input is non empty Matlab-structure */
	if (!mxIsStruct(prhs))
        mexErrMsgTxt("Input needs to be structure.");
	if (!mxGetM(prhs) || !mxGetN(prhs))
        mexErrMsgTxt("Empty structure.");

/*  Checking field names and data type  */
    id_x = mxGetFieldNumber(prhs,"x");
    if (id_x==-1)
        mexErrMsgTxt("Input structure must contain a field 'x'.");
    
	field_x = mxGetFieldByNumber(prhs, 0, id_x);
    if (!mxIsDouble(field_x))
        mexErrMsgTxt("Structure field 'x' must be of type DOUBLE.");

    id_y = mxGetFieldNumber(prhs,"y");
    if (id_y==-1)
        mexErrMsgTxt("Input structure must contain a field 'y'.");
	field_y = mxGetFieldByNumber(prhs, 0, id_y);
    if (!mxIsDouble(field_y))
        mexErrMsgTxt("Structure field 'y' must be of type DOUBLE.");

	id_hole = mxGetFieldNumber(prhs,"hole");
	if (id_hole==-1)
		read_hole_flag = FALSE;
	else
		read_hole_flag = TRUE;

		

/*  Passing data: Matlab -> C++	*/
	p->num_contours = mxGetNumberOfElements(prhs);

	p->hole = mxMalloc(p->num_contours * sizeof(int));
	p->contour = mxMalloc(p->num_contours * sizeof(gpc_vertex_list));

	for (c=0; c < p->num_contours; c++) {

		/* Check vertices */
		field_x = mxGetFieldByNumber(prhs, c, id_x);
		field_y = mxGetFieldByNumber(prhs, c, id_y);
        
		nx[0] = mxGetM(field_x);
		nx[1] = mxGetN(field_x);
		ny[0] = mxGetM(field_y);
		ny[1] = mxGetN(field_y);
		if (Min(nx,2)!=1 || Min(ny,2)!=1 || Max(nx,2)!=Max(ny,2))
			mexErrMsgTxt("Structure fields x and y must be non empty vectors of the same length.");

		p->contour[c].num_vertices = Max(nx,2);
		p->contour[c].vertex = mxMalloc(p->contour[c].num_vertices * sizeof(gpc_vertex));
		
		x = mxGetPr(field_x);
		y = mxGetPr(field_y);
		for (v= 0; v < p->contour[c].num_vertices; v++) {
			p->contour[c].vertex[v].x = x[v];
			p->contour[c].vertex[v].y = y[v];
		}

		if (read_hole_flag) {
            field_hole = mxGetFieldByNumber(prhs, c, id_hole);
            if (field_hole != NULL)
                p->hole[c] = mxGetScalar(field_hole);
            else
                p->hole[c] = 0;
        }
		if (!read_hole_flag || p->hole[c]!=1)
			p->hole[c] = FALSE; /* Assume all contours to be external */
        
  }
}


void gpc_write_polygon_MATLAB(mxArray *plhs, gpc_polygon *p)
{
	int c, v;
	int dims[2] = {1, p->num_contours};
	const char *field_names[] = {"x","y","hole"};
    
    mxArray *field_x, *field_y, *field_hole;
        
	for (c=0; c < p->num_contours; c++)
	{	
		field_x = mxCreateDoubleMatrix(p->contour[c].num_vertices,1,mxREAL);
		field_y = mxCreateDoubleMatrix(p->contour[c].num_vertices,1,mxREAL);
		for (v= 0; v < p->contour[c].num_vertices; v++)
		{
            ((double*)mxGetPr(field_x))[v]=p->contour[c].vertex[v].x;
            ((double*)mxGetPr(field_y))[v]=p->contour[c].vertex[v].y;
		}
        
        mxSetFieldByNumber(plhs,c,0,field_x);
        mxSetFieldByNumber(plhs,c,1,field_y);
       
		field_hole = mxCreateDoubleMatrix(1,1,mxREAL);
        ((double*)mxGetPr(field_hole))[0]=p->hole[c];
		mxSetFieldByNumber(plhs,c,2,field_hole);
    }
    gpc_free_polygon(p);
}

/* =================================
	END Im- / Export (Matlab)
	=================================*/
