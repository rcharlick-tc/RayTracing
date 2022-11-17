/*---------------------------------------------------------------------------

File:                   Surface.c

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           May 9, 1992
Last Modified:          September 28, 1992

Description:            This file contains the heart of the Ray Tracing
                        algorithm.  All of the surface and intersection
                        calculation modules as well as the illumination
                        function are contained in this file.  Additionally,
                        this file contains the pixel selection routine and
                        the main image rendering driver, which directly or
                        indirectly calls all of the above routines.  Finally,
                        several root finding algorithms were implemented
                        throughout the course of this project.  Initially
                        a bracketing and bisection method was used, followed
                        by a Laguerre based root finder.  The best overall
                        method for efficiency and accuarcy was an implemention
                        supplied from Jochen Schwarze.


---------------------------------------------------------------------------*/


/*  Graphics and other supplied include files  */

#include <stdio.h>
#include <string.h>
#include <gxlib.h>
#include <grlib.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <io.h>


/*  Project include files  */

#include "surface.h"
#include "select.h"
#include "globals.h"
#include "display.h"
#include "errmsg.h"


/*  Global Variables  */

extern           GXDINFO        disp_info;
extern           REGION_TYPE    rightRegion, leftRegion;
extern           COORD_TYPE     planeRotation, centerRotation;
extern           int            checkRoot, imageNumber;
int              scanOne, scanTwo, fibOne, fibTwo, fibThree, leftVert,
                 rightVert, leftStart = TRUE, rightStart = TRUE;


/*  External function prototypes  */

extern int  UpdateStatus (int, int far *, int far *, time_t, long, long);
extern int  Fibonacci (int, int);
extern int  Gcd (int, int);
extern void FatPixel (int, int, int, int, int);
extern void FatScan (int, int, int far *, int, int);
extern int  SelectPixel (int far *, int far *, int far *, int far *, int, long far *, REGION_TYPE, int, int far *);
extern void ErrMsg (int);


/*  Internal function prototypes  */

void FreeList (POINT_LIST far *);
void AssignMatrix (double RDIM, int);
void MatrixMultiply (double RDIM, double RDIM);
void VectorMultiply (RAY_TYPE far *, double RDIM);
void Rotate (RAY_TYPE far *);
int  Driver (IMAGE_TYPE far *, IMAGE_TYPE far *, int, int, int, int, int, COORD_TYPE, short);
int  Trace (IMAGE_TYPE far *, RAY_TYPE, COORD_TYPE, int, int, short);
int  Intersect (IMAGE_TYPE far *, RAY_TYPE, POINT_LIST far *);
int  Illuminate (COORD_TYPE, COORD_TYPE, POINT_LIST far *, int, short);
int  Apply (double [MAX_DEGREE], int, POINT_LIST far *, IMAGE_TYPE far *, RAY_TYPE, int);
int  Quadric (double [MAX_DEGREE], double [MAX_DEGREE]);
int  Cubic (double [MAX_DEGREE], double [MAX_DEGREE]);
int  Quartic (double [MAX_DEGREE], double [MAX_DEGREE]);
double      Evaluate (double, int, double [MAX_DEGREE]);
COORD_TYPE  GetNormal (POINT_LIST far *);
POINT_LIST  far * FindImage (POINT_LIST far *);



/*----------------------------------------------------------------------------

      This function is the main driver of the rendering routine.  It is
   responsible for dual or mono screen management of pixel selection and
   intersection calculation.  In addition this module must manage viewport
   and clipping regions.  The functions argument list contains an image list
   for each screen region, the binary encoded method, the screen mode and
   the boolean status bar indicator.  In addition, lighting information
   such as the coordinates of the light source, light flooding effects,
   background color and the saturation angle of the images.  Finally, the
   function returns a success or error code.

----------------------------------------------------------------------------*/

