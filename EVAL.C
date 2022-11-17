/*----------------------------------------------------------------------------

File:                   Eval.c

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 8, 1992
Last Modified           July 22, 1992

Description:            This file contains the primary menu evaluation
                        routines necessary to implement the menu mode
                        features of this project.  Many of the menu
                        library functions are called from these modules.
                        In addition, the majority of the mouse routines
                        are called from here as well.


---------------------------------------------------------------------------*/


/*  Graphics and other supplied include files  */

#include <gxlib.h>
#include <grlib.h>
#include <pcxlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <time.h>


/*  Project include files   */

#include "render.h"
#include "menu.h"
#include "eval.h"
#include "globals.h"
#include "surface.h"
#include "select.h"
#include "display.h"
#include "errmsg.h"


/*  Global Variables  */

extern           REGION_TYPE    rightRegion, leftRegion;
extern           COORD_TYPE     sun;
extern           int            saturate, backColor, imageNumber;
extern           GXDINFO        disp_info;
extern           MENU_TYPE      far *rootTree;
COORD_TYPE       centerRotation;
GXHEADER         vPtr;
FILE            *fImage;


/*  Internal function prototypes  */

void  ClearRegion (int);
int   MenuChoice (int, int);
IMAGE_TYPE far * AssignImage (char far *);
int   GetMouseChoice (int far *, int far *, int, int, int, int, int);


/*  External function prototypes  */

extern void ShowTitle (int);
extern void StatusText (int);
extern int  ClearStatus (int);
extern void ShowInfo (int, int);
extern void DrawSep (int, int, int);
extern int  Strct (char far *, char);
extern void SetRegion (int, int, int);
extern int  DisplayStatus (int, int, int);
extern int  ParseString (char far *, char far *);
extern int  ShowStatus (int, int, char *, char *, int, int, int);
extern void GetFileName (char far *, int, int);
extern void Alert (char far *, int);
extern int  Driver (IMAGE_TYPE far *, IMAGE_TYPE far *, int, int, int, int,
                    int, COORD_TYPE, short);



/*----------------------------------------------------------------------------

         This function is used to evaluate and execute options selected by
      the user in the menu mode.  The number of colors supported from the 
      current video mode and the video mode are passed as arguments.  The
      function returns a success or error code.

---------------------------------------------------------------------------*/

