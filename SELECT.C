/*---------------------------------------------------------------------------

File:                   Select.c

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           May 9, 1992
Last Modified:          August 29, 1992

Description:            This file contains the pixel selection routines
                        and fibonacci utility modules required for the
                        various rendering methods supported by this
                        project.


---------------------------------------------------------------------------*/


/*  Graphics and other supplied include files  */

#include <stdio.h>
#include <string.h>
#include <gxlib.h>
#include <grlib.h>
#include <malloc.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <math.h>


/*  Project include files  */

#include "select.h"
#include "surface.h"
#include "globals.h"
#include "display.h"
#include "errmsg.h"


/*  Global Variables  */

extern           GXDINFO        disp_info;
extern           REGION_TYPE    rightRegion, leftRegion;
extern           int            scanOne, scanTwo, fibOne, fibTwo, fibThree, leftVert,
                                rightVert, leftStart, rightStart;



/*  Internal function prototypes  */

int  Fibonacci (int, int);
int  Gcd (int, int);
void FatPixel (int, int, int, int, int);
void FatScan (int, int, int far *, int, int);
void SetRegion (int, int, int);
int  SelectPixel (int far *, int far *, int far *, int far *, int, long far *, REGION_TYPE, int, int far *);



/*----------------------------------------------------------------------------

      This procedure assigns normalized screen coordinates to the active
   rendering region(s).  The procedure requires the rendering mode, mono
   or dual and the two boolean variables for the status bar and the menu
   bar indicating an ON or OFF state.

----------------------------------------------------------------------------*/

void SetRegion (drawMode, status, menuOff)
int      drawMode, status, menuOff;
   {
      int      barHeight;


      /*  Calculate menu bar height  */

      if (!menuOff)
         if (disp_info.vres == V_CGA)
            barHeight = (int) (V_EXT_RATIO * disp_info.vres) + 1;
         else
            barHeight = (int) (V_RATIO * disp_info.vres) + 1;

      else
         barHeight = NO_MENU;


      /*  Assign Region(s)  */

      if (drawMode == DUAL)
         {
            rightRegion.upper_left.x = ((int) (disp_info.hres / 2) + SEP_WIDTH + 1);
            rightRegion.upper_left.y = barHeight + 1;
            rightRegion.lower_right.x = disp_info.hres - 1;
            rightRegion.lower_right.y = disp_info.vres - (status + BORDER);
            leftRegion.upper_left.x = X_ORIGIN;
            leftRegion.upper_left.y = barHeight + BORDER;
            leftRegion.lower_right.x = ((int) (disp_info.hres / 2) - (SEP_WIDTH + BORDER + 1));
            leftRegion.lower_right.y = disp_info.vres - (status + BORDER);
         }

      else
         {
            leftRegion.upper_left.x = X_ORIGIN;
            leftRegion.upper_left.y = barHeight + BORDER;
            leftRegion.lower_right.x = disp_info.hres - 1;
            leftRegion.lower_right.y = disp_info.vres - (status + BORDER);
         }

      return;
   }



/*----------------------------------------------------------------------------

      This function returns a fibonacci number based on the user supplied
   argument list.  The first argument is the sequence reference in to
   the fibonacci based sequence of numbers.  The second argument enables
   a slight deviation from the standard fibonacci sequence.  The function
   returns the appropriate fibonacci based number.

----------------------------------------------------------------------------*/

int Fibonacci (reference, alternate)
int      reference, alternate;
   {
      int   i, answer, first = 0, second, third;


      /*  Set up for alternate or standard Fibonacci sequence  */

      if (alternate)
         {
            second = 0;
            third = 1;
         }

      else 
         second = 1;


      /*  Iterate through the fibonacci sequence  */

      for (i = 0; i < (reference - 1); ++i)
         {
            if (alternate)
               {
                  /*  Fibonacci:  f(n) = f(n-1) + f(n-3)  */

                  answer = first + third;
                  first = second;
                  second = third;
                  third = answer;
               }
            else
               {
                  /*  Fibonacci:  f(n) = f(n-1) + f(n-2)  */

                  answer = first + second;
                  first = second;
                  second = answer;
               }

         }

      return (answer);
   }




/*----------------------------------------------------------------------------

      This function returns the greatest common denominator between two
   argument list supplied integers.  

----------------------------------------------------------------------------*/