int Driver (leftList, rightList, method, drawMode, lighting, backGround, status, candle, overSpill)
IMAGE_TYPE     far *leftList, far *rightList;
int            method, drawMode, lighting, backGround, status;
COORD_TYPE     candle;
short          overSpill;
   {
      RAY_TYPE       ray;
      time_t         startTime;
      long           leftPix = NONE, rightPix = NONE, totPix = NONE, leftIter = NONE, rightIter = NONE;
      int            leftWide, rightWide, count, done, leftDone, rightDone, active;
      int            intensity, leftX, leftY, rightX, rightY;
      int            leftMethod, rightMethod, leftCount = NONE, rightCount = NONE;
      int            leftContinue = FALSE, rightContinue = FALSE, inValid = FALSE, cancelled = FALSE;
      int            leftComp = UPD_START, leftTime = NONE, rightComp = UPD_START, rightTime = NONE;
      char           keyBrk = ' ';
      double         aspect;
      unsigned long  mem;



      /*  Initialization of iteration control variables and region coordinates  */

      done = leftDone = rightDone = FALSE;
      if (drawMode == MONO)
         rightDone = TRUE;

      leftX = rightX = X_ORIGIN;
      leftY = rightY = Y_ORIGIN;
      active = LEFT;


      /*  Set the centroid for image plane rotation if centroid is not from an image  */

      if (imageNumber == CENTER_SPACE)
         {
            centerRotation.xPos = (int) (((leftRegion.lower_right.x - leftRegion.upper_left.x) / 2) + 0.5);
            centerRotation.yPos = (int) (((leftRegion.lower_right.y - leftRegion.upper_left.y) / 2) + 0.5);
            centerRotation.zPos = CENTER_Z;
         }

      else if (imageNumber == CENTER_PLANE)
              {
                 centerRotation.xPos = (int) (((leftRegion.lower_right.x - leftRegion.upper_left.x) / 2) + 0.5);
                 centerRotation.yPos = (int) (((leftRegion.lower_right.y - leftRegion.upper_left.y) / 2) + 0.5);
                 centerRotation.zPos = 0;
              }

      aspect = (double) (leftRegion.lower_right.x - leftRegion.upper_left.x) /
                        (leftRegion.lower_right.y - leftRegion.upper_left.y);
      if (aspect > 1.0)
         aspect = (double) (1.0 / aspect);


      /*  Decode method and orientation for each region  */

      leftVert = method & OL_MASK;
      rightVert = method & OR_MASK;
      leftMethod = (method >> METHOD_CODE) & R_MASK;
      rightMethod = method & R_MASK;


      /*  Calculate optimum fibonacci constants for uniform scan line
          selection                                                       */

      if (leftMethod == FIBSCAN || rightMethod == FIBSCAN || leftMethod == FATFIBSCAN ||
          rightMethod == FATFIBSCAN)
         {
            count = FIBSTART;
            scanOne = Fibonacci (count, FIB_STD);


            /*  Find the first fibonacci type number greater than the minimum
                of the two region dimensions                               */

            while (scanOne <= (leftRegion.lower_right.y - leftRegion.upper_left.y) ||
                   scanOne <= (leftRegion.lower_right.x - leftRegion.upper_left.x))
               scanOne = Fibonacci (count++, FIB_STD);

            scanTwo = Fibonacci (count, FIB_STD);
            if (leftMethod == FATFIBSCAN)
               leftWide = scanTwo;

            if (rightMethod == FATFIBSCAN)
               rightWide = scanTwo;

         }



      /*  Calculate optimum fibonacci constants for uniform pixel methods  */

      if (leftMethod == FIBONACCI || rightMethod == FIBONACCI)
         {
            count = FIBSTART;
            fibOne = Fibonacci (count, FIB_MOD);


            /*  Find the first fibonacci type number greater than the minimum
                of the two region dimensions                               */

            while (fibOne <= (leftRegion.lower_right.y - leftRegion.upper_left.y) ||
                   fibOne <= (leftRegion.lower_right.x - leftRegion.upper_left.x))
               fibOne = Fibonacci (count++, FIB_MOD);

            fibTwo = Fibonacci (count, FIB_MOD);
            fibThree = Fibonacci (count - 3, FIB_MOD);


            /*  Verify that the greatest common denominator of the acquired
                fibonacci numbers is one                                   */

            while (Gcd (fibOne, fibThree) > 1 && Gcd (fibTwo, fibThree) > 1)
               {
                  fibOne = Fibonacci (count++, FIB_MOD);
                  fibTwo = Fibonacci (count, FIB_MOD);
                  fibThree = Fibonacci (count - 3, FIB_MOD);
               }

         }


      /*  Set constants for fat uniform pixel selection methods  */

      if (leftMethod == FATFIB || rightMethod == FATFIB)
         {
            fibOne = LG_FIB;
            fibTwo = MID_FIB;
            fibThree = SM_FIB;
            if (leftMethod == FATFIB)
               leftWide = FATSIZE;

            if (rightMethod == FATFIB)
               rightWide = FATSIZE;

         }


      /*  Initialize performance variables  */

      totPix = (long) ((leftRegion.lower_right.x - leftRegion.upper_left.x) + 1) *
                      ((leftRegion.lower_right.y - leftRegion.upper_left.y) + 1);
      startTime = time (NULL);


      /*  Loop until all pixels are rendered in both regions  */

      while (!done && !inValid && !cancelled)
         {
            if (active == LEFT && !leftDone)
               {
                  /*  Set up viewport and clipping for the left region  */

                  grSetViewPort (leftRegion.upper_left.x, leftRegion.upper_left.y,
                                 leftRegion.lower_right.x, leftRegion.lower_right.y);
                  grSetBkColor (grBLACK);
                  grSetClipRegion (leftRegion.upper_left.x, leftRegion.upper_left.y,
                                   leftRegion.lower_right.x, leftRegion.lower_right.y);
                  grSetClipping (grCLIP);


                  /*  Select a pixel according to the region method  */

                  leftDone = SelectPixel (&leftX, &leftY, &leftCount, &leftContinue,
                                          leftMethod, &leftIter, leftRegion, LEFT, &leftWide);
                  ++leftPix;
                  ray.refx = leftX;
                  ray.refy = leftY;
                  ray.refz = 0;


                  /*  Trace the ray into object space  */

                  intensity = Trace (leftList, ray, candle, lighting, backGround, overSpill);


                  /*  Plot the selected pixel from the calculated intensity  */

                  if (intensity < VALID)
                     return (intensity - m_BAD_INTENS);

                  else
                     if (leftMethod == FATFIB)
                        FatPixel (leftX, leftY, leftWide, intensity, active);

                     else if (leftMethod == FATFIBSCAN)
                             FatScan (leftX, leftY, &leftWide, leftVert, intensity);

                          else
                             grPutPixel (leftX, leftY, intensity);


                  /*  Conditionally update the status bar time and completion
                      percentage                                            */

                  if (((leftPix % INTERVAL) == 0 || leftPix == totPix) && status)
                     inValid = UpdateStatus (LEFT, &leftComp, &leftTime, startTime, leftPix, totPix);

               }

            else
               {
                  /*  Set up viewport and clipping for the right region  */

                  grSetViewPort (rightRegion.upper_left.x, rightRegion.upper_left.y,
                                 rightRegion.lower_right.x, rightRegion.lower_right.y);
                  grSetBkColor (grBLACK);
                  grSetClipRegion (rightRegion.upper_left.x, rightRegion.upper_left.y,
                                   rightRegion.lower_right.x, rightRegion.lower_right.y);
                  grSetClipping (grCLIP);


                  /*  Select a pixel according to the region method  */

                  rightDone = SelectPixel (&rightX, &rightY, &rightCount, &rightContinue,
                                           rightMethod, &rightIter, rightRegion, RIGHT, &rightWide);
                  ++rightPix;
                  ray.refx = rightX;
                  ray.refy = rightY;
                  ray.refz = 0;


                  /*  Trace the ray into object space  */

                  intensity = Trace (rightList, ray, candle, lighting, backGround, overSpill);


                  /*  Plot the selected pixel from the calculated intensity  */

                  if (intensity < VALID)
                     return (intensity - m_BAD_INTENS);

                  else
                     if (rightMethod == FATFIB)
                        FatPixel (rightX, rightY, rightWide, intensity, active);

                     else if (rightMethod == FATFIBSCAN)
                             FatScan (rightX, rightY, &rightWide, rightVert, intensity);

                          else
                             grPutPixel (rightX, rightY, intensity);



                  /*  Conditionally update the status bar time and completion
                      percentage                                            */

                  if (((rightPix % INTERVAL) == 0 || rightPix == totPix) && status)
                     inValid = UpdateStatus (RIGHT, &rightComp, &rightTime, startTime, rightPix, totPix);

               }


            /*  Alternate bewteen left and right regions  */

            if (drawMode == DUAL)
               if (active == LEFT)
                  active = RIGHT;

               else
                  active = LEFT;


            /*  Check for rendering completion  */

            if (leftDone && rightDone)
               done = TRUE;


            /*  Look for a key press  */

            if (kbhit ())
               keyBrk = getch ();


            /*  See if user would like to quit  */

            if (keyBrk == 'Q' || keyBrk == 'q')
               cancelled = TRUE;


            /*  Check for minimum available memory  */

            mem = farcoreleft ();
            if (mem < MIN_AVAIL)
               return (m_ERR_AVAIL);

         }


      /*  Restore the global viewport and remove clipping region  */

      grSetViewPort (X_ORIGIN, Y_ORIGIN, disp_info.hres, disp_info.vres);
      grSetClipping (grNOCLIP);
      if (!done && !inValid)
         inValid = m_STOP;

      return (inValid);
   }



