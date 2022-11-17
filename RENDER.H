/*----------------------------------------------------------------------------

File:                   Render.h

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 14, 1992
Last Modified:          July  21, 1992

Description:            This file contains a list of defines that are 
                        particular to the RENDER.C source code module.

----------------------------------------------------------------------------*/


/*  General Definitions  */

#define  NO_DISP              -1
#define  HALF                 -1
#define  NOSUCHFILE           -1
#define  START                -1
#define  EMPTY                 0
#define  BLACK                 0
#define  EXIST                 00
#define  NO_ARGS               1
#define  FIRST_ARG             1
#define  HELP_ARG              2
#define  PCX_ARG               2
#define  RED_ARG               2
#define  SWITCH_SIZE           2
#define  MAX_RED               8
#define  MODES                 24
#define  DEF_SAT               140


/*  Definitions for binary encoding of rendering methods  */

#define  M_ENCODE              3
#define  METHODS               7
#define  RS_MASK               7
#define  LS_MASK              56


/*  Definitions for configuration read function  */

#define  SUN_XPOS             -1500
#define  SUN_YPOS             -1000
#define  SUN_ZPOS             -1000
#define  SAT_ANGLE             135
#define  LABEL_SIZE            5
#define  FLD_SIZE              6


/*  Definitions for menu tree designators  */

#define  ROOT                 -1
#define  M_MAIN                0
#define  ORPHAN                0
#define  SUB1                  1
#define  SUB2                  2
#define  SUB3                  3
#define  SUB4                  4
#define  SUB5                  5
#define  SUB6                  6
#define  SUB7                  7
#define  SUB8                  8
#define  SUB9                  9
#define  SUB10                 10
#define  SUB11                 11


/*  Definitions for menu tree options  */

#define  NO_OPT               -1
#define  OPT1                  0
#define  OPT2                  1
#define  OPT3                  2
#define  OPT4                  3
#define  OPT5                  4
#define  OPT6                  5
#define  OPT7                  6
#define  OPT8                  7