int Gcd (numOne, numTwo)
int      numOne, numTwo;
   {
      int      i, maxOne, maxTwo, min, answer = 1;


      /*  Set the maximum range of numbers for test  */

      maxOne = (int) sqrt (numOne);
      maxTwo = (int) sqrt (numTwo);


      /*  Set up expected orientation of variables to be tested  */

      if (maxOne < maxTwo)
         min = maxOne;
      else
         min = maxTwo;


      /*  Iterate the minimum number of times to determin the GCD between
          the two numbers                                                  */

      for (i = 2; i < min; ++i)
         if ((numOne % i) == 0 && (numTwo % i) == 0)
            answer = i;

      return (answer);
   }



/*----------------------------------------------------------------------------

      This procedure Takes the screen coordinates of a pixel a width
   and intensity value and a screen region indicator and plots an
   "artificially fat" pixel.

----------------------------------------------------------------------------*/

void FatPixel (xPos, yPos, wide, intensity, active)
int      xPos, yPos, wide, intensity, active;
   {
      /*  Plot a fat pixel  */

      if (wide > 0)
         {
            grSetFillStyle (grFSOLID, intensity, grOPAQUE);


            /*  Calculate the size and position of the large pixel  */

            if (active == LEFT)
               grDrawRect (max (xPos - wide, X_ORIGIN), max (yPos - wide, Y_ORIGIN),
                           min (xPos + wide, leftRegion.lower_right.x), min (yPos + wide, leftRegion.lower_right.y), grFILL);

            else
               grDrawRect (max (xPos - wide, X_ORIGIN), max (yPos - wide, Y_ORIGIN),
                           min (xPos + wide, rightRegion.lower_right.x), min (yPos + wide, rightRegion.lower_right.y), grFILL);
         }

      else
         {
            /* Plot a normal pixel  */

            grPutPixel (xPos, yPos, intensity);
         }

      return;
   }



/*----------------------------------------------------------------------------

      This procedure takes the current screen coordinates and the specified
   scanline width, drawing orientation and intensity and plots fat scan
   lines.  This procedure is called with continually decreasing values of
   width, until there is only a single pixel width to plot.  

----------------------------------------------------------------------------*/

void FatScan (xPos, yPos, wide, vert, intensity)
int      xPos, yPos, *wide, vert, intensity;
   {
      int      startPos;


      if (*wide > 1)
         {
            /*  Set a fat line  */

            grSetLineStyle (grLSOLID, 1);
            grSetColor (intensity);
            if (vert)
               {
                  /*  Vertical orientation  */

                  if (xPos < *wide)
                     *wide = xPos;

                  startPos = (xPos - *wide) + 1;
                  if (startPos == 1)
                     startPos = 0;

                  grDrawLine (startPos, yPos, xPos, yPos);
               }

            else
               {
                  /*  Horizontal orientation  */

                  if (yPos < *wide)
                     *wide = yPos;

                  startPos = (yPos - *wide) + 1;
                  if (startPos == 1)
                     startPos = 0;

                  grDrawLine (xPos, startPos, xPos, yPos);
               }

         }

      else
         {
            /*  Single line width  */

            grPutPixel (xPos, yPos, intensity);
         }

      return;
   }



/*----------------------------------------------------------------------------

      This function is the key component in determing the rendering method
   of any image.  This module is responsible for determining the next pixel
   for display based upon precious selections and the rendition method
   currently being used.  It must handle the left, right or both regions
   in either horizontal or vertical orientations (for scan line methods
   only).  This function can select pixel from any of the five rendition
   models.  Finally, this modules determines when all the pixels from
   each region have been displayed and signals the driver module that
   rendering is complete.  The module is passed the variables that contain
   previously displayed row and column coordinates.  Also the total pixel
   count for the status bar completion percentage is passed and locally
   modified.  The method of rendition is passed as an argument as is 
   the total number of fibonacci iterations (used for the elimination of
   plotting outside the predefined region(s).  Finally, the region
   boundaries, screen region indicator and a pixel width variable complete
   the argument list.  The return value is TRUE if rendering is completed 
   and FALSE otherwise.

----------------------------------------------------------------------------*/