/*----------------------------------------------------------------------------

      This function traces a ray through object space and determines which
   of any objects that ray intersects.  If the ray intersects more than
   one object, the nearest surface point is determined.  The coordinate
   that is determined to be the nearest intersection is illuminated if
   no other object klies between it and the light source.  The function
   takes as arguments, the list of images, the ray vector, the coordinates
   of the light source, the lighting saturation angle, the background
   region color and a light flooding boolean variable.  The function
   returns a success or error code.

----------------------------------------------------------------------------*/

int Trace (imageList, ray, candle, lighting, backGround, overSpill)
IMAGE_TYPE            far *imageList;
RAY_TYPE              ray;
COORD_TYPE            candle;
int                   lighting, backGround;
short                 overSpill;
   {
      IMAGE_TYPE      far *image;
      POINT_LIST      far *hitImage, far *hitList, far *newList, far *startList, far *lastList;
      COORD_TYPE      surfacePoint, normal;
      int             brightness, hit = FALSE;


      /*  Allocation and verification of linked list  */

      hitList = (POINT_LIST far *) farcalloc (SINGLE, sizeof (POINT_LIST));
      if (hitList == NULL)
         return (m_ERR_ALLOC);


      /*  See if any of the images are intersected with the ray  */

      startList = hitList;
      image = imageList;
      while (image != NULL)
         {
            if (Intersect (image, ray, hitList))
               {
                  /*  This image intersects the ray  */

                  lastList = hitList;


                  /*  Allocate and verify a hitlist node  */

                  newList = (POINT_LIST far *) farcalloc (SINGLE, sizeof (POINT_LIST));
                  if (newList == NULL)
                     return (m_ERR_ALLOC);


                  /*  Add this node to the list  */

                  hitList->next = newList;
                  hitList = newList;
                  hit = TRUE;
               }

            image = image->next;
         }

      if (!hit)
         {
            /*  No intersection with the ray, do some clean up  */

            farfree (hitList);
            return (backGround);
         }

      else
         {
            /*  Go through the list of intersections and find the closest  */

            lastList->next = NULL;
            hitImage = FindImage (startList);


            /*  Calculate the normal to the surface point and illuminate
                the pixel                                                   */

            normal = GetNormal (hitImage);
            brightness = Illuminate (candle, normal, hitImage, lighting, overSpill);


            /*  House cleaning  */

            farfree (hitList);
            FreeList (startList);
            return (brightness);
         }

   }




/*----------------------------------------------------------------------------

      This function evaluates the algebraic function for the intersection
   of the surface and a given ray.  The parameters are the value of the
   single variable in the substituted polynomial, the degree of the equation 
   and an array of coefficients for the polynomial function.  The function
   returns the evaluation of the single variable of the polynomial equation.


----------------------------------------------------------------------------*/

double Evaluate (val, degree, a)
double     val;
int        degree;
double   a [MAX_DEGREE];
   {
      double      tmp, sum = 0.0;
      int         j, k;


      tmp = val;
      for (k = degree; k >= 0; k--)
         {
            /*  Calculate the powers of the univariate  */

            if (k == 0)
               tmp = 1.0;

            else if (k == 1)
                    tmp = val;

                 else
                    for (j = 1; j < k; ++j)
                       tmp *= val;


            /*  Apply the coeffient to the term  */

            tmp *= a [k];


            /*  Add each term of the equation  */

            sum += tmp;
            tmp = val;
         }

      return (sum);
   }



/*----------------------------------------------------------------------------

      This function calculates the normal the normal to the point on the
   image surface which was previously determined to intersect the ray.
   This normal is returned as a coordinate and is used to calculate pixel
   intensity for the image intersection.

----------------------------------------------------------------------------*/

