/*----------------------------------------------------------------------------

File:                   Surface.h
Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           May 16, 1992
Last Modified:          July 21, 1992

Description:            This file contains the definitions and structures
                        for the source file SURFACE.C.


----------------------------------------------------------------------------*/


/*  Illumination constants  */

#define  THRESH           1.0e-6
#define  QEPS             2.0e-8
#define  QEPS2            1.0e-22
#define  NEC_ASPECT       0.8625
#define  DIRECT           1
#define  TANGENT_HIT      2
#define  PI               3.141592654
#define  COLOR_RANGE      64
#define  FULL_RANGE       80
#define  QUADRANT         90.0
#define  HEMISPHERE       180.0


/*  Rotation Constants  */

#define  X_SUBSTITUTE     0
#define  Y_SUBSTITUTE     1
#define  Z_SUBSTITUTE     2
#define  MULT_SUBSTITUTE  3
#define  X_RESUB          4
#define  Y_RESUB          5
#define  Z_RESUB          6
#define  MULT_RESUB       7
#define  X_ROTATE         8
#define  Y_ROTATE         9
#define  Z_ROTATE         10
#define  RDIM             [4][4]
#define  RROW             4
#define  RCOL             4
#define  PASS1            1
#define  PASS2            2
#define  NUM_AXIS         3
#define  CENTER_PLANE     0
#define  CENTER_SPACE    -1


/*  Binary decoding for rendering methods  */

#define  VH_MASK          1
#define  R_SHIFT          3
#define  METHOD_CODE      3
#define  LO_SHIFT         6
#define  RO_SHIFT         7
#define  R_MASK           7
#define  OR_MASK          64
#define  OL_MASK          128


/*  General definitions  */

#define  UPD_START       -1
#define  NEG             -1
#define  BI_ACC           0.000001
#define  BR_ACC           0.00390625
#define  B_RANGE          0.05
#define  I_RANGE          0.25
#define  CUBE_ROOT        (1.0 / 3.0)
#define  POS              1
#define  MENU             1
#define  XY_PLANE         1
#define  YZ_PLANE         2
#define  ZX_PLANE         3
#define  MAX_DEGREE       6
#define  MAX_FIELD        7
#define  STD_COLOR        15
#define  WHITE_LIGHT      16
#define  INTERVAL         30
#define  MAX_FORM         30
#define  MAX_BI           40
#define  MAX_ITER         100
#define  MAX_M            100
#define  FULL_ROTATION    360
#define  MIN_AVAIL        1024
#define  MAX_Z            1024
#define  CENTER_Z         MAX_Z / 2
#define  MAX_DEPTH        32767


/*  Type definitions  */

typedef struct  space     COORD_TYPE;
typedef struct  image     IMAGE_TYPE;
typedef struct  vector    RAY_TYPE;
typedef struct  point     POINT_TYPE;
typedef struct  plist     POINT_LIST;
typedef struct  region    REGION_TYPE;


/*  Structure Definitions  */

struct space
   {
      long                     xPos;
      long                     yPos;
      long                     zPos;
   };

struct image
   {
      char                     surface [MAX_FORM];
      COORD_TYPE               center;
      int                      size;
      int                      size2;
      int                      color;
      IMAGE_TYPE          far *next;
   };

struct point
   {
      int                      x;
      int                      y;
   };

struct plist
   {
      COORD_TYPE               local;
      IMAGE_TYPE          far *imagePtr;
      POINT_LIST          far *next;
   };

struct region
   {
      POINT_TYPE               upper_left;
      POINT_TYPE               lower_right;
   };

struct vector
   {
      long                     refx;
      long                     refy;
      long                     refz;
   };