int SelectPixel (column, row, count, cont, method, iter, region, side, wide)
int             far *column, far *row, far *count, far *cont, method;
long            far *iter;
REGION_TYPE     region;
int             side, far *wide;
   {
      int      done = FALSE;

      switch (method)
         {
            /*  Iterative method for this region  */

            case ITERATIVE:
               if ((side == LEFT && !leftStart) || (side == RIGHT && !rightStart))
                  {
                     if ((side == LEFT && leftVert) || (side == RIGHT && rightVert))
                        {
                           /*  Orientation is vertical, so increment the row, check
                               for the bottom edge and start back at the top as needed  */

                           ++*row;
                           if (*row > (region.lower_right.y - region.upper_left.y))
                              {
                                 *row = 0;
                                 ++*column;


                                 /*  Last row and last column, this side is all done!  */

                                 if (*column > (region.lower_right.x - region.upper_left.x))
                                    return (TRUE);

                              }

                        }

                     else
                        {
                           /*  Orientation is horizontal, so increment the column, check
                               for the right edge and start back at the left as needed  */


                           ++*column;
                           if (*column > (region.lower_right.x - region.upper_left.x))
                              {
                                 *column = 0;
                                 ++*row;


                                 /*  Last column and last row, this region is all done!  */

                                 if (*row > (region.lower_right.y - region.upper_left.y))
                                    return (TRUE);

                              }

                        }

                  }


               /*  Take care of first pixel selected in this method  */

               if (side == LEFT && leftStart)
                  leftStart = FALSE;

               if (side == RIGHT && rightStart)
                  rightStart = FALSE;

               break;


            /*  Either the standard or fat uniform scan line method was chosen  */

            case FATFIBSCAN:
            case FIBSCAN:
               if (*cont)
                  {
                     /*  Select the remaining pixels on the current scna line  */

                     if ((side == LEFT && leftVert) || (side == RIGHT && rightVert))
                        {
                           /*  Orientation is vertical for this region  */

                           ++*row;
                           if ((*row + 1) > (region.lower_right.y - region.upper_left.y))
                              *cont = FALSE;

                        }

                     else
                        {
                           /*  Orientation is horizontal for this region  */

                           ++*column;
                           if ((*column + 1) > (region.lower_right.x - region.upper_left.x))
                              *cont = FALSE;

                        }

                  }

               else
                  {
                     /*  Select a new scan line  */

                     if ((side == LEFT && leftVert) || (side == RIGHT && rightVert))
                        {
                           /*  Select a vertical scan line inside the clipping 
                               region                                            */

                           while (!done)
                              {
                                 *column = ((long) *column + scanOne) % scanTwo;
                                 if (*column < (region.lower_right.x - region.upper_left.x) + 1)
                                    done = TRUE;

                                 ++*count;
                              }

                           *row = Y_ORIGIN;
                           if (*count > scanTwo)
                              return (TRUE);

                           *cont = TRUE;
                        }

                     else
                        {
                           /*  Select a horizontal scan line inside the clipping
                               region                                            */

                           while (!done)
                              {
                                 *row = ((long) *row + scanOne) % scanTwo;
                                 if (*row < (region.lower_right.y - region.upper_left.y) + 1)
                                    done = TRUE;

                                 ++*count;
                              }

                           *column = X_ORIGIN;
                           if (*count > scanTwo)
                              return (TRUE);

                           *cont = TRUE;
                        }

                  }

               break;


            /*  Either a standard or fat uniform pixel selection method was chosen  */

            case FATFIB:
            case FIBONACCI:
               /*  Check to see if this region is completely rendered  */

               if (*iter >= ((long) fibTwo * fibOne))
                  return (TRUE);


               /*  Make sure the selected pixel is within the clipping region  */

               while (!done)
                  {
                     *row = (*iter * fibThree) % fibTwo;
                     *column = (*iter * fibThree) % fibOne;
                     ++*iter;


                     /*  Decrease the fat pixel width at predetermined interval  */

                     if ((method == FATFIB) && (*iter == FIRST_RED || *iter == SECOND_RED || *iter == THIRD_RED))
                        --*wide;

                     if (*row <= (region.lower_right.y - region.upper_left.y) && *column <= (region.lower_right.x -
                         region.upper_left.x))
                        done = TRUE;

                  }

               break;

         }

      return (FALSE);
   }