COORD_TYPE GetNormal (IntObj)
POINT_LIST far *IntObj;
   {
      COORD_TYPE   tnorm;
      double       dx, dy, dz, da, db;


      /*  Simplify the normal equation  */

      dx = IntObj->local.xPos - IntObj->imagePtr->center.xPos;
      dy = IntObj->local.yPos - IntObj->imagePtr->center.yPos;
      dz = IntObj->local.zPos - IntObj->imagePtr->center.zPos;
      da = IntObj->imagePtr->size;
      db = IntObj->imagePtr->size2;


      /*  Calculate the normal for the given surface equation  */

      if (!_fstrcmp (IntObj->imagePtr->surface, "SPHERE"))
         {
            tnorm.xPos = 2 * dx + IntObj->local.xPos;
            tnorm.yPos = 2 * dy + IntObj->local.yPos;
            tnorm.zPos = 2 * dz + IntObj->local.zPos;
         }

      else if (!_fstrcmp (IntObj->imagePtr->surface, "GENERIC"))
              {
                 tnorm.xPos = 2 * (dx * dy + dx);
                 tnorm.yPos = 2 * (dy * dy - dy) + dx * dx - 2;
                 tnorm.zPos = 2 * dz;
              }

           else if (!_fstrcmp (IntObj->imagePtr->surface, "TORUS"))
                   {
                      tnorm.xPos  = (4 * dx * dx * dx + 4 * dy * dy * dx + 4 * dz * dz * dx -
                                     2 * da * da * dx - 2 * db * db * dx) + IntObj->local.xPos;
                      tnorm.yPos  = (4 * dy * dy * dy + 4 * dx * dx * dy + 4 * dz * dz * dy -
                                     2 * da * da * dy - 2 * db * db * dy) + IntObj->local.yPos;
                      tnorm.zPos  = (4 * dz * dz * dz + 4 * dx * dx * dz + 4 * dy * dy * dz -
                                     10 * da * da * dz - 2 * db * db * dz) + IntObj->local.zPos;
                    }



      return (tnorm);
   }



/*----------------------------------------------------------------------------

      This function determines the intersection if an image and a ray.  The
   ray is cast from an anchor point in a specified direction.  The equation
   of the ray is substituted into the surface equation (manually) and 
   the coefficients of the polynomial for a single real variable t are
   calculated.  If the degree of the original polynomial is 2 then the
   quadratic equation is used to find the roots otherwise the
   method is used.  If an intersection is detected a node containing the
   surface coordinates of the intersection is assigned (for subsequent
   addition to the linked list of ray intersection from the calling
   routine) and a value of TRUE is returned.  If no intersection is found
   then a FALSE value is returned.

----------------------------------------------------------------------------*/

int Intersect (image, ray, hitNode)
IMAGE_TYPE     far *image;
RAY_TYPE       ray;
POINT_LIST     far *hitNode;
   {
      RAY_TYPE    tRay;
      int         contact = FALSE, degree, zCount = 0, clipped = FALSE, i;
      double      a [MAX_DEGREE], z [MAX_DEGREE];
      double      h, r, r2, R, R2, x0, y0, z0, x1, y1, z1;
      double      g14, g13, g12, g11, g10, g09, g08, g07;
      double      g06, g05, g04, g03, g02, g01;



      /*  Initialization of algebraic substitution variables  */

      tRay.refx = ray.refx - image->center.xPos;
      tRay.refy = ray.refy - image->center.yPos;
      tRay.refz = ray.refz - image->center.zPos;


      /*  Rotate the image plane as necessary to determine valid coordinates
          for the origin of the ray  */

      if ((planeRotation.xPos > NONE && planeRotation.xPos < FULL_ROTATION) || (planeRotation.yPos > NONE &&
           planeRotation.yPos < FULL_ROTATION) || (planeRotation.zPos > NONE && planeRotation.zPos < FULL_ROTATION))
         Rotate (&tRay);

      x0 = tRay.refx;
      y0 = tRay.refy;
      z0 = tRay.refz;


      /*  Parallel Projection  */


      tRay.refx = 0;
      tRay.refy = 0;
      tRay.refz = leftRegion.lower_right.y - leftRegion.upper_left.y;

      /*  Rotate the image plane as necessary to determine valid coordinates
          for the directional vector ray  */

      if ((planeRotation.xPos > NONE && planeRotation.xPos < FULL_ROTATION) || (planeRotation.yPos > NONE &&
           planeRotation.yPos < FULL_ROTATION) || (planeRotation.zPos > NONE && planeRotation.zPos < FULL_ROTATION))
         Rotate (&tRay);

      x1 = tRay.refx;
      y1 = tRay.refy;
      z1 = tRay.refz;


      /*  Calculate the coefficients of the substitution of the ray
          equation to the surface equation                                 */

      if (!_fstrcmp(image->surface,"SPHERE"))
         {
            /*  Assign required variables  */

            degree = 2;
            r  = (double) image->size;


            /*  Univariate polynomial equation for a sphere  */

            a [2] = x1 * x1 + y1 * y1 + z1 * z1;
            a [1] = 2 * (x0 * x1 + y0 * y1 + z0 * z1);
            a [0] = x0 * x0 + y0 * y0 + z0 * z0 - r * r;
         }

      else if (!_fstrcmp(image->surface,"GENERIC"))
              {
                 /*  Initialization for optimization  */

                 degree = 3;
                 g01 = x1 * x1;
                 g02 = y0 * y0;
                 g03 = y0 * y1;
                 g04 = x0 * x0;
                 g05 = x0 * x1;
                 g06 = y1 * y1;


                 /*  Univariate polynomial equation for an undefined surface  */

                 a [3] = g01 * y1 + g12 * y1 + 1;
                 a [2] = 2 * (g05 * y1) + 3 * (g03 * y1) - g06 +
                         g01 * y0 + g01 - z1 * z1 + 1;
                 a [1] = g04 * y1 + 3 * (g02 * y1) - 2 * g03 +
                         2 * (g05 * y0) + 2 * g05 - 2 * y1 - 2 * (z0 * z1) +1;
                 a [0] = g02 * y0 - g02 + g04 - 2 * y0 - z0 * z0 + 1;
              }

           else if (!_fstrcmp(image->surface,"TORUS"))
                   {
                      /*  Initialization  */

                      degree = 4;
                      r  = (double) image->size;
                      R  = (double) image->size2;


                      /*  Assignment for optimization  */

                      r2 = r * r;
                      R2 = R * R;
                      g01 = x0 * x0;
                      g02 = y0 * y0;
                      g03 = z0 * z0;
                      g04 = x1 * x1;
                      g05 = y1 * y1;
                      g06 = z1 * z1;
                      g07 = x0 * x1;
                      g08 = y0 * y1;
                      g09 = z0 * z1;
                      g10 = R2 + r2;
                      g11 = g01 + g02 + g03;
                      g12 = g04 + g05 + g06;
                      g13 = g07 + g08 + g09;
                      g14 = g11 - g10;


                      /*  Univariate polynomial for a torus  */

                      a [4] = g12 * g12;
                      a [3] = 4 * g13 * g12;
                      a [2] = 2 * g12 * g14 + 4 * g13 * g13 + 4 * R2 * g06;
                      a [1] = 4 * g13 * g14 + 8 * R2 * g09;
                      a [0] = g14 * g14 - 4 * R2 * (r2 - g03);
                   }



      /*  Find the roots of the above coefficients  */

      for (i = 0; i < degree; ++i)
         if (a [i] == 0.0)
            ++zCount;

      if ((zCount + 1) >= degree || clipped)
         contact = FALSE;

      contact = Apply (a, degree, hitNode, image, ray, z1);

      return (contact);
   }