int MenuChoice (colors, video)
int      colors, video;
   {
      IMAGE_TYPE     far *leftImage, far *rightImage;
      MENU_TYPE      far *branch;
      char           far *leftScene, far *rightScene, far *fileStr;
      int            lastLeftImage = NO_OPT, lastRightImage = NO_OPT, mType, status = OFF, method= NONE, drawMode = MONO;
      int            lastStatus = OPT1, lastSide = OPT1, depress, quit, menu, option, hold;
      int            lastLeftMethod = OPT1, lastRightMethod = OPT1, menuOff = FALSE, flood = TRUE, side = LEFT;
      int            orSelect, lastLeftOrient [OR_OPT] = {OPT1, OPT1, OPT1}, lastRightOrient [OR_OPT] = {OPT1, OPT1, OPT1};
      int            clrTitle = FALSE, virtual = FALSE, seconds = NONE, scrClear = TRUE, errorCond = VALID;
      int            dtype, virtClear = TRUE;
      time_t         begTime, endTime;
      long           vfree;


      /*  Check for expanded memory  */

      if (gxEMSInstalled () == gxSUCCESS)
         {
            vfree = gxVirtualFree (gxEMM);
            if (vfree >= MIN_VIRT)
               virtual = TRUE;

         }


      /*  Allocation and verification of memory for data structures  */

      leftScene = (char far *) farcalloc (LARGE_STR, sizeof (char));
      rightScene = (char far *) farcalloc (LARGE_STR, sizeof (char));
      fileStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      leftImage = (IMAGE_TYPE far *) farcalloc (SINGLE, sizeof (IMAGE_TYPE));
      rightImage = (IMAGE_TYPE far *) farcalloc (SINGLE, sizeof (IMAGE_TYPE));
      branch = (MENU_TYPE far *) farcalloc (SINGLE, sizeof (MENU_TYPE));
      if (leftScene == NULL || rightScene == NULL || leftImage == NULL || rightImage == NULL || branch == NULL ||
          fileStr == NULL)
         return (m_ERR_ALLOC);

      quit = FALSE;
      _fstrcpy (leftScene, "None");
      _fstrcpy (rightScene, "None");
      SetRegion (drawMode, status, menuOff);
      ShowTitle (ON);


      /*  Set initial menu privileges  */

      errorCond = mnEnableOpt (SUB6, OPT2, DISABLE);
      errorCond = mnEnableOpt (SUB1, OPT2, DISABLE);
      errorCond = mnEnableOpt (SUB1, OPT3, DISABLE);
      errorCond = mnEnableOpt (SUB1, OPT5, DISABLE);


      /*  Create virtual buffer for image persistence against menu displays  */

      if (virtual)
         errorCond = gxCreateVirtual (gxEMM, &vPtr, video, disp_info.hres, disp_info.vres);


      /*  Main menu loop  */

      while (!quit && !errorCond)
         {
            depress = FALSE;


            /*  Determine the selected options from the current mouse state  */

            errorCond = GetMouseChoice (&menu, &option, status, drawMode, virtual, virtClear, scrClear);
            if (!clrTitle)
               {
                  ShowTitle (OFF);
                  clrTitle = TRUE;
               }


            /*  Execute the selected menu and option choice  */

            switch (menu)
               {
                  /*  Main menu is active  */

                  case M_MAIN:
                     switch (option)
                        {
                           /*  Execute option selected from main menu  */

                           case OPT1:
                              errorCond = mnDispMenu (SUB1, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;


                           /*  Display option selected from main menu  */

                           case OPT2:
                              errorCond = mnDispMenu (SUB2, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;


                           /*  Options option selected from main menu  */

                           case OPT3:
                              errorCond = mnDispMenu (SUB3, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;


                           /*  Image option selected from main menu  */

                           case OPT4:
                              errorCond = mnDispMenu (SUB5, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;

                        }

                     break;


                  /*  Execute menu is active  */

                  case SUB1:
                     switch (option)
                        {
                           /*  Run option selected from execute menu  */

                           case OPT1:
                              mnClearMenu (SUB1, scrClear);
                              if (drawMode == MONO)
                                 leftImage = AssignImage (leftScene);
                              else
                                 {
                                    leftImage = AssignImage (leftScene);
                                    rightImage = AssignImage (rightScene);
                                 }


                              /*  Set menu privileges  */

                              errorCond = mnEnableOpt (SUB1, OPT1, DISABLE);
                              errorCond = mnEnableOpt (SUB1, OPT3, ENABLE);
                              errorCond = mnEnableOpt (SUB1, OPT5, ENABLE);
                              errorCond = mnEnableOpt (SUB1, OPT6, DISABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT2, DISABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT3, DISABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT4, DISABLE);
                              ClearRegion (drawMode);


                              /*  Special considerations for running without
                                  the main menu bar                         */

                              if (menuOff)
                                 {
                                    mnClearMenu (M_MAIN, TRUE);
                                    if (drawMode == DUAL)
                                       DrawSep (grYELLOW, status, TRUE);

                                    begTime = time (NULL);
                                    errorCond = Driver (leftImage, rightImage, method, drawMode, saturate, backColor, 
                                                        status, sun, flood);
                                    endTime = time (NULL);
                                    if (errorCond != m_STOP)
                                       getch ();

                                    hold = mnDispMenu (M_MAIN, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                                 }
                              else
                                 {
                                    errorCond = mnUpdateMenu (M_MAIN);
                                    begTime = time (NULL);
                                    errorCond = Driver (leftImage, rightImage, method, drawMode, saturate, backColor, 
                                                        status, sun, flood);
                                    endTime = time (NULL);
                                 }


                              /*  See if rendering was prematurely aborted  */

                              if (errorCond != m_STOP)
                                 {
                                    seconds = abs (difftime (begTime, endTime));
                                    errorCond = mnEnableOpt (SUB1, OPT2, ENABLE);
                                 }

                              else
                                 errorCond = mnEnableOpt (SUB1, OPT2, DISABLE);


                              /*  Check previous menu display retrospectively  */

                              if (!errorCond && menuOff)
                                 errorCond = hold;


                              /*  Save the appropriate rendering region to a
                                  virtual buffer                            */

                              if (virtual)
                                 errorCond = gxDisplayVirtual (VIRT_X, VIRT_Y, disp_info.hres - 1, disp_info.vres - 1, PAGE_0,
                                                               &vPtr, VIRT_X, VIRT_Y);

                              virtClear = scrClear = FALSE;
                              depress = TRUE;
                              break;


                           /*  The info option was selected from the execute
                               menu                                         */

                           case OPT2:
                              mnClearMenu (SUB1, scrClear);


                              /*  Restore the appropriate rendering region 
                                  from a virtual buffer after the menu is
                                  cleared                                   */

                              if (virtual && !virtClear)
                                 errorCond = gxVirtualDisplay (&vPtr, VIRT_X, VIRT_Y, VIRT_X, VIRT_Y, disp_info.hres - 1, 
                                                               disp_info.vres - 1, PAGE_0);

                              ShowInfo (mnGRAY, seconds);


                              /*  Restore the appropriate rendering region
                                  from a virtual buffer after the information
                                  screen is cleared                         */

                              if (virtual && !virtClear)
                                 errorCond = gxVirtualDisplay (&vPtr, VIRT_X, VIRT_Y, VIRT_X, VIRT_Y, disp_info.hres - 1, 
                                                               disp_info.vres - 1, PAGE_0); 


                              depress = TRUE;
                              break;


                           /*  The clear option was selected from the execute
                               menu                                         */

                           case OPT3:
                              mnClearMenu (SUB1, scrClear);
                              ClearRegion (drawMode);


                              /*  Set the menu privileges  */

                              errorCond = mnEnableOpt (SUB1, OPT1, ENABLE);
                              errorCond = mnEnableOpt (SUB1, OPT2, DISABLE);
                              errorCond = mnEnableOpt (SUB1, OPT3, DISABLE);
                              errorCond = mnEnableOpt (SUB1, OPT5, DISABLE);
                              errorCond = mnEnableOpt (SUB1, OPT6, ENABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT2, ENABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT3, ENABLE);
                              errorCond = mnEnableOpt (M_MAIN, OPT4, ENABLE);
                              errorCond = mnUpdateMenu (M_MAIN);
                              if (status)
                                 {
                                    /*  Redisplay the status region(s)  */

                                    status = DisplayStatus (colors, mnGRAY, drawMode);
                                    StatusText (drawMode);
                                    errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, FULL);
                                 }

                              virtClear = scrClear = TRUE;
                              depress = TRUE;
                              break;


                           /*  Save the rendering region(s) to a PCX file  */

                           case OPT5:
                              GetFileName (fileStr, SAVE_MODE, mnGRAY);
                              errorCond = pcxVirtualFile (&vPtr, VIRT_X, VIRT_Y, disp_info.hres - 1, disp_info.vres - 1, fileStr);
                              depress = TRUE;
                              mnClearMenu (SUB1, TRUE);
                              errorCond = gxVirtualDisplay (&vPtr, VIRT_X, VIRT_Y, VIRT_X, VIRT_Y, disp_info.hres - 1, 
                                                            disp_info.vres - 1, PAGE_0);
                              errorCond = mnEnableOpt (SUB1, OPT5, DISABLE);
                              break;


                           /*  Restore a PCX file to the screen  */

                           case OPT6:
                              GetFileName (fileStr, REST_MODE, mnGRAY);
                              dtype = pcxGetFileType (fileStr);
                              depress = TRUE;
                              mnClearMenu (SUB1, TRUE);
                              if (dtype == video) 
                                 {
                                    /*  Display the image and Set menu privileges  */

                                    pcxFileVirtual (fileStr, &vPtr, VIRT_X, VIRT_Y);   
                                    gxVirtualDisplay (&vPtr, VIRT_X, VIRT_Y, VIRT_X, VIRT_Y, disp_info.hres - 1, 
                                                      disp_info.vres - 1, PAGE_0);
                                    getch ();
                                    mnEnableOpt (SUB1, OPT1, DISABLE);
                                    mnEnableOpt (SUB1, OPT2, DISABLE);
                                    mnEnableOpt (SUB1, OPT3, ENABLE);
                                    mnEnableOpt (SUB1, OPT5, DISABLE);
                                    mnEnableOpt (SUB1, OPT6, DISABLE);
                                    mnEnableOpt (M_MAIN, OPT2, DISABLE);
                                    mnEnableOpt (M_MAIN, OPT3, DISABLE);
                                    mnEnableOpt (M_MAIN, OPT4, DISABLE);
                                    mnUpdateMenu (M_MAIN);
                                    virtClear = errorCond = FALSE;
                                 }

                              else if (dtype == gxERR_NOTFOUND)
                                      Alert ("File not Found!", mnGRAY);

                                   else
                                      Alert ("Incorrect PCX mode!", mnGRAY);

                              break;


                           /*  Quit the program !  */

                           case OPT8:
                              quit = TRUE;
                              break;

                        }

                     break;


                  /*  The display menu is active  */

                  case SUB2:
                     if ((option == OPT1 || option == OPT2) && scrClear)
                        errorCond = mnMarkOpt (SUB2, grGREEN, grGRAY, lastStatus, OFF, ON);

                     switch (option)
                        {
                           /*  Set mode to mono screen mode  */

                           case OPT1:
                              if (scrClear)
                                 {
                                    ClearStatus (drawMode);
                                    drawMode =  MONO;
                                    if (status)
                                       {
                                          status = DisplayStatus (colors, mnGRAY, drawMode);
                                          StatusText (drawMode);
                                          SetRegion (drawMode, status, menuOff);
                                          errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, FULL);
                                       }

                                    else
                                       SetRegion (drawMode, status, menuOff);

                                    DrawSep (grBLACK, status, FALSE);
                                    errorCond = mnEnableOpt (SUB6, OPT2, DISABLE);
                                    errorCond = mnMarkOpt (SUB6, grGREEN, grGRAY, OPT2, OFF, OFF);
                                    errorCond = mnMarkOpt (SUB6, grGREEN, grGRAY, OPT1, ON, OFF);
                                    side = LEFT;
                                 }

                              break;


                           /*  Set screen rendering to dual screen mode  */

                           case OPT2:
                              if (scrClear)
                                 {
                                    ClearStatus (drawMode);
                                    drawMode = DUAL;
                                    if (status)
                                       {
                                          status = DisplayStatus (colors, mnGRAY, drawMode);
                                          StatusText (drawMode);
                                          SetRegion (drawMode, status, menuOff);
                                          errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, FULL);
                                       }

                                    else
                                       SetRegion (drawMode, status, menuOff);

                                    DrawSep (grYELLOW, status, FALSE);
                                    errorCond = mnEnableOpt (SUB6, OPT2, ENABLE);
                                 }

                              break;


                           /*  Display left-right screen select menu  */

                           case OPT4:
                              if (side == RIGHT)
                                 {
                                    errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastRightMethod, OFF, OFF);
                                    errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastLeftMethod, ON, OFF);
                                 }

                              else
                                 {
                                    errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastLeftMethod, OFF, OFF);
                                    errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastRightMethod, ON, OFF);
                                 }

                              errorCond = mnDispMenu (SUB6, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;

                        }


                     /*  Toggle between mono and dual screen mode indicators  */

                     if ((option == OPT1 || option == OPT2) && scrClear)
                        {
                           lastStatus = option;
                           errorCond = mnMarkOpt (SUB2, grGREEN, grGRAY, option, ON, ON);
                        }

                     break;


                  /*  The options menu is active  */

                  case SUB3:
                     switch (option)
                        {
                           /*  Toggle status screen on or off  */

                           case OPT1:
                              if (scrClear)
                                 {
                                    /*  Turn status on if it's off and off if 
                                        it is on                             */

                                    if (status)
                                       {
                                          status = ClearStatus (drawMode);
                                          SetRegion (drawMode, status, menuOff);
                                          errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT1, OFF, ON);
                                       }

                                    else
                                       {
                                          status = DisplayStatus (colors, mnGRAY, drawMode);
                                          SetRegion (drawMode, status, menuOff);
                                          StatusText (drawMode);
                                          errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, FULL);
                                          errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT1, ON, ON);
                                       }


                                    /*  Draw or erase screen seperator bar  */

                                    if (drawMode == DUAL)
                                       DrawSep (grYELLOW, status, FALSE);
                                    else
                                       DrawSep (grBLACK, status, FALSE);

                                 }

                              break;


                           /*  Toggle menu off option  */

                           case OPT2:
                              if (menuOff)
                                 {
                                    menuOff = FALSE;
                                    SetRegion (drawMode, status, menuOff);
                                    if (status)
                                       errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, AREA);

                                    errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT2, OFF, ON);

                                 }

                              else
                                 {
                                    menuOff = TRUE;
                                    SetRegion (drawMode, status, menuOff);
                                    if (status)
                                       errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, AREA);

                                    errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT2, ON, ON);

                                 }

                              break;


                           /*  Display the pixel selection technique menu  */

                           case OPT4:
                              errorCond = mnDispMenu (SUB4, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              depress = TRUE;
                              break;


                           /*  Enable or diasable flood lighting  */

                           case OPT5:
                              if (flood)
                                 {
                                    flood = FALSE;
                                    errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT5, OFF, ON);
                                 }

                              else
                                 {
                                    flood = TRUE;
                                    errorCond = mnMarkOpt (SUB3, grGREEN, grGRAY, OPT5, ON, ON);
                                 }

                              if (status)
                                 errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, FLOOD);

                              break;

                        }

                     break;


                  /*  Technique menu is activated  */

                  case SUB4:
                     /*  Maintain status of selected methods  */
               
                     if (side == LEFT)
                        errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastLeftMethod, OFF, ON);
                     else
                        errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, lastRightMethod, OFF, ON);

                     switch (option)
                        {
                           /*  Iterative method selected  */

                           case OPT1:
                              /*  Restore state of marked options  */

                              errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, OPT1, ON, ON);
                              if (side == LEFT)
                                 {
                                    errorCond = mnMarkOpt (SUB7, grGREEN, grGRAY, lastRightOrient [ITER_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB7, grGREEN, grGRAY, lastLeftOrient [ITER_OPT], ON, OFF);
                                 }
                              else
                                 {
                                    errorCond = mnMarkOpt (SUB7, grGREEN, grGRAY, lastLeftOrient [ITER_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB7, grGREEN, grGRAY, lastRightOrient [ITER_OPT], ON, OFF);
                                 }

                              errorCond = mnDispMenu (SUB7, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              orSelect = ITER_OPT;
                              depress = TRUE;
                              break;


                           /*  Uniform scan method selected  */

                           case OPT2:
                              /*  Restore state of marked options  */

                              errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, OPT2, ON, ON);
                              if (side == LEFT)
                                 {
                                    errorCond = mnMarkOpt (SUB8, grGREEN, grGRAY, lastRightOrient [UNI_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB8, grGREEN, grGRAY, lastLeftOrient [UNI_OPT], ON, OFF);
                                 }
                              else
                                 {
                                    errorCond = mnMarkOpt (SUB8, grGREEN, grGRAY, lastLeftOrient [UNI_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB8, grGREEN, grGRAY, lastRightOrient [UNI_OPT], ON, OFF);
                                 }

                              errorCond = mnDispMenu (SUB8, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              orSelect = UNI_OPT;
                              depress = TRUE;
                              break;


                           /*  Fat uniform scan method selected  */

                           case OPT4:
                              /*  Restore state of marked options  */

                              errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, OPT4, ON, ON);
                              if (side == LEFT)
                                 {
                                    errorCond = mnMarkOpt (SUB9, grGREEN, grGRAY, lastRightOrient [FAT_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB9, grGREEN, grGRAY, lastLeftOrient [FAT_OPT], ON, OFF);
                                 }
                              else
                                 {
                                    errorCond = mnMarkOpt (SUB9, grGREEN, grGRAY, lastLeftOrient [FAT_OPT], OFF, OFF);
                                    errorCond = mnMarkOpt (SUB9, grGREEN, grGRAY, lastRightOrient [FAT_OPT], ON, OFF);
                                 }

                              errorCond = mnDispMenu (SUB9, mnGRAY, mnBLACK, mnWHITE, grGREEN);
                              orSelect = FAT_OPT;
                              depress = TRUE;
                              break;

                        }


                     /*  Binary encoding of global rendering model  */

                     if (side == LEFT)
                        {
                           method = method & CL_MASK;
                           method = method | (option << M_ENCODE);
                           if (lastLeftOrient [orSelect] == OPT1)
                              method = method & HL_MASK;
                           else
                              method = method | VL_MASK;
                        }

                     else
                        {
                           method = method & CR_MASK;
                           method = method | option;
                           if (lastRightOrient [orSelect] == OPT1)
                              method = method & HR_MASK;
                           else
                              method = method | VR_MASK;
                        }


                     /*  Record active region menu state  */

                     if (side == LEFT)
                        lastLeftMethod = option;
                     else
                        lastRightMethod = option;

                     if (option == OPT3 || option == OPT5)
                        errorCond = mnMarkOpt (SUB4, grGREEN, grGRAY, option, ON, ON);

                     mType = option;
                     if (status)
                        errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, METHOD);

                     break;


                  /*  Image selection menu is active  */

                  case SUB5:
                     /*  Restore active region menu state  */

                     if (side == LEFT)
                        errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastLeftImage, OFF, ON);
                     else
                        errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastRightImage, OFF, ON);


                     /*  Assign chosen image name  */

                     branch = rootTree->next [IMAGE_MENU];
                     if (side == LEFT)
                        _fstrcpy (leftScene, branch->next [option]->title);
                     else
                        _fstrcpy (rightScene, branch->next [option]->title);

                     if (status)
                        errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, SCENE);


                     /*  Record current menu state  */

                     if (side == LEFT)
                        lastLeftImage = option;
                     else
                        lastRightImage = option;

                     errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, option, ON, ON);
                     break;


                  /*  Left-right selection menu is active  */

                  case SUB6:
                     /*  Toggle left-right selection  */

                     if ((lastSide == OPT1 || lastSide == OPT2) && (option == OPT1 || option == OPT2))
                        errorCond = mnMarkOpt (SUB6, grGREEN, grGRAY, lastSide, OFF, ON);


                     /*  Restore active menu state  */

                     if (option == OPT1)
                        {
                           errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastRightImage, OFF, OFF);
                           errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastLeftImage, ON, OFF);
                           side = LEFT;
                        }

                     if (option == OPT2)
                        {
                           errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastLeftImage, OFF, OFF);
                           errorCond = mnMarkOpt (SUB5, grGREEN, grGRAY, lastRightImage, ON, OFF);
                           side = RIGHT;
                        }


                     /*  Save active menu region  */

                     if (option == OPT1 || option == OPT2)
                        {
                           lastSide = option;
                           errorCond = mnMarkOpt (SUB6, grGREEN, grGRAY, option, ON, ON);
                        }

                     break;


                  /*  One of the three horizontal-vertical selection menus  
                      is active                                             */

                  case SUB7:
                  case SUB8:
                  case SUB9:
                     /*  Restore the current menu state for the active menu
                         region                                             */

                     errorCond = mnMarkOpt (menu, grGREEN, grGRAY, lastLeftOrient [menu - SUB7], OFF, ON);
                     errorCond = mnMarkOpt (menu, grGREEN, grGRAY, lastRightOrient [menu - SUB7], OFF, ON);


                     /*  Modify the encoded global rendering method */

                     if (side == LEFT)
                        {
                           method = (method & CL_MASK) | (mType << M_ENCODE);
                           if (option == OPT1)
                              method = method & HL_MASK;
                           else
                              method = method | VL_MASK;

                        }

                     else
                        {
                           method = (method & CR_MASK) | mType;
                           if (option == OPT1)
                              method = method & HR_MASK;
                           else
                              method = method | VR_MASK;

                        }


                     /*  Save the current menu state  */

                     if (side == LEFT)
                        lastLeftOrient [menu - SUB7] = option;
                     else
                        lastRightOrient [menu - SUB7] = option;

                     errorCond = mnMarkOpt (menu, grGREEN, grGRAY, option, ON, ON);
                     if (status)
                        errorCond = ShowStatus (drawMode, method, leftScene, rightScene, flood, saturate, METHOD);

                     break;

               }


            /*  Deselect the menu option where appropriate  */

            if (!depress)
               mnChooseOpt (menu, option, grGRAY, grBLACK, FLUSH);

         }


      /*  House cleaning  */

      farfree (leftImage);
      farfree (rightImage);
      farfree (leftScene);
      farfree (rightScene);
      farfree (fileStr);
      farfree (branch);
      if (virtual)
         errorCond = gxDestroyVirtual (&vPtr);

      mnEraseMenu (rootTree);
      grStopMouse ();
      return (errorCond);
   }



