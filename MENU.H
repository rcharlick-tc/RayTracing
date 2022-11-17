/*----------------------------------------------------------------------------

File:                   Menu.h
Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 16, 1992
Last Modified:          July 21, 1992

Description:            This file contains menu library definitions, menu
                        structures and menu function prototypes.  This file
                        should be included in any module which uses any of
                        the included menu functions.


----------------------------------------------------------------------------*/


/*  General definitions for menuing library  */

#define  DIS_OPTION           -2
#define  DOWN                 -1
#define  NO_OPTION            -1
#define  MN_PARENT             0
#define  DISABLE               0
#define  MN_CHILD              1
#define  UP                    1
#define  ENABLE                1
#define  VERT_MENU             3
#define  HORIZ_MENU            4
#define  MAX_OPTIONS           12
#define  MAX_TITLE             18
#define  MAX_SPACE             30
#define  MENU_SPACE            44
#define  MAX_MENU_TEXT         80
#define  MARK_CHAR             175


/*  Menu sizing constants  */

#define  X_ORIGIN              0
#define  Y_ORIGIN              0
#define  V_RATIO               0.04
#define  V_EXT_RATIO           0.06
#define  SEL_SHADE             1
#define  X_SHADE               2
#define  Y_SHADE               2
#define  SEL_SIZE              5
#define  CGA_BOX               8
#define  CHAR_WIDTH            8
#define  EGA_BOX               14
#define  LEFT_OPT              16
#define  VGA_BOX               16


/*  Graphic mode specific constants  */

#define  V_CGA                 200
#define  V_HERC                348
#define  V_EGA                 350
#define  V_VGA                 480
#define  V_SVGA                600
#define  V_XGA                 768


/*  Menu error code definitions  */

#define  MN_VALID              0
#define  MN_EMPTY             -1
#define  MN_ERR_EMPTY         -1
#define  MN_ERR_OPTION        -2
#define  MN_ERR_ALLOC         -3
#define  MN_ERR_NEWALLOC      -4
#define  MN_ERR_DISPMENU      -5
#define  MN_ERR_CLRMISS       -6
#define  MN_ERR_DISPMISS      -7
#define  MN_ERR_SEPLINE       -8
#define  MN_ERR_DISORD        -9
#define  MN_ERR_CHOICE        -10
#define  MN_ERR_READ          -11
#define  MN_ERR_CLRMENU       -12
#define  MN_ERR_UPDMENU       -13
#define  MN_ERR_ENABLE        -14


/*  Menu color definitions  */

#define  mnBLACK               0
#define  mnBLUE                1
#define  mnGREEN               2
#define  mnCYAN                3
#define  mnRED                 4
#define  mnMAGENTA             5
#define  mnBROWN               6
#define  mnGRAY                7
#define  mnDARKGRAY            8
#define  mnLIGHTBLUE           9
#define  mnLIGHTGREEN          10
#define  mnLIGHTCYAN           11
#define  mnLIGHTRED            12
#define  mnLIGHTMAGENTA        13
#define  mnYELLOW              14
#define  mnWHITE               15


/*  Menu type definitions  */

typedef struct  tree           MENU_TYPE;
typedef struct  spot           SPOT_TYPE;
typedef struct  barRegion      BAR_TYPE;


/*  Menu Structures  */

struct spot
   {
      int                      x;
      int                      y;
   };

struct barRegion
   {
      SPOT_TYPE                upper_left;
      SPOT_TYPE                lower_right;
   };

struct tree
   {
      int                      menu_id;
      BAR_TYPE                 boundary;
      int                      num;
      int                      assigned;
      int                      active;
      int                      disabled;
      int                      selected;
      int                      display;
      char                     title [MAX_TITLE];
      MENU_TYPE         far   *next[MAX_OPTIONS];
   };


/*  Menu function prototypes  */

extern int mnDefMenu (int, int, int, char far *);
extern int mnDispMenu (int, int, int, int, int);
extern int mnUpdateMenu (int);
extern int mnClearMenu (int, int);
extern int mnEnableOpt (int, int, int);
extern int mnSelectOpt (int, int, int);
extern int mnMarkOpt (int, int, int, int, int, int);
extern int mnReadMenu (char far *, char far *);
extern int mnChooseOpt (int, int, int, int, int);
extern int mnFindActive (MENU_TYPE far *, int far *);
extern int mnEraseMenu (MENU_TYPE far *);