/*----------------------------------------------------------------------------

      This function Determines the nearest intersection of the ray with
   the list of images.  All appropriate data structures are modified in the
   case of a hit.  The function returns a boolean corresponding to the
   intersection of the given ray and an image.

----------------------------------------------------------------------------*/

int Apply (a, degree, hitNode, image, ray, z1)
double       a [MAX_DEGREE];
int          degree;
POINT_LIST   far *hitNode;
IMAGE_TYPE   far *image;
RAY_TYPE     ray;
int          z1;
   {
      int      num, nearest = 0, i;
      double   roots [MAX_DEGREE], stdErr;


      /*  Determine the number of roots and the coefficients for an algebraic
          surface                                                             */

      if (degree == 2 )
         num = Quadric (a, roots);

      else if (degree == 3)
              num = Cubic (a, roots);

           else if (degree == 4)
                   num = Quartic (a, roots);


      /*  Find the closest intersection  */

      for (i = 0; i < num; i++)
         if (roots [i] < roots [nearest] && roots [nearest] > 0.0)
            nearest = i;


      /*  Modify the hitlist and determine the z coordinate position  */

      if (roots [nearest] > 0.0 && num > 0)
         {
            hitNode->local.xPos = ray.refx;
            hitNode->local.yPos = ray.refy;
            hitNode->local.zPos = (int) ((z1 * roots [nearest]) + 0.5);
            hitNode->imagePtr = image;
            hitNode->next = NULL;
            if (checkRoot)
               {
                  stdErr = Evaluate (roots [nearest], degree, a);
                  if (stdErr > THRESH)
                     ErrMsg (m_BAD_ROOT);

               }

            return (TRUE);
         }

      else
         return (FALSE);

   }




/*----------------------------------------------------------------------------

      This function is used to solve for the roots of polynomial equations
   of degree two.

----------------------------------------------------------------------------*/

int Quadric (a, roots)
double       a [MAX_DEGREE], roots [MAX_DEGREE];
   {
      double      discriminant, p, q, ds, num = 0;


      /*  Calculate the discriminant  */

      if (a [2] > 0)
         {
            p = a [1] / (2 * a [2]);
            q = a [0] / a [2];
            discriminant = p * p - q;
         }

      else
         {
            gxSetMode (gxTEXT);
            printf ("A run-time error occured in the Quadric root module.\n");
            printf ("Strike any key to continue.\n\n");
            getch ();
            exit (0);
         }

      if (discriminant > -QEPS && discriminant < QEPS)
         {
            /*  One single root  */

            roots [0] = -p;
            num = 1;
         }

      else if (discriminant > 0)
              {
                 /*  Two roots  */

                 ds = sqrt (discriminant);
                 roots [0] = ds - p;
                 roots [1] = -ds - p;
                 num = 2;
              }

      return (num);
   }



/*----------------------------------------------------------------------------

      This function is used to solve for the roots of polynomial equations
   of degree three.

----------------------------------------------------------------------------*/

int Cubic (a, roots)
double       a [MAX_DEGREE], roots [MAX_DEGREE];
   {
      int         num, i;
      double      sub, A, B, C, A2, p, q, p3, discriminant;
      double      u, phi, t, v, ds;


      /*  Normal form of cubic equation  */

      A = a [2] / a [3];
      B = a [1] / a [3];
      C = a [0] / a [3];


      /*  Calculation of discriminant using Cardano's Formula  */

      A2 = A * A;
      p = 1.0 / 3 * (-1.0 / 3 * A2 + B);
      p3 = p * p * p;
      q = 1.0 / 2 * (2.0 / 27 * A * A2 - 1.0 / 3 * A * B + C);
      discriminant = q * q + p3;

      if (discriminant > -QEPS2 && discriminant < QEPS2)
         if (q > -QEPS && q < QEPS)
            {
               roots [0] = 0.0;
               num = 1;
            }

         else
            {
               if (q < 0)
                  {
                     /*  Two roots  */

                     u = pow (-q, CUBE_ROOT);
                     roots [0] = 2 * u;
                     roots [1] = -u;
                     num = 2;
                  }

               else
                  num = 0;

            }

      else if (discriminant < 0)
              {
                 /*  Trigonometric substitution for casus irreducibilis  */

                 phi = 1.0 / 3 * acos (-q / sqrt (-p3));
                 t = 2 * sqrt (-p);
                 roots [0] =  t * cos (phi);
                 roots [1] = -t * cos (phi + PI / 3);
                 roots [2] = -t * cos (phi - PI / 3);
                 num = 3;
              }

           else
              {
                 /*  Single root  */

                 ds = sqrt (discriminant);
                 if (-q + ds > 0.0)
                    u = pow (-q + ds, CUBE_ROOT);
                 else
                    u = 0.0;

                 if (-q - ds > 0.0)
                    {
                       v = pow (-q - ds, CUBE_ROOT);
                       roots [0] = u + v;
                    }

                 else
                    {
                       v = pow (ds + q, CUBE_ROOT);
                       roots [0] = u - v;
                    }

                 num = 1;
              }


      /*  Resubstitution  */

      sub = 1.0 / 3 * A;
      for (i = 0; i < num; ++i)
         roots [i] -= sub;

      return (num);
   }