/*----------------------------------------------------------------------------

      This function takes as input a character string and opens an image
   file with the corresponding name.  Each field in the image record is
   assigned to an image node with a predefined structure.  As each record
   is read, an image list is built.  The list is returned.

---------------------------------------------------------------------------*/

IMAGE_TYPE far * AssignImage (fileName)
char           far *fileName;
   {
      IMAGE_TYPE  far *imageList, *imageNode, far *newNode;
      char        far *hold, far *temp;
      int         strSize, i, j, iCount = NONE, xSize, ySize, zSize;


      /*  Determine the region size for image scaling  */

      xSize = (leftRegion.lower_right.x - leftRegion.upper_left.x) + 1;
      ySize = (leftRegion.lower_right.y - leftRegion.upper_left.y) + 1;
      if (xSize < ySize)
         zSize = xSize;
      else
         zSize = ySize;


      /*  Allocate memory and check for errors  */

      temp = (char far *) farcalloc (LARGE_STR, sizeof (char));
      hold = (char far *) farcalloc (SMALL_STR, sizeof (char));
      imageList = (IMAGE_TYPE far *) farcalloc (SINGLE, sizeof (IMAGE_TYPE));
      imageNode = (IMAGE_TYPE far *) farcalloc (SINGLE, sizeof (IMAGE_TYPE));
      if (imageNode == NULL || imageList == NULL || temp == NULL || hold == NULL)
         return (NULL);


      /*  Open the image file for input */

      imageList = imageNode;
      _fstrcpy (hold, fileName);
      _fstrcat (hold, ".img");
      fImage = fopen (hold, "r");
      if (fImage == NULL)
         return (NULL);


      /*  Read all images in selected file  */

      while (!feof (fImage))
         {
            /*  Build an image list conatining one node for each image file
                record                                                      */

            fscanf (fImage, "%s\n", temp);
            newNode = (IMAGE_TYPE far *) farcalloc (SINGLE, sizeof (IMAGE_TYPE));
            if (newNode == NULL)
               return (NULL);


            /*  Verify a valid record format  */

            if ((Strct(temp, ',') + 1) == FIELD_COUNT)
               {
                  /*  Assign record fields to image variables */

                  ParseString (temp, imageNode->surface);
                  strSize = _fstrlen (imageNode->surface);
                  for (j = NONE; j < strSize; ++j)
                     imageNode->surface [j] = toupper (imageNode->surface [j]);


                  /*  For each field clear the hold string and parse the
                      input record                                       */

                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->center.xPos = (int) ((xSize * atof (hold)) + RND_UP);
                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->center.yPos = (int) ((ySize * atof (hold)) + RND_UP);
                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->center.zPos = (int) ((zSize * atof (hold)) + RND_UP);
                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->size = (int) ((zSize * atof (hold)) + RND_UP);
                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->size2 = (int) ((zSize * atof (hold)) + RND_UP);
                  for (i = NONE; i < SMALL_STR; ++i)
                     hold [i] = '\0';

                  ParseString (temp, hold);
                  imageNode->color = atoi (hold);
                  if ((iCount + 1) == imageNumber || (iCount == NONE && imageNumber > NONE))
                     {
                        centerRotation.xPos = imageNode->center.xPos;
                        centerRotation.yPos = imageNode->center.yPos;
                        centerRotation.zPos = imageNode->center.zPos;
                     }

               }

            else
               return (NULL);

            if (!feof (fImage))
               {
                  imageNode->next = newNode;
                  imageNode = newNode;
               }
            else
               imageNode->next = NULL;

            ++iCount;
         }


      /*  House cleaning  */

      farfree (newNode);
      farfree (temp);
      farfree (hold);
      fclose (fImage);

      return (imageList);
   }




