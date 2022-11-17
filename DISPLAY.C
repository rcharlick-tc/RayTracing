/*----------------------------------------------------------------------------

File:                   Display.c

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 9, 1992
Last Modified:          July 25, 1992

Description:            This file contains functions and procedures specific
                        to the drawing, updating and clearing of status and
                        information displays.  Also included are some utility
                        graphics functions for palette assignment, render
                        mode display and rendering method decoding.


----------------------------------------------------------------------------*/


/*    Standard and Graphics Include files                                   */

#include <gxlib.h>
#include <txlib.h>
#include <grlib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <malloc.h>



/*    Project include files                                                 */

#include "errmsg.h"
#include "display.h"
#include "globals.h"
#include "surface.h"
#include "select.h"



/*    Global variables                                                      */

extern   GXDINFO         disp_info;
extern   POINT_TYPE      statPos [MAX_FIELD];
extern   REGION_TYPE     rightRegion, leftRegion;
         TXHEADER        tv;


/*    Internal function prototypes                                          */

int  DisplayStatus (int, int, int);
void GetFileName (char far *, int, int);
void StatusText (int);
int  ClearStatus (int);
int  ShowStatus (int, int, char *, char *, int, int, int);
void ShowInfo (int, int);
void Alert (char far *, int);
void ShowTitle (int);
int  UpdateStatus (int, int far *, int far *, time_t, long, long);
int  AssignPalette (int);
void DrawSep (int, int, int);
void Decode (char far *, int, int);
void GetButton (int, int, int, int);



/*----------------------------------------------------------------------------

      This function is responsible for determining the status region size 
   from the given display mode and state of the rendering program.  The
   function then draws the status region box.  The function takes some
   general screen mode information, the number of colors for the current
   video mode and a color argument for selecting the status box color.
   DisplayStatus will return the height of the status box.

----------------------------------------------------------------------------*/

int DisplayStatus (colors, selectColor, mode)
int      colors, selectColor, mode;
   {
      int      x1, y1, x2, y2, width, height;


      width = disp_info.hres - 1;
      x1 = X_ORIGIN;


      /*  Determine the rendering mode and draw the status box(es) to the
          required specifications of the calling routine.                   */

      if (mode == DUAL)
         {
            height = (int) (RATIO_DUAL * disp_info.vres);
            y1 = (disp_info.vres - 1) - height;
            x2 = ((int) (disp_info.hres / 2) - SEP_WIDTH) - 1;
            y2 = disp_info.vres - 1;


            /*  Clear the status region  */

            grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
            grDrawRect (X_ORIGIN, y1, width, y2, grFILL);
            if (colors >= REQD_COLORS)
               {
                  /*  Draw the left box in 3D  */

                  grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                  grDrawRect (x1, y1, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);

                  x1 = x2 + (2 * SEP_WIDTH) + 1;
                  x2 = width;


                  /*  Draw the right box in 3D  */

                  grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                  grDrawRect (x1, y1, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);
               }

         }

      else
         {
            height = (int) (RATIO_MONO * disp_info.vres);
            y1 = (disp_info.vres - 1) - height;
            x2 = width;
            y2 = disp_info.vres - 1;
            if (colors >= REQD_COLORS)
               {
                  /*  Draw the single region status box in 3D  */

                  grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                  grDrawRect (x1, y1, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
                  grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
                  grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);
               }

         }

      return (height + Y_SHADE);
   }




/*----------------------------------------------------------------------------

      This function will clear the status box region, in mono or dual 
   rendering mode and return a success code.

----------------------------------------------------------------------------*/

int ClearStatus (mode)
int      mode;
   {
      int      height;


      /*  Determine the menu bar height  */

      if (mode == DUAL)
         height = (int) (RATIO_DUAL * disp_info.vres);
      else
         height = (int) (RATIO_MONO * disp_info.vres);


      /*  Clear the status region  */

      grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
      grDrawRect (X_ORIGIN, (disp_info.vres - 1) - height, disp_info.hres - 1,
                  disp_info.vres - 1, grFILL);

      return (VALID);
   }



/*----------------------------------------------------------------------------

      This procedure displays the status bar title header information for
   either mono or dual rendering modes.  The single argument is the mode 
   variable.  No value is returned.

----------------------------------------------------------------------------*/