/*----------------------------------------------------------------------------

      This function is used to solve for the roots of polynomial equations
   of degree four.

----------------------------------------------------------------------------*/

int Quartic (a, roots)
double       a [MAX_DEGREE], roots [MAX_DEGREE];
   {
      int      num, i;
      double   aa, bb, cc, dd, c [MAX_DEGREE], u, v, z, a2, p, q, r, sub, stdErr;


      /*  Normal form of Quartic equation  */

      aa = a [3] / a [4];
      bb = a [2] / a [4];
      cc = a [1] / a [4];
      dd = a [0] / a [4];


      /*  Calculate new coefficients without cubic term  */

      a2 = aa * aa;
      p = - 3.0 / 8 * a2 + bb;
      q = 1.0 / 8 * a2 * aa - 1.0 / 2 * aa * bb + cc;
      r = -3.0 / 256 * a2 * a2 + 1.0 / 16 * a2 * bb - 1.0 / 4 * aa * cc + dd;

      if (r > -QEPS && r < QEPS)
         {
            c [0] = q;
            c [1] = p;
            c [2] = 0.0;
            c [3] = 1.0;
            num = Cubic (c, roots);
            roots [num++] = 0.0;
         }

      else
         {
            /*  Create two Qaudric eqautions  */

            c [0] = 1.0 / 2 * r * p - 1.0 / 8 * q * q;
            c [1] = -r;
            c [2] = -1.0 / 2 * p;
            c [3] = 1;

            Cubic (c, roots);
            z = roots [0];
            if (checkRoot)
               {
                  stdErr = Evaluate (z, 3, c);
                  if (stdErr > THRESH)
                     ErrMsg (m_BAD_ROOT);

               }

            u = z * z - r;
            v = 2 * z - p;

            /*  Verify domain for Quadric equations  */

            if (u > -QEPS && u < QEPS)
               u = 0.0;
            else if (u > 0.0)
                    u = sqrt (u);
                 else
                    return (0);

            if (v > -QEPS && v < QEPS)
               v = 0.0;
            else if (v > 0.0)
                    v = sqrt (v);
                 else
                    return (0);


            /*  Find the roots for each Quadric equation  */

            c [0] = z - u;
            c [1] = v;
            c [2] = 1;
            num = Quadric (c, roots);

            c [0] = z + u;
            c [1] = -v;
            c [2] = 1;
            num += Quadric (c, roots + num);
         }


      /*  Resubstitution  */

      sub = 1.0 / 4 * aa;
      for (i = 0; i < num; ++i)
         roots [i] -= sub;

      return (num);
   }




/*----------------------------------------------------------------------------

      This procedure assigns components to rotational and transformation 
   matrices.  The results are passed through in the matrix parameter and 
   are determined by the type specification also provided in the parameter
   list.  Image information is also passed to this procedure via the parameter
   list.

----------------------------------------------------------------------------*/

void AssignMatrix (tMat, type)
double         tMat RDIM;
int            type;
   {
      double      radian, theta [NUM_AXIS];


      /*  Assign static matrix components for rotational or tansformational matrices  */

      if (type < X_ROTATE)
         {
            tMat [0] [0] = 1.0;
            tMat [0] [1] = 0.0;
            tMat [0] [2] = 0.0;
            tMat [0] [3] = 0.0;
            tMat [1] [0] = 0.0;
            tMat [1] [1] = 1.0;
            tMat [1] [2] = 0.0;
            tMat [1] [3] = 0.0;
            tMat [2] [0] = 0.0;
            tMat [2] [1] = 0.0;
            tMat [2] [2] = 1.0;
            tMat [2] [3] = 0.0;
            tMat [3] [3] = 1.0;
         }

      else
         {
            radian = HEMISPHERE / PI;
            tMat [3] [0] = 0.0;
            tMat [3] [1] = 0.0;
            tMat [3] [2] = 0.0;
            tMat [3] [3] = 1.0;
            theta [X_SUBSTITUTE] = (FULL_ROTATION - planeRotation.xPos) / radian;
            theta [Y_SUBSTITUTE] = (FULL_ROTATION - planeRotation.yPos) / radian;
            theta [Z_SUBSTITUTE] = (FULL_ROTATION - planeRotation.zPos) / radian;

            theta [X_SUBSTITUTE] = planeRotation.xPos / radian;
            theta [Y_SUBSTITUTE] = planeRotation.yPos / radian;
            theta [Z_SUBSTITUTE] = planeRotation.zPos / radian;
         }


      /*  Assign rotational or transformational matrices  */

      switch (type)
         {
            /*  Transformational matrices for substitution of image centroid  */

            case X_SUBSTITUTE:
               tMat [3] [0] = 0.0;
               tMat [3] [1] = NEG * centerRotation.yPos;
               tMat [3] [2] = NEG * centerRotation.zPos;
               break;

            case Y_SUBSTITUTE:
               tMat [3] [0] = NEG * centerRotation.xPos;
               tMat [3] [1] = 0.0;
               tMat [3] [2] = NEG * centerRotation.zPos;
               break;

            case Z_SUBSTITUTE:
               tMat [3] [0] = NEG * centerRotation.xPos;
               tMat [3] [1] = NEG * centerRotation.yPos;
               tMat [3] [2] = 0.0;
               break;

            case MULT_SUBSTITUTE:
               tMat [3] [0] = NEG * centerRotation.xPos;
               tMat [3] [1] = NEG * centerRotation.yPos;
               tMat [3] [2] = NEG * centerRotation.zPos;
               break;


            /*  Transformational matrices to return object to original position  */

            case X_RESUB:
               tMat [3] [0] = 0.0;
               tMat [3] [1] = centerRotation.yPos;
               tMat [3] [2] = centerRotation.zPos;
               break;

            case Y_RESUB:
               tMat [3] [0] = centerRotation.xPos;
               tMat [3] [1] = 0.0;
               tMat [3] [2] = centerRotation.zPos;
               break;

            case Z_RESUB:
               tMat [3] [0] = centerRotation.xPos;
               tMat [3] [1] = centerRotation.yPos;
               tMat [3] [2] = 0.0;
               break;

            case MULT_RESUB:
               tMat [3] [0] = centerRotation.xPos;
               tMat [3] [1] = centerRotation.yPos;
               tMat [3] [2] = centerRotation.zPos;
               break;



            /*  Rotational matrices for single or multiple rotations.  */

            case X_ROTATE:
               tMat [0] [0] = 1.0;
               tMat [0] [1] = 0.0;
               tMat [0] [2] = 0.0;
               tMat [0] [3] = 0.0;
               tMat [1] [0] = 0.0;
               tMat [1] [1] = cos (theta [X_SUBSTITUTE]);
               tMat [1] [2] = sin (theta [X_SUBSTITUTE]);
               tMat [1] [3] = 0.0;
               tMat [2] [0] = 0.0;
               tMat [2] [1] = NEG * sin (theta [X_SUBSTITUTE]);
               tMat [2] [2] = cos (theta [X_SUBSTITUTE]);
               tMat [2] [3] = 0.0;
               break;

            case Y_ROTATE:
               tMat [0] [0] = cos (theta [Y_SUBSTITUTE]);
               tMat [0] [1] = 0.0;
               tMat [0] [2] = NEG * sin (theta [Y_SUBSTITUTE]);
               tMat [0] [3] = 0.0;
               tMat [1] [0] = 0.0;
               tMat [1] [1] = 1.0;
               tMat [1] [2] = 0.0;
               tMat [1] [3] = 0.0;
               tMat [2] [0] = sin (theta [Y_SUBSTITUTE]);
               tMat [2] [1] = 0.0;
               tMat [2] [2] = cos (theta [Y_SUBSTITUTE]);
               tMat [2] [3] = 0.0;
               break;

            case Z_ROTATE:
               tMat [0] [0] = cos (theta [Z_SUBSTITUTE]);
               tMat [0] [1] = sin (theta [Z_SUBSTITUTE]);
               tMat [0] [2] = 0.0;
               tMat [0] [3] = 0.0;
               tMat [1] [0] = NEG * sin (theta [Z_SUBSTITUTE]);
               tMat [1] [1] = cos (theta [Z_SUBSTITUTE]);
               tMat [1] [2] = 0.0;
               tMat [1] [3] = 0.0;
               tMat [2] [0] = 0.0;
               tMat [2] [1] = 0.0;
               tMat [2] [2] = 1.0;
               tMat [2] [3] = 0.0;
               break;

         }

      return;
   }