/*----------------------------------------------------------------------------

      This procedure clears the currently defined rendering regions.  A
   single parameter to indicate mono or dual rendering mode is passed.  
   There is no value returned.

---------------------------------------------------------------------------*/

void ClearRegion (drawMode)
int      drawMode;
   {
      int      barHeight;


      /*  Account for the main menu region  */

      if (disp_info.vres == V_CGA)
         barHeight = (int) (V_EXT_RATIO * disp_info.vres);
      else
         barHeight = (int) (V_RATIO * disp_info.vres);

      grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);


      /*  Clear the region(s)  */

      if (drawMode == DUAL)
         if (rightRegion.upper_left.y < barHeight)
            grDrawRect (rightRegion.upper_left.x, barHeight + 1, rightRegion.lower_right.x,
                        rightRegion.lower_right.y, grFILL);

         else
            grDrawRect (rightRegion.upper_left.x, rightRegion.upper_left.y, rightRegion.lower_right.x,
                        rightRegion.lower_right.y, grFILL);

      if (leftRegion.upper_left.y < barHeight)
         grDrawRect (leftRegion.upper_left.x, barHeight + 1, leftRegion.lower_right.x,
                     leftRegion.lower_right.y, grFILL);

      else
         grDrawRect (leftRegion.upper_left.x, leftRegion.upper_left.y, leftRegion.lower_right.x,
                     leftRegion.lower_right.y, grFILL);

      return;
   }



