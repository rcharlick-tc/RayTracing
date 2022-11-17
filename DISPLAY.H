/*----------------------------------------------------------------------------

File:                   Display.h
Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 16, 1992
Last Modified:          July 21, 1992

Description:            This file contains definitions for the source code
                        file DISPLAY.C
         

----------------------------------------------------------------------------*/



/*  Definitions for color palette assignment  */

#define     BCOLOR_1         48
#define     BCOLOR_2         96
#define     BCOLOR_3         144
#define     VCOLOR_1         192
#define     VCOLOR_2         384
#define     VCOLOR_3         576
#define     END_VCOLOR       768

#define     RGB              3
#define     BLEND            13
#define     MAX_INTENSITY    63
#define     REQD_COLORS      256

#define     PAL_CGA          2
#define     PAL_EGA          17
#define     PAL_VGA          48
#define     PAL_SVGA         768

#define     RED_SHADE        0
#define     GREEN_SHADE      1
#define     BLUE_SHADE       2


/*  Definitions for standard video mode vertical thresholds  */

#define     V_CGA            200
#define     V_EGA            350
#define     V_VGA            480


/*  Standard graphic constants and other general definitions  */

#define     NO_SHOW          0
#define     SEP_WIDTH        1
#define     NO_CLEAR         1
#define     X_SHADE          2
#define     Y_SHADE          2
#define     HIMLINE          3
#define     HMNUM            3
#define     HDNUM            3
#define     HIDLINE          4
#define     CHAR_WIDTH       8
#define     BASE             10
#define     HIRES            14
#define     SECMIN           60
#define     SECHR            3600


/*  Graphics image ratio constants  */

#define     M_RATIO          0.025
#define     RATIO_MONO       0.100
#define     VERT_DIALOG      0.135
#define     RATIO_DUAL       0.150
#define     BUT_WIDTH        0.200
#define     VERT_INFO        0.250
#define     VERT_TITLE       0.280
#define     FILE_SPACE       0.315
#define     HOR_INFO         0.330
#define     HOR_TITLE        0.480
#define     FLD_ADJ          1.600
#define     ALGN_CNTR        3.333
#define     ALGN_CNTR2       3.846


/*  Update routine field designator definitions  */

#define     METH_FLD         0
#define     IMG_FLD          1
#define     REG_FLD          2
#define     FLO_FLD          3
#define     SAT_FLD          4
#define     PCT_FLD          5
#define     ELAP_FLD         6

#define     FULL             0
#define     REST_MODE        0
#define     SAVE_MODE        1
#define     METHOD           1
#define     SCENE            2
#define     AREA             3
#define     FLOOD            4
#define     MAX_SAT          4
#define     MAX_FLO          4
#define     MAX_PCT          6
#define     MAX_ELAP         8
#define     MAX_REG          11
#define     MAX_IMG          12
#define     INPUT_STR        13
#define     PIX_LINE         15
#define     MAX_METH         28
#define     REG_SIZE         48
#define     ELAP_ABR         56
#define     FLO_SIZE         64
#define     IMG_SIZE         64
#define     METH_SIZE        72
#define     PCT_ABR          88
#define     SAT_FULL         104
#define     ELAP_FULL        120
#define     PCT_FULL         152