/*----------------------------------------------------------------------------

      This procedure performs matrix multiplication on two predetermined
   matrices and returns the results in the first matrix argument.

----------------------------------------------------------------------------*/

void MatrixMultiply (aMat, bMat)
double      aMat RDIM, bMat RDIM;
   {
      int         i, j, k;
      double      AnswerMat RDIM = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


      /*  Calculate the resulting matrix  */

      for (i = 0; i < RCOL; ++i)
         for (j = 0; j < RROW; ++j)
            for (k = 0; k < RCOL; ++k)
               AnswerMat [j] [i] += aMat [j] [k] * bMat [k] [i];


      /*  Copy the result to the first matrix  */

      for (i = 0; i < RCOL; ++i)
         for (j = 0; j < RROW; ++j)
            aMat [i] [j] = AnswerMat [i] [j];

      return;
   }



/*----------------------------------------------------------------------------

      This procedure multiplies a vector by a transformational matrix and
   returns the results by modifying the first vector argument.

----------------------------------------------------------------------------*/

void VectorMultiply (ray, rMat)
RAY_TYPE    far *ray;
double      rMat RDIM;
   {
      int         i, j, result [RCOL] = {0, 0, 0, 0}, vector [RCOL];
      RAY_TYPE    tRay;


      /*  Assign original coordinates to be transformed  */

      tRay = *ray;
      vector [0] = tRay.refx;
      vector [1] = tRay.refy;
      vector [2] = tRay.refz;
      vector [3] = 1;


      /*  Calculate the new vector  */

      for (i = 0; i < RROW; ++i)
         for (j = 0; j < RCOL; ++j)
            result [i] += vector [j] * rMat [i] [j];


      /*  Reassign the rotated coordinates  */

      tRay.refx = result [0];
      tRay.refy = result [1];
      tRay.refz = result [2];
      *ray = tRay;

      return;
   }



/*----------------------------------------------------------------------------

      This procedure rotates the image plane a specified number of degrees in
   any of the three dimension coordinate planes.  The object, containing image
   center and size, the casting ray and 
   are passed as arguments.  The ray value is modified according to
   the degree of rotation.

----------------------------------------------------------------------------*/