void StatusText (mode)
int      mode;
   {
      int      xAdj, yAdj, xPos, yPos, cHeight, strSize;
      int      lx1, ly1, lx2, ly2, rx1, ry1, rx2, ry2, width, height;
      int      pixline, lMargin, rMargin, lastX, prevX;


      /*  Initialization of position variables, fonts and colors  */

      grSetTextStyle (grTXT8X14, grTRANS);
      cHeight = HIRES;
      width = disp_info.hres - 1;
      lx1 = X_ORIGIN + X_SHADE;
      grSetColor (grDARKGRAY);
      if (mode == DUAL)
         {
            /*  Assignment of margins and incremental variables  */

            height = (int) (RATIO_DUAL * disp_info.vres);
            ly1 = ((disp_info.vres - 1) - height) + Y_SHADE;
            lx2 = ((int) ((disp_info.hres / 2) - SEP_WIDTH) - 1) - X_SHADE;
            ly2 = (disp_info.vres - 1) - Y_SHADE;
            rx1 = (lx2 + (2 * SEP_WIDTH) + 1) + X_SHADE;
            rx2 = width - X_SHADE;
            lMargin = (int) (lx2 - lx1) * M_RATIO;
            rMargin = (int) (rx2 - rx1) * M_RATIO;
            pixline = (int) ((ly2 - ly1) + 1) / HIDLINE;
            yAdj = (int) (pixline - cHeight) / 2;
            xAdj = (int) (lx2 - (lx1 + lMargin)) / HDNUM;


            /*  Print header and assign position for "method" field  */

            yPos = ly1 + yAdj;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Method");
            statPos [METH_FLD].x = xPos + METH_SIZE;
            statPos [METH_FLD].y = yPos;
            xPos = rx1 + rMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Method");


            /*  Print header and assign position for "image" field  */

            yPos += pixline;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Image");
            statPos [IMG_FLD].x = xPos + IMG_SIZE;
            statPos [IMG_FLD].y = yPos;
            lastX = xPos;
            xPos = rx1 + rMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Image");
            prevX = xPos;


            /*  Print header and assign position for "size" field  */

            xPos = lastX + (int) (FLD_ADJ * xAdj);
            grMoveTo (xPos, yPos);
            grOutText ("Size");
            statPos [REG_FLD].x = xPos + REG_SIZE;
            statPos [REG_FLD].y = yPos;
            xPos = prevX + (int) (FLD_ADJ * xAdj);
            grMoveTo (xPos, yPos);
            grOutText ("Size");


            /*  Print header and assign position for "flood" field  */

            yPos += pixline;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Flood");
            statPos [FLO_FLD].x = xPos + FLO_SIZE;
            statPos [FLO_FLD].y = yPos;
            lastX = xPos;
            xPos = rx1 + rMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Flood");
            prevX = xPos;


            /*  Print header and assign position for "saturation" field  */

            xPos = lastX + xAdj;
            grMoveTo (xPos, yPos);
            grOutText ("Saturation");
            statPos [SAT_FLD].x = xPos + SAT_FULL;
            statPos [SAT_FLD].y = yPos;
            xPos = prevX + xAdj;
            grMoveTo (xPos, yPos);
            grOutText ("Saturation");


            /*  Print header and assign position for "completion" field  */

            yPos += pixline;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Complete");
            statPos [PCT_FLD].x = xPos + PCT_ABR;
            statPos [PCT_FLD].y = yPos;
            lastX = xPos;
            xPos = rx1 + rMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Complete");
            prevX = xPos;


            /*  Print header and assign position for "elapsed time" field  */

            xPos = lastX + (2 * xAdj) - (3 * CHAR_WIDTH);
            grMoveTo (xPos, yPos);
            grOutText ("Time");
            statPos [ELAP_FLD].x = xPos + ELAP_ABR;
            statPos [ELAP_FLD].y = yPos;
            xPos = prevX + (2 * xAdj) - (3 * CHAR_WIDTH);
            grMoveTo (xPos, yPos);
            grOutText ("Time");
         }

      else
         {
            /*  Mono Mode  */

            /*  Assignment of incremental variables  */

            height = (int) (RATIO_MONO * disp_info.vres);
            ly1 = ((disp_info.vres - 1) - height) + Y_SHADE;
            lx2 = width - X_SHADE;
            ly2 = (disp_info.vres - 1) - Y_SHADE;
            lMargin = (int) (lx2 - lx1) * M_RATIO;
            pixline = (int) ((ly2 - ly1) + 1) / HIMLINE;
            yAdj = (int) (pixline - cHeight) / 2;
            xAdj = (int) (lx2 - (lx1 + lMargin)) / HMNUM;


            /*  Print header and assign position for "method" field  */

            yPos = ly1 + yAdj;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Method");
            statPos [METH_FLD].x = xPos + METH_SIZE;
            statPos [METH_FLD].y = yPos;
            lastX = xPos;


            /*  Print header and assign position for "image" field  */

            xPos = lastX + (2 *xAdj);
            grMoveTo (xPos, yPos);
            grOutText ("Image");
            statPos [IMG_FLD].x = xPos + IMG_SIZE;
            statPos [IMG_FLD].y = yPos;
            lastX = xPos;


            /*  Print header and assign position for "size" field  */

            yPos += pixline;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Size");
            statPos [REG_FLD].x = xPos + REG_SIZE;
            statPos [REG_FLD].y = yPos;
            lastX = xPos;


            /*  Print header and assign position for "flood" field  */

            xPos = lastX + xAdj;
            grMoveTo (xPos, yPos);
            grOutText ("Flood");
            statPos [FLO_FLD].x = xPos + FLO_SIZE;
            statPos [FLO_FLD].y = yPos;
            lastX = xPos;


            /*  Print header and assign position for "saturation" field  */

            xPos = lastX + xAdj;
            grMoveTo (xPos, yPos);
            grOutText ("Saturation");
            statPos [SAT_FLD].x = xPos + SAT_FULL;
            statPos [SAT_FLD].y = yPos;


            /*  Print header and assign position for "percent complete" field  */

            yPos += pixline;
            xPos = lx1 + lMargin;
            grMoveTo (xPos, yPos);
            grOutText ("Percent Complete");
            statPos [PCT_FLD].x = xPos + PCT_FULL;
            statPos [PCT_FLD].y = yPos;
            lastX = xPos;


            /*  Print header and assign position for "elapsed time" field  */

            xPos = lastX + (2 * xAdj);
            grMoveTo (xPos, yPos);
            grOutText ("Elapsed Time");
            statPos [ELAP_FLD].x = xPos + ELAP_FULL;
            statPos [ELAP_FLD].y = yPos;
         }

      return;
   }




