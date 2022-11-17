/*----------------------------------------------------------------------------

File:                   Eval.h
Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 16, 1992
Last Modified:          July 21, 1992

Description:            This file contains definitions relative to the source
                        code file EVAL.C.  

----------------------------------------------------------------------------*/


/*  Definitions for the calls to the menuChoice function regarding the status
    of selected menu options.                                               */

#define  PUSH                   -1
#define  POP                     1
#define  FLUSH                   0


/*  General Definitions  */

#define  IDLE                    0
#define  RND_UP                  0.5
#define  IMAGE_MENU              3
#define  DUAL_WIDTH              5
#define  FIELD_COUNT             7


/*  Menu option and method option definitions  */

#define  FIRST_OPT               0
#define  ITER_OPT                0
#define  UNI_OPT                 1
#define  FAT_OPT                 2
#define  OR_OPT                  3


/*  Masking definitions for method encoding  */

#define  HL_MASK                 127
#define  VL_MASK                 128
#define  HR_MASK                 191
#define  VR_MASK                 64
#define  CL_MASK                 71
#define  CR_MASK                 184


/*  Definitions for virtual buffering functions  */

#define  MIN_VIRT                1048576
#define  PAGE_0                  0
#define  VIRT_X                  0
#define  VIRT_Y                  0