void Rotate (ray)
RAY_TYPE       far *ray;
   {
      double      ResultMatrix RDIM, TransMatrix RDIM, TempMatrix RDIM;
      int         i, j, resubstitute, previous = FALSE;


      /*  Assign local X, Y and Z axis transformation matrix  */

      if (planeRotation.xPos && !planeRotation.yPos && !planeRotation.zPos)
         {
            AssignMatrix (TransMatrix, X_SUBSTITUTE);
            resubstitute = X_RESUB;
         }

      else if (planeRotation.yPos && !planeRotation.xPos && !planeRotation.zPos)
              {
                 AssignMatrix (TransMatrix, Y_SUBSTITUTE);
                 resubstitute = Y_RESUB;
              }

           else if (planeRotation.zPos && !planeRotation.xPos && !planeRotation.yPos)
                   {
                      AssignMatrix (TransMatrix, Z_SUBSTITUTE);
                      resubstitute = Z_RESUB;
                   }

                else
                   {
                      AssignMatrix (TransMatrix, MULT_SUBSTITUTE);
                      resubstitute = MULT_RESUB;
                   }


      /*  Rotate about X axis  */

      if (planeRotation.xPos)
         {
            AssignMatrix (ResultMatrix, X_ROTATE);
            previous = TRUE;
         }


      /*  Rotate about Y axis  */

      if (planeRotation.yPos)
         {
            if (previous)
               {
                  AssignMatrix (TempMatrix, Y_ROTATE);
                  MatrixMultiply (ResultMatrix, TempMatrix);
               }

            else
               {
                  AssignMatrix (ResultMatrix, Y_ROTATE);
                  previous = TRUE;
               }

         }


      /*  Rotate about Z axis  */

      if (planeRotation.zPos)
         {
            if (previous)
               {
                  AssignMatrix (TempMatrix, Z_ROTATE);
                  MatrixMultiply (ResultMatrix, TempMatrix);
               }

            else
               AssignMatrix (ResultMatrix, Z_ROTATE);


         }



      /*  Apply the axis transformations to the multiple rotation matrix  */

      MatrixMultiply (ResultMatrix, TransMatrix);
      AssignMatrix (TransMatrix, resubstitute);
      MatrixMultiply (ResultMatrix, TransMatrix);


      /*  Determine the final coordinates  */

      VectorMultiply (ray, ResultMatrix);
      return;
   }



/*----------------------------------------------------------------------------

      This function is used to deterimine the pixel color value determined
   by the corresponding surface point and the given light source.  The
   function takes the coordinates of the light source, the surface
   intersection and the surface normal as arguments.  In addition, the
   saturation angle and a flooding boolean are passed to the function.
   The function returns an intensity value which corresponds to the
   predefined color palette.

----------------------------------------------------------------------------*/

int Illuminate (candle, normal, hitNode, lighting, overSpill)
COORD_TYPE     candle, normal;
POINT_LIST     far *hitNode;
int            lighting;
short          overSpill;
   {
      int         intensity;
      double      numerator, denominator;
      double      incidence, theta, sub1, sub2;


      /*  Calculations for the angle of incidence  */

      numerator  =  ((double) (hitNode->local.xPos - candle.xPos) * (hitNode->local.xPos - normal.xPos));
      numerator +=  ((double) (hitNode->local.yPos - candle.yPos) * (hitNode->local.yPos - normal.yPos));
      numerator +=  ((double) (hitNode->local.zPos - candle.zPos) * (hitNode->local.zPos - normal.zPos));

      sub1  =  ((double) (candle.xPos - hitNode->local.xPos) * (candle.xPos - hitNode->local.xPos));
      sub1 +=  ((double) (candle.yPos - hitNode->local.yPos) * (candle.yPos - hitNode->local.yPos));
      sub1 +=  ((double) (candle.zPos - hitNode->local.zPos) * (candle.zPos - hitNode->local.zPos));

      sub2  =  ((double) (normal.xPos - hitNode->local.xPos) * (normal.xPos - hitNode->local.xPos));
      sub2 +=  ((double) (normal.yPos - hitNode->local.yPos) * (normal.yPos - hitNode->local.yPos));
      sub2 +=  ((double) (normal.zPos - hitNode->local.zPos) * (normal.zPos - hitNode->local.zPos));

      if (sub1 >= 0 && sub2 >= 0)
         denominator = sqrt (sub1) * sqrt (sub2);
      else
         denominator = 0;

      if (denominator != 0)
         theta = acos ((double) numerator / denominator);
      else
         theta = HEMISPHERE;

      incidence = (theta * HEMISPHERE) / PI;


      /*  Compare the angle of incidence to the saturation angle  */

      if (incidence > lighting)
         return (grBLACK);

      else
         {
            if (overSpill)
               {
                  /*  Use the color palettes extended color range to include
                      varying shades of white                               */

                  intensity = (int) ((FULL_RANGE - 1) - ((incidence / lighting) * (FULL_RANGE - 1)));
                  if (intensity >= COLOR_RANGE)
                     return ((STD_COLOR + (WHITE_LIGHT * hitNode->imagePtr->color)) - ((FULL_RANGE - 1) - intensity));

                  else
                     return (intensity + (COLOR_RANGE * hitNode->imagePtr->color));

               }

            else
               {
                  /*  Use the color palette's standard range for the selected
                      image color                                             */

                  intensity = (int) ((COLOR_RANGE - 1) - ((incidence / lighting) * (COLOR_RANGE - 1)));
                  return (intensity + (COLOR_RANGE * hitNode->imagePtr->color));
               }

         }

   }



/*----------------------------------------------------------------------------

      This function returns a pointer to an image whose surface is closest
  to the viewer's perspective.  This selection is made from among all images
  whose surfaces have intersections with a given ray.  The lone argument
  is the list of such images.

----------------------------------------------------------------------------*/

POINT_LIST far * FindImage (startNode)
POINT_LIST far *startNode;
   {
      POINT_LIST     far *hitNode, far *foundNode;
      int            closest = MAX_DEPTH;


      /*  Search through the hit list  */

      hitNode = startNode;
      while (hitNode != NULL)
         {
            /*  Record the closest surface  */

            if (hitNode->local.zPos <= closest)
               {
                  foundNode = hitNode;
                  closest = hitNode->local.zPos;
             }

            hitNode = hitNode->next;
         }

      return (foundNode);
   }



/*----------------------------------------------------------------------------

      This utility procedure traverses the hitnode list and frees all nodes.

----------------------------------------------------------------------------*/

void FreeList (hitNode)
POINT_LIST     far *hitNode;
   {
      POINT_LIST     far *startNode, far *lastNode;

      lastNode = startNode = hitNode;
      while (startNode->next != NULL)
         {
            /*  Go to the bottom of the list  */

            while (hitNode->next != NULL)
               {
                  lastNode = hitNode;
                  hitNode = hitNode->next;
               }


            /*  Free the last node and go back to the start of the list  */

            farfree (hitNode);
            lastNode->next = NULL;
            hitNode = startNode;
         }


      /*  Free the first and only node  */

      farfree (startNode);
      return;
   }