/*----------------------------------------------------------------------------

      This function monitors mouse activity and determines the associated
   menu region that is selected when a mouse button is "clicked".  The 
   selected menu and option variable are returned through argument list.
   In addition, the function requires the status and drawMode variables
   for region size calculations.  The virtual, virtClear and scrClear
   variables are used to restore rendering region(s) after a sub menu
   has been displayed.  The function return a success or failure code.

---------------------------------------------------------------------------*/

int GetMouseChoice (menu, option, status, drawMode, virtual, virtClear, scrClear)
int      far *menu, far *option;
int      status, drawMode, virtual, virtClear, scrClear;
   {
      int      result, xPos, yPos, found, pressed = FALSE;


      result = VALID;
      found = FALSE;
      grDisplayMouse (grSHOW);
      while (!pressed)
         {
            pressed = grGetMouseButtons ();
            if (pressed & grLBUTTON)
               {
                  grGetMousePos (&xPos, &yPos);
                  grDisplayMouse (grHIDE);
                  do
                     {
                        /*  Find selected menu and option  */

                        *menu = mnFindActive (rootTree, &found);
                        *option = mnSelectOpt (xPos, yPos, *menu);


                        /*  Select option  */

                        if (*option != NO_OPTION)
                           result = mnChooseOpt (*menu, *option, grGRAY, grBLACK, PUSH);

                        else
                           {
                              if (*menu != M_MAIN)
                                 mnClearMenu (*menu, scrClear);


                              /*  Restore rendering region  */

                              if (virtual && !virtClear)
                                 result = gxVirtualDisplay (&vPtr, VIRT_X, VIRT_Y, VIRT_X,
                                                            VIRT_Y, disp_info.hres - 1, disp_info.vres - 1, PAGE_0);

                              else if ((*menu == SUB4 || *menu == SUB5) && drawMode == DUAL)
                                      DrawSep (grYELLOW, status, FALSE);
   
                              found = FALSE;
                           }

                     }
                  while (*menu != M_MAIN && *option == NO_OPTION);

                  grDisplayMouse (grSHOW);
                  while (grGetMouseButtons () != IDLE);

                  grDisplayMouse (grHIDE);
                  grDisplayMouse (grSHOW);
               }

         }

      grDisplayMouse (grHIDE);  
      return (result);
   }