/*----------------------------------------------------------------------------

      This function displays the static status bar information for each
   rendering.  For either dual or mono mode, the rendering method, surface
   name, flood status and saturation angle is displayed.  The function
   returns a success or error value.

----------------------------------------------------------------------------*/

int ShowStatus (mode, method, leftScene, rightScene, flood, saturate, extent)
int      mode, method;
char     far *leftScene, far *rightScene;
int      flood, saturate, extent;
   {
      int       xSize, ySize, cHeight, offSet;
      char      far *methStr, far *satStr, far *regStr;


      /*  Allocation and verification of memory for string variables  */

      methStr = (char far *) farcalloc (LARGE_STR, sizeof (char));
      satStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      regStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      if (methStr == NULL || satStr == NULL || regStr == NULL)
         return (m_ERR_ALLOC);

      grSetTextStyle (grTXT8X14, grTRANS);
      cHeight = HIRES;
      grSetFillStyle (grFSOLID, grGRAY, grOPAQUE);
      grSetColor (grBLUE);


      /*  Display the right side status screen in dual mode  */

      if (mode == DUAL)
         {
            xSize = (rightRegion.lower_right.x - rightRegion.upper_left.x) + 1;
            ySize = (rightRegion.lower_right.y - rightRegion.upper_left.y) + 1;
            offSet = disp_info.hres / 2;


            /*  Display the pixel selection method  */

            if (extent == FULL || extent == METHOD)
               {
                  Decode (methStr, RIGHT, method);
                  grDrawRect (statPos [METH_FLD].x + offSet, statPos [METH_FLD].y + 1, statPos [METH_FLD].x + 
                             (MAX_METH * CHAR_WIDTH) + offSet, (statPos [METH_FLD].y + cHeight) - 1, grFILL);
                  grMoveTo (statPos [METH_FLD].x + offSet, statPos [METH_FLD].y);
                  grOutText (methStr);
               }


            /*  Display the algebraic surface name  */

            if (extent == FULL || extent == SCENE)
               {
                  grDrawRect (statPos [IMG_FLD].x + offSet, statPos [IMG_FLD].y + 1, statPos [IMG_FLD].x + 
                             (MAX_IMG * CHAR_WIDTH) + offSet, (statPos [IMG_FLD].y + cHeight) - 1, grFILL);
                  grMoveTo (statPos [IMG_FLD].x + offSet, statPos [IMG_FLD].y);
                  grOutText (rightScene);
               }


            /*  Display the rendering region size in pixels  */

            if (extent == FULL || extent == AREA)
               {
                  grDrawRect (statPos [REG_FLD].x + offSet, statPos [REG_FLD].y + 1, statPos [REG_FLD].x + 
                             (MAX_REG * CHAR_WIDTH) + offSet, (statPos [REG_FLD].y + cHeight) - 1, grFILL);
                  grMoveTo (statPos [REG_FLD].x + offSet, statPos [REG_FLD].y);
                  itoa (xSize, regStr, BASE);
                  _fstrcat (regStr, " x ");
                  itoa (ySize, satStr, BASE);
                  _fstrcat (regStr, satStr);
                  grOutText (regStr);
               }


            /*  Display the flood status  */

            if (extent == FULL || extent == FLOOD)
               {
                  grDrawRect (statPos [FLO_FLD].x + offSet, statPos [FLO_FLD].y + 1, statPos [FLO_FLD].x + 
                             (MAX_FLO * CHAR_WIDTH) + offSet, (statPos [FLO_FLD].y + cHeight) - 1, grFILL);
                  grMoveTo (statPos [FLO_FLD].x + offSet, statPos [FLO_FLD].y);
                  if (flood)
                     grOutText ("ON");
                  else
                     grOutText ("OFF");

               }


            /*  Display the saturation angle  */

            if (extent == FULL)
               {
                  grDrawRect (statPos [SAT_FLD].x + offSet, statPos [SAT_FLD].y + 1, statPos [SAT_FLD].x + 
                             (MAX_SAT * CHAR_WIDTH) + offSet, (statPos [SAT_FLD].y + cHeight) - 1, grFILL);
                  grMoveTo (statPos [SAT_FLD].x + offSet, statPos [SAT_FLD].y);
                  itoa (saturate, satStr, BASE);
                  grOutText (satStr);
               }

         }

      xSize = (leftRegion.lower_right.x - leftRegion.upper_left.x) + 1;
      ySize = (leftRegion.lower_right.y - leftRegion.upper_left.y) + 1;


      /*  Display the left side status screen in either mode  */

      /*  Display the pixel selection method  */

      if (extent == FULL || extent == METHOD)
         {
            Decode (methStr, LEFT, method);
            grDrawRect (statPos [METH_FLD].x, statPos [METH_FLD].y + 1, statPos [METH_FLD].x + MAX_METH * CHAR_WIDTH,
                       (statPos [METH_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [METH_FLD].x, statPos [METH_FLD].y);
            grOutText (methStr);
         }


      /*  Display the algrebraic surface name  */

      if (extent == FULL || extent == SCENE)
         {
            grDrawRect (statPos [IMG_FLD].x, statPos [IMG_FLD].y + 1, statPos [IMG_FLD].x + MAX_IMG * CHAR_WIDTH,
                      (statPos [IMG_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [IMG_FLD].x, statPos [IMG_FLD].y);
            grOutText (leftScene);
         }


      /*  Display the size of the rendering region in pixels  */

      if (extent == FULL || extent == AREA)
         {
            grDrawRect (statPos [REG_FLD].x, statPos [REG_FLD].y + 1, statPos [REG_FLD].x + MAX_REG * CHAR_WIDTH,
                       (statPos [REG_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [REG_FLD].x, statPos [REG_FLD].y);
            itoa (xSize, regStr, BASE);
            _fstrcat (regStr, " x ");
            itoa (ySize, satStr, BASE);
            _fstrcat (regStr, satStr);
            grOutText (regStr);
         }


      /*  Display the flood variable status  */

      if (extent == FULL || extent == FLOOD)
         {
            grDrawRect (statPos [FLO_FLD].x, statPos [FLO_FLD].y + 1, statPos [FLO_FLD].x + MAX_FLO * CHAR_WIDTH,
                       (statPos [FLO_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [FLO_FLD].x, statPos [FLO_FLD].y);
            if (flood)
               grOutText ("ON");
            else
               grOutText ("OFF");

         }


      /*  Display the value of the saturation angle  */

      if (extent == FULL)
         {
            grDrawRect (statPos [SAT_FLD].x, statPos [SAT_FLD].y + 1, statPos [SAT_FLD].x + MAX_SAT * CHAR_WIDTH,
                       (statPos [SAT_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [SAT_FLD].x, statPos [SAT_FLD].y);
            itoa (saturate, satStr, BASE);
            grOutText (satStr);
         }


      /*  House cleaning  */

      farfree (methStr);
      farfree (satStr);
      return (m_VALID);
   }




/*----------------------------------------------------------------------------

      This function dynamically updates the status bar values percent
   complete and elapsed time.  The number of pixels rendered, total 
   number of pixels, start time of the rendering, display mode and
   previous values of elapsed time and completion percentage are
   passed as parameters.  The function returns a success value.

----------------------------------------------------------------------------*/

int UpdateStatus (mode, oldComp, oldTime, startTime, totPixel, numPixel)
int      mode, far *oldComp, far *oldTime;
time_t   startTime;
long     totPixel, numPixel;
   {
      time_t      currTime;
      int         offSet, complete, cHeight, totSecs, hrs, mins, secs;
      char        far *compStr, far *timStr, far *tempStr;


      /*  Allocation and verification of string variables  */

      compStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      timStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      tempStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      if (compStr == NULL || timStr == NULL || tempStr == NULL)
         return (m_ERR_ALLOC);


      /*  Reorient the viewport for dynamic status updates  */

      grSetViewPort (X_ORIGIN, Y_ORIGIN, disp_info.hres, disp_info.vres);
      grSetClipping (grNOCLIP);


      /*  Calculate percentage complete  */

      complete = (int) (((double) totPixel / (double) numPixel) * 100);
      itoa (complete, compStr, BASE);
      _fstrcat (compStr, " %");


      /*  Determine elapsed time and build time string  */

      currTime = time (NULL);
      totSecs = abs (difftime (startTime, currTime));
      hrs  = (int) totSecs / SECHR;
      totSecs = totSecs % SECHR;
      mins = (int) totSecs / SECMIN;
      secs = (int) totSecs % SECMIN;
      itoa (hrs, tempStr, BASE);
      if (hrs < 10)
         {
            _fstrcpy (timStr, "0");
            _fstrcat (timStr, tempStr);
         }

      else
         _fstrcpy (timStr, tempStr);

      strcat (timStr, ":");
      itoa (mins, tempStr, BASE);
      if (mins < 10)
         {
            _fstrcat (timStr, "0");
            _fstrcat (timStr, tempStr);
         }

      else
         _fstrcat (timStr, tempStr);

      strcat (timStr, ":");
      itoa (secs, tempStr, BASE);
      if (secs < 10)
         {
            strcat (timStr, "0");
            strcat (timStr, tempStr);
         }

      else
         strcat (timStr, tempStr);


      /*  Output completion percentage and elapsed time strings  */

      grSetTextStyle (grTXT8X14, grTRANS);
      cHeight = HIRES;
      grSetColor (grRED);
      if (mode == LEFT)
         offSet = NONE;
      else
         offSet = disp_info.hres / 2;

      grSetFillStyle (grFSOLID, grGRAY, grOPAQUE);
      if (*oldComp != complete)
         {
            grDrawRect (statPos [PCT_FLD].x + offSet, statPos [PCT_FLD].y + 1, statPos [PCT_FLD].x + (MAX_PCT * CHAR_WIDTH) + 
                        offSet, (statPos [PCT_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [PCT_FLD].x + offSet, statPos [PCT_FLD].y);
            grOutText (compStr);
         }

      if (*oldTime != totSecs)
         {
            grDrawRect (statPos [ELAP_FLD].x + offSet, statPos [ELAP_FLD].y + 1, statPos [ELAP_FLD].x + (MAX_ELAP * CHAR_WIDTH) + 
                        offSet, (statPos [ELAP_FLD].y + cHeight) - 1, grFILL);
            grMoveTo (statPos [ELAP_FLD].x + offSet, statPos [ELAP_FLD].y);
            grOutText (timStr);
         }


      /*  Retain previous values of completion and time to avoid redundant
          output                                                            */

      *oldComp = complete;
      *oldTime = totSecs;


      /*  House cleaning  */

      farfree (compStr);
      farfree (timStr);
      farfree (tempStr);

      return (m_VALID);
   }




/*----------------------------------------------------------------------------

      This procedure draws and erase the dual mode screen seperation bar.
   The argument list consists of the seperator bar color, the height of
   the status bar and a status of whether or not there is a main menu bar.

----------------------------------------------------------------------------*/

void DrawSep (color, statusHeight, fullScreen)
int      color, statusHeight, fullScreen;
   {
      int         x1, y1, x2, y2;


      /*  Assign the starting vertical region value  */

      if (fullScreen)
         y1 = 0;
      else
         if (disp_info.vres == V_CGA)
            y1 = (int) (V_EXT_RATIO * disp_info.vres) + 1;
         else
            y1 = (int) (V_RATIO * disp_info.vres) + 1;


      /*  Assign the remaining three vertices for the seperator bar  */

      y2 = (disp_info.vres - statusHeight) - 1;
      x1 = ((int) (disp_info.hres / 2) - SEP_WIDTH);
      x2 = (x1 + (2 * SEP_WIDTH)) - 1;


      /*  Draw the Bar  */

      grSetFillStyle (grFSOLID, color, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);

      return;
    }




/*----------------------------------------------------------------------------

      This function assigns the color palette for the current display mode.
   This version of the palette function requires 256 simultaneous colors.
   A variable containing the number of colors for the current video mode is
   passed as the only parameter.  The function returns a success or error
   code.

----------------------------------------------------------------------------*/

int AssignPalette (colors)
int      colors;
   {
      int      i, rct, gct, bct, status = VALID;
      char     pal256 [PAL_SVGA];


      if (colors == REQD_COLORS)
         {
            gxGetDisplayPalette (pal256);


            /*  Assign flood colors (light red to white)  */

            rct = MAX_INTENSITY;
            bct = gct = BLEND;
            for (i = BCOLOR_1; i < BCOLOR_2; ++i)
               if ((i % RGB) == RED_SHADE)
                  pal256 [i] = rct;
               else
                  if ((i % RGB) == GREEN_SHADE)
                     pal256 [i] = gct += RGB;
                  else
                     pal256 [i] = bct += RGB;


            /*  Assign flood colors (light green to white)  */

            gct = MAX_INTENSITY;
            rct = bct = BLEND;
            for (i = BCOLOR_2; i < BCOLOR_3; ++i)
               if ((i % RGB) == GREEN_SHADE)
                  pal256 [i] = gct;
               else
                  if ((i % RGB) == RED_SHADE)
                     pal256 [i] = rct += RGB;
                  else
                     pal256 [i] = bct += RGB;


            /*  Assign flood colors (light blue to white)  */

            bct = MAX_INTENSITY;
            rct = gct = BLEND;
            for (i = BCOLOR_3; i < VCOLOR_1; ++i)
               if ((i % RGB) == BLUE_SHADE)
                  pal256 [i] = bct;
               else
                  if ((i % RGB) == RED_SHADE)
                     pal256 [i] = rct += RGB;
                  else
                     pal256 [i] = gct += RGB;


            /*  Assign varying degrees of red  */

            rct = gct = bct = OFF;
            for (i = VCOLOR_1; i < VCOLOR_2; ++i)
               if ((i % RGB) == RED_SHADE)
                  pal256 [i] = rct++;
               else
                  pal256 [i] = OFF;


            /*  Assign varying degrees of green  */

            rct = gct = bct = OFF;
            for (i = VCOLOR_2; i < VCOLOR_3; ++i)
               if ((i % RGB) == GREEN_SHADE)
                  pal256 [i] = gct++;
               else
                  pal256 [i] = OFF;


            /*  Assign varying debrees of blue  */

            rct = gct = bct = OFF;
            for (i = VCOLOR_3; i < END_VCOLOR; ++i)
               if ((i % RGB) == BLUE_SHADE)
                  pal256 [i] = bct++;
               else
                  pal256 [i] = OFF;

            status = gxSetDisplayPalette (pal256);
         }

      return (status);
   }




/*----------------------------------------------------------------------------

      This procedure decodes the binary encoded pixel selection variable
   and assigns a valid description to a string variable passed as an
   argument.  Other than the string to be modified and the method variable,
   a third argument indicating the left or right rendering region is
   supplied.  

----------------------------------------------------------------------------*/

void Decode (mStr, side, method)
char   far *mStr;
int    side, method;
   {
      int       vert, type;


      /*  Determine the orientation (vertical or horizontal) for scan line
          methods                                                           */

      if (side == LEFT)
         {
            type = (method >> R_SHIFT) & R_MASK;
            vert = (method >> RO_SHIFT) & VH_MASK;
         }

      else
         {
            type = method & R_MASK;
            vert = (method >> LO_SHIFT) & VH_MASK;
         }


      /*  Assign the orientation where appropriate  */

      if (type == ITERATIVE || type == FIBSCAN || type == FATFIBSCAN)
         if (vert)
            _fstrcpy (mStr, "Vertical");
         else
            _fstrcpy (mStr, "Horizontal");

      else
         _fstrcpy (mStr, "");


      /*  Determine the selection method and assign the description  */

      switch (type)
         {
            case ITERATIVE:
               _fstrcat (mStr, " Iterative");
               break;

            case FIBSCAN:
               _fstrcat (mStr, " Uniform Scan");
               break;

            case FIBONACCI:
               _fstrcat (mStr, "Uniform Pixel");
               break;

            case FATFIB:
               _fstrcat (mStr, "Uniform Fat Pixel");
               break;

            case FATFIBSCAN:
               _fstrcat (mStr, " Uniform Fat Scan");
               break;

            default:
               _fstrcat (mStr, "Unknown Method");
               break;

         }

      return;
   }




/*----------------------------------------------------------------------------

      This function retrieves the name of a PCX file from an interactive
   message box.  This function is used in conjunction with saving and
   restoring PCX files.

----------------------------------------------------------------------------*/

void GetFileName (fileStr, mode, selectColor)
char     far *fileStr;
int      mode, selectColor;
   {
      int      x1, x2, y1, y2, xPos, yPos, yAdj, height, width, size;
      char     mask [INPUT_STR], tempStr [INPUT_STR];


      /*  Draw the file name box  */

      height = (int) (VERT_DIALOG * disp_info.vres);
      width = (int) (HOR_TITLE * disp_info.hres);
      x1 = (int) (disp_info.hres / ALGN_CNTR2);
      y1 = (int) (disp_info.vres / ALGN_CNTR2);
      x2 = (int) (disp_info.hres / ALGN_CNTR2) + width;
      y2 = (int) (disp_info.vres / ALGN_CNTR2) + height;
      grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);

      /*  Display the screen header  */

      xPos = (int) x1 + ((width - (11 * CHAR_WIDTH)) / 2);
      yPos = (int) y1 + (height / 8);
      grSetColor (grBLUE);
      grMoveTo (xPos, yPos);
      if (mode == REST_MODE)
         grOutText ("RESTORE PCX");
      else
         grOutText (" SAVE PCX");


      /*  Get the input file name  */

      _fstrcpy (tempStr, "            ");
      _fstrcpy (mask,    "XXXXXXXXXXXX");
      xPos = (int) x1 + CHAR_WIDTH * 2;
      yPos += (int) height * FILE_SPACE;
      txRomFont (gxCMM, txTXT8X16, &tv);
      txSetFont (&tv);
      txSetColor (gxWHITE, selectColor);
      txPutString ("Enter file name: ", xPos, yPos);
      xPos += (int) CHAR_WIDTH * 18;
      txGetString (tempStr, mask, xPos, yPos, '|', '_');
      grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      tempStr [INPUT_STR - 1] = '\0';
      _fstrcpy (fileStr, tempStr);
      txFreeFont (&tv);

      return;
   }



/*----------------------------------------------------------------------------

      This procedure displays an information screen containing performance
   statistics for the preceeding screen rendering.  The arguments supplied
   are the color of the information screen and the elapsed time in seconds
   that the preceeding rendering took.  The procedure calculates the number
   of pixels and the throughput of the rendering method in pixels per
   second.

----------------------------------------------------------------------------*/

void ShowInfo (selectColor, seconds)
int      selectColor, seconds;
   {
      int      mins, secs, height, width, x1, y1, x2, y2, xSize, ySize;
      int      bSize, xPos, yPos, xAdj, yAdj, margin, pixline, thruPut;
      int      bx1, bx2, by1;
      char     far *regStr, far *satStr, far *timStr, far *tempStr;
      long     numPix;


      /*  Allocation and verification of string variables  */

      timStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      tempStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      satStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      regStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      if (tempStr == NULL || timStr == NULL || regStr == NULL || satStr == NULL)
         return;


      /*  Draw the information box  */

      height = (int) (VERT_INFO * disp_info.vres);
      width = (int) (HOR_INFO * disp_info.hres);
      x1 = (int) (disp_info.hres / ALGN_CNTR);
      y1 = (int) (disp_info.vres / ALGN_CNTR);
      x2 = (int) (disp_info.hres / ALGN_CNTR) + width;
      y2 = (int) (disp_info.vres / ALGN_CNTR) + height;
      grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);


      /*  Initialization of the text positioning variables  */

      grSetTextStyle (grTXT8X14, grTRANS);
      margin = (int) (x2 - x1) * M_RATIO;
      pixline = (int) ((y2 - y1) + 1) / 6.5;
      yAdj = (int) (pixline - HIRES) / 2;
      xAdj = (int) (x2 - (x1 + margin)) / 2;


      /*  Display the screen header  */

      yPos = y1 + yAdj + 4;
      xPos = (int) (width - 80) / 2 + x1;
      grSetColor (grRED);
      grMoveTo (xPos, yPos);
      grOutText ("STATISTICS");


      /*  Display the region label and region size  */

      yPos += (int) (1.5 * pixline);
      xPos = x1 + margin;
      grSetColor (grDARKGRAY);
      grMoveTo (xPos, yPos);
      grOutText ("Region");
      xSize = (leftRegion.lower_right.x - leftRegion.upper_left.x) + 1;
      ySize = (leftRegion.lower_right.y - leftRegion.upper_left.y) + 1;
      numPix = (long) xSize * ySize;
      itoa (xSize, regStr, BASE);
      _fstrcat (regStr, " x ");
      itoa (ySize, satStr, BASE);
      _fstrcat (regStr, satStr);
      grSetColor (grBLUE);
      grMoveTo (xPos + xAdj, yPos);
      grOutText (regStr);


      /*  Display the time label and the elapsed time  */

      yPos += pixline;
      xPos = x1 + margin;
      grSetColor (grDARKGRAY);
      grMoveTo (xPos, yPos);
      grOutText ("Time");
      mins = (int) seconds / SECMIN;
      secs = (int) seconds % SECMIN;
      itoa (mins, tempStr, BASE);
      if (mins < 10)
         {
            _fstrcpy (timStr, "0");
            _fstrcat (timStr, tempStr);
         }

      else
         _fstrcpy (timStr, tempStr);

      strcat (timStr, ":");
      itoa (secs, tempStr, BASE);
      if (secs < 10)
         {
            strcat (timStr, "0");
            strcat (timStr, tempStr);
         }

      else
         strcat (timStr, tempStr);

      grSetColor (grBLUE);
      grMoveTo (xPos + xAdj, yPos);
      grOutText (timStr);


      /*  Display the throughput label and the performance value  */

      yPos += pixline;
      xPos = x1 + margin;
      grSetColor (grDARKGRAY);
      grMoveTo (xPos, yPos);
      grOutText ("Throughput");
      thruPut = (long) numPix / seconds;
      grSetColor (grBLUE);
      grMoveTo (xPos + xAdj, yPos);
      itoa (thruPut, tempStr, BASE);
      _fstrcat (tempStr, " Pix/Sec");
      grOutText (tempStr);


      /*  Present the user with an "OK" button  */

      by1 = yPos + (int) (1.5 * pixline);
      bSize = BUT_WIDTH * width;
      bx1 = (int) ((width - bSize) / 2) + x1;
      bx2 = bx1 + bSize;
      GetButton (bx1, by1, bx2, by1 + pixline);


      /*  House cleaning  */

      farfree (regStr);
      farfree (timStr);
      farfree (tempStr);
      return;
   }





/*----------------------------------------------------------------------------

      This procedure displays an information screen containing performance

----------------------------------------------------------------------------*/

void Alert (msgStr, selectColor)
char     far *msgStr;
int      selectColor;
   {
      int      height, width, x1, y1, x2, y2, bSize, xPos, yPos, bx1, bx2, by1, size;


      /*  Draw the information box  */

      height = (int) (VERT_INFO * disp_info.vres);
      width = (int) (HOR_INFO * disp_info.hres);
      x1 = (int) (disp_info.hres / ALGN_CNTR);
      y1 = (int) (disp_info.vres / ALGN_CNTR);
      x2 = (int) (disp_info.hres / ALGN_CNTR) + width;
      y2 = (int) (disp_info.vres / ALGN_CNTR) + height;
      grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);


      /*  Initialization of the text positioning variables  */

      grSetTextStyle (grTXT8X14, grTRANS);
      size = _fstrlen (msgStr);


      /*  Display the screen header  */

      xPos = (int) x1 + ((width - (17 * CHAR_WIDTH)) / 2);
      yPos = (int) y1 + (height / 6);
      grSetColor (grRED);
      grMoveTo (xPos, yPos);
      grOutText ("*** A L E R T ***");


      /*  Display the Alert box message  */

      yPos += (int) height * VERT_TITLE;
      xPos = (int) x1 + ((x2 - x1) - (size * CHAR_WIDTH)) / 2;
      grSetColor (grWHITE);
      grMoveTo (xPos, yPos);
      grOutText (msgStr);



      /*  Present the user with an "OK" button  */

      by1 = (int) (yPos + (height * VERT_TITLE));
      bSize = BUT_WIDTH * width;
      bx1 = (int) ((width - bSize) / 2) + x1;
      bx2 = bx1 + bSize;
      GetButton (bx1, by1, bx2, by1 + (int) (height / 6));


      /*  House Cleaning  */

      grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);

      return;
   }





/*----------------------------------------------------------------------------

      This procedure displays or erases the opening title and screen
   depending on the value of the single supplied argument.

----------------------------------------------------------------------------*/

void ShowTitle (onoff)
int      onoff;
   {
      int      height, width, x1, y1, x2, y2, xSize, ySize;
      int      xPos, yPos, yAdj, margin, pixline;


      /*  Determine size and position coordinates  */

      height = (int) (VERT_TITLE * disp_info.vres);
      width = (int) (HOR_TITLE * disp_info.hres);
      x1 = (int) (disp_info.hres / ALGN_CNTR);
      y1 = (int) (disp_info.vres / ALGN_CNTR);
      x2 = (int) (disp_info.hres / ALGN_CNTR) + width;
      y2 = (int) (disp_info.vres / ALGN_CNTR) + height;
      if (onoff == ON)
         {
            /*  Draw the box */

            grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
            grDrawRect (x1, y1, x2, y2, grFILL);
            grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
            grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
            grSetFillStyle (grFSOLID, grGRAY, grOPAQUE);
            grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);


            /*  Initialize string placement variables  */

            grSetTextStyle (grTXT8X14, grTRANS);
            margin = (int) (x2 - x1) * M_RATIO;
            pixline = (int) ((y2 - y1) + 1) / 6.5;
            yAdj = (int) (pixline - HIRES) / 2;


            /*  Display the title  */

            yPos = y1 + yAdj + 4;
            xPos = x1 + margin;
            grSetColor (grDARKGRAY);
            grMoveTo (xPos, yPos);
            grOutText ("Fast Image Rendering for Ray Tracing");
            yPos += (int) (0.67 * pixline);
            xPos = x1 + margin;
            grMoveTo (xPos, yPos);
            grOutText ("Algebraic Surfaces Using A Fibonacci");
            yPos += (int) (0.67 * pixline);
            xPos = x1 + margin;
            grMoveTo (xPos, yPos);
            grOutText ("Based Psuedo-Random Number Generator");


            /*  Display the author and date  */

            grSetColor (grWHITE);
            yPos += (int) (1.67 * pixline);
            xPos = x1 + margin;
            grMoveTo (xPos, yPos);
            grOutText ("        By Randall W. Charlick");
            yPos += (int) (0.67 * pixline);
            xPos = x1 + margin;
            grMoveTo (xPos, yPos);
            grOutText ("             August 1992");


            /*  Display the current vide mode  */

            grSetColor (grBLUE);
            yPos += (int) (1.67 * pixline);
            xPos = x1 + margin;
            grMoveTo (xPos, yPos);
            grOutText ("     Video: ");
            xPos += 35;
            grOutText (disp_info.descrip);
         }

      else
         {
            /*  Erase the title screen  */

            grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
            grDrawRect (x1, y1, x2, y2, grFILL);
         }

      return;
   }




/*----------------------------------------------------------------------------

      This procedure Draws a button on the information screen according
   to the four supplied screen coordinates and monitors the mouse activity
   for the selection of the defined button region.  Upon the selection of
   the above mentioned button, the information screen is cleared.

---------------------------------------------------------------------------*/

void GetButton (x1, y1, x2, y2)
int      x1, y1, x2, y2;
   {
      int      mX, mY, xPos, yPos, found, pressed = FALSE;


      /*  Draw the button  */

      grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);
      grSetTextStyle (grTXT8X14, grTRANS);
      yPos = y1 + 2;
      xPos = (int) ((x2 - x1) / 2) + (x1 - 8);
      grSetColor (grRED);
      grMoveTo (xPos, yPos);
      grOutText ("OK");


      /*  Wait for the button to be pressed  */

      found = FALSE;
      grDisplayMouse (grSHOW);
      while (!found)
         {
            pressed = grGetMouseButtons ();


            /*  Detect a mouse click  */

            if (pressed & grLBUTTON)
               {
                  /*  Determine whether the click occured in the button 
                      region                                               */

                  grGetMousePos (&mX, &mY);
                  if (mX >= x1 && mX <= x2 && mY >= y1 && mY <= y2)
                     {
                        /*  OK to quit, push the button  */

                        found = TRUE;
                        grDisplayMouse (grHIDE);
                        grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                        grDrawRect (x1, y1, x2, y2, grFILL);
                        grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                        grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
                        grSetFillStyle (grFSOLID, grGRAY, grOPAQUE);
                        grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);
                        grMoveTo (xPos + 1, yPos + 1);
                        grOutText ("OK");
                        grDisplayMouse (grSHOW);
                     }

                  pressed = FALSE;
                  while (grGetMouseButtons () != 0);
               }

         }

      grDisplayMouse (grHIDE);
      return;
   }
