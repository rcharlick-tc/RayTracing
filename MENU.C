/*----------------------------------------------------------------------------

File:                   Menu.c

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

Date Started:           April 8, 1992
Last Modified:          July 28, 1992

Description:            This file contains a complete set of menu routines
                        for creating a fully functional menu system.  This
                        library of menu functions was written for use with
                        Borland C++ version 2.0 or later and the Genus
                        microprogramming graphics library.  To use this
                        portable library, compile this MENU.C source file
                        with your other source code routines and make sure
                        any source file that uses the library has an
                        include for the MENU.H file.  There should also be
                        two global definitions in the main source file to be
                        compiled with this library (one of which is required 
                        by the Genus graphics library) as defined below.


----------------------------------------------------------------------------*/


/*  Graphics and other supplied include files  */

#include <gxlib.h>
#include <grlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>


/*  Project include files  */

#include "menu.h"


/*  Internal definitions  */

#define  NONE                  0
#define  FALSE                 0
#define  TRUE                  1
#define  OFF                   0
#define  ON                    1
#define  M_MAIN                0
#define  LARGE_STR            80


/*  Global variables, must be declared in external source file  */

extern   GXDINFO     disp_info;
extern   MENU_TYPE   far *rootTree;


/*  Internal function prototypes for callable menu functions  */

int mnDefMenu (int, int, int, char far *);
int mnDispMenu (int, int, int, int, int);
int mnUpdateMenu (int);
int mnClearMenu (int, int);
int mnEnableOpt (int, int, int);
int mnSelectOpt (int, int, int);
int mnChooseOpt (int, int, int, int, int);
int mnMarkOpt (int, int, int, int, int, int);
int mnFindActive (MENU_TYPE far *, int far *);
int mnReadMenu (char far *, char far *);
int mnEraseMenu (MENU_TYPE far *);


/*  Internal function prototypes for local functions  */

int  ParseString (char far *, char far *);
int  GetMenuPos (MENU_TYPE far *, int, int far *, int far *, int far *);
int  DrawMenuBox (int, int, int, int, int, int);
int  Strct (char far *, char);
void SetTextColor (int, int, int);
void TitleInfo (MENU_TYPE far *, int far *, int far *);
MENU_TYPE far * FindMenu (int, MENU_TYPE far *, int far *, int far *, int far *, int);



/*----------------------------------------------------------------------------

         This function builds the menu tree with the specified menu options.
   Submenus are linked to either a main menu or other sub menus by a
   specified menu option number.

----------------------------------------------------------------------------*/

int mnDefMenu (menuId, parent, option, menuString)
int    menuId, parent, option;
char   far *menuString;
   {
      MENU_TYPE   far *branch, far *newbranch;
      int         hold, done, i, found;


      /*  Initialization of control variables and verification of argument 
          list                                                              */

      found = done = hold = FALSE;
      if (_fstrlen (menuString) == 0)
         return (MN_ERR_EMPTY);

      if (option > MAX_OPTIONS)
         return (MN_ERR_OPTION);


      /*  Allocation and verification of menu node  */

      branch = ((MENU_TYPE far *) farcalloc (1, sizeof (MENU_TYPE)));
      if (branch == NULL)
         return (MN_ERR_ALLOC);


      /*  Find the parent menu  */

      if (menuId == M_MAIN)
         {
            rootTree = branch;
            _fstrcpy(branch->title, "ROOT");
         }

      else
         {
            branch = FindMenu (parent, rootTree, &found, &i, &hold, MN_CHILD);
            if (option > branch->num - 1)
               return (MN_ERR_OPTION);

            branch = branch->next [option];
         }


      /*  Initialize menu structure elements for parent menu  */

      branch->menu_id = menuId;
      branch->boundary.upper_left.x = NONE;
      branch->boundary.upper_left.y = NONE;
      branch->boundary.lower_right.x = NONE;
      branch->boundary.lower_right.y = NONE;
      branch->assigned = FALSE;
      branch->active = FALSE;
      branch->num = NONE;
      branch->disabled = FALSE;
      branch->display = OFF;
      for (i = 0; i < MAX_OPTIONS; ++i)
         branch->next [i] = NULL;

      while (!done)
         {
            /*  Allocate and verify memory for each child menu node */

            newbranch = ((MENU_TYPE far *) farcalloc (1, sizeof (MENU_TYPE)));
            if (newbranch == NULL)
               return (MN_ERR_NEWALLOC);


            /*  Assign valid name and state for each child menu  */

            newbranch->selected = ParseString (menuString, newbranch->title);
            newbranch->menu_id = MN_EMPTY;
            newbranch->boundary.upper_left.x = NONE;
            newbranch->boundary.upper_left.y = NONE;
            newbranch->boundary.lower_right.x = NONE;
            newbranch->boundary.lower_right.y = NONE;
            newbranch->assigned = FALSE;
            newbranch->active = FALSE;
            newbranch->disabled = FALSE;
            newbranch->display = OFF;
            newbranch->num = NONE;
            for (i = 0; i < MAX_OPTIONS; ++i)
               newbranch->next [i] = NULL;


            /*  Link the tree  */

            branch->next [branch->num] = newbranch;
            ++branch->num;


            /*  Check for completion of branch or overflow  */

            if (branch->num == MAX_OPTIONS || _fstrlen(menuString) == NONE)
               done = TRUE;

         }

      return (MN_VALID);
   }



/*----------------------------------------------------------------------------

         This function displays the menu specified and assigns an active
   mouse selection region to each menu option.  The main menu is drawn
   horizontally and sub menus are drawn vertically.  Menus are drawn
   proportionally to the current display mode characteristics.

----------------------------------------------------------------------------*/

int mnDispMenu (menuId, menuColor, textColor, disColor, selColor)
int      menuId, menuColor, textColor, disColor, selColor;
   {
      MENU_TYPE      far *branch, far *node, far *parent;
      int            i, adjust, tHeight, height, width, spacing, colorMode, hBarPos;
      int            result = gxSUCCESS, offSet, vBarPos, hFrom, hTo, vFrom, vTo, hold = FALSE;
      int            x1, y1, x2, y2, charCt, maxTitle, mainPos, valid, found = FALSE;


      /*  Determine menu height based upon video mode  */

      if (disp_info.vres == V_CGA)
         height = (int) (V_EXT_RATIO * disp_info.vres);

      else
         height = (int) (V_RATIO * disp_info.vres);

      if (menuId == M_MAIN)
         {
            /*  Draw the main menu bar  */

            branch = rootTree;
            if (branch->display == OFF || !branch->active)
               {
                  /*  Valid state for main menu draw  */

                  /*  Calculate text spacing variables  */

                  width = disp_info.hres - 1;
                  TitleInfo (branch, &charCt, &maxTitle);
                  spacing = (int) (((disp_info.hres - (charCt * CHAR_WIDTH)) - (2 * LEFT_OPT)) / (branch->num - 1));
                  if (spacing > MAX_SPACE)
                     spacing = MAX_SPACE;

                  vBarPos = DrawMenuBox (height, menuColor, X_ORIGIN, Y_ORIGIN, width, height);
                  hBarPos = LEFT_OPT;


                  /*  Display each menu option  */

                  for (i = 0; i < branch->num; ++i)
                     {
                        node = branch->next [i];
                        SetTextColor (textColor, disColor, node->disabled);
                        if (_fstrcmp (node->title, "_"))
                           {
                              grMoveTo (hBarPos + (2 * X_SHADE), vBarPos + 1);
                              grOutText (node->title);
                           }

                        else
                           return (MN_ERR_SEPLINE);


                        /*  Assign valid mouse selection regions to each option  */

                        if (!node->assigned)
                           {
                              node->assigned = TRUE;
                              node->boundary.upper_left.x = hBarPos - CHAR_WIDTH;
                              node->boundary.upper_left.y = 0;
                              hBarPos = hBarPos + (_fstrlen(node->title) * CHAR_WIDTH) + spacing;
                              if (hBarPos > width)
                                 hBarPos = width;

                              node->boundary.lower_right.x = hBarPos - CHAR_WIDTH;
                              node->boundary.lower_right.y = height;
                           }

                        else
                           {
                              hBarPos = hBarPos + (_fstrlen(node->title) * CHAR_WIDTH) + spacing;
                              if (hBarPos > width)
                                 hBarPos = width;

                           }

                     }


                  /*  Set the current menu state  */

                  branch->display = ON;
                  branch->active = TRUE;
               }

            else
               return (MN_ERR_DISPMENU);

         }

      else
         {
            /*  Display a vertically oriented submenu  */

            /*  Find the parent menu  */

            branch = FindMenu (menuId, rootTree, &found, &mainPos, &hold, MN_CHILD);
            if (branch->display == OFF || !branch->active)
               {
                  /*  A valid menu state exists  */

                  /*  Get the position to display this submenu relative to
                      the parent menu                                       */

                  valid = GetMenuPos (branch, mainPos, &hFrom, &width, &vFrom);
                  if (valid != MN_VALID)
                     return (MN_ERR_DISPMISS);


                  /*  Calculate text positioning variables and draw the menu 
                      box                                                   */

                  tHeight = height * branch->num;
                  hTo = hFrom + width;
                  vTo = vFrom + tHeight;
                  adjust = DrawMenuBox (height, menuColor, hFrom, vFrom, hTo, vTo);
                  vBarPos = vFrom;


                  /*  For each submenu option, display text and assign mouse
                      selection regions                                     */

                  for (i = 0; i < branch->num; ++i)
                     {
                        node = branch->next [i];
                        SetTextColor (textColor, disColor, node->disabled);
                        if (!node->assigned)
                           {
                              node->assigned = TRUE;
                              node->boundary.upper_left.x = hFrom;
                              node->boundary.lower_right.x = hTo;
                              node->boundary.upper_left.y = vBarPos;
                              node->boundary.lower_right.y = vBarPos + height;
                           }


                        /*  Output option text or a menu seperator line  */

                        if (_fstrcmp (node->title, "_"))
                           {
                              if (node->selected)
                                 {
                                    x1 = (hFrom + CHAR_WIDTH) - X_SHADE;
                                    y1 = vBarPos + Y_SHADE + adjust + Y_SHADE;
                                    x2 = x1 + SEL_SIZE;
                                    y2 = y1 + SEL_SIZE;
                                    result = grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                                    result = grDrawRect (x1 - SEL_SHADE, y1 - SEL_SHADE, x2 - SEL_SHADE, y2 - SEL_SHADE, grFILL);
                                    result = grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                                    result = grDrawRect (x1 + SEL_SHADE, y1 + SEL_SHADE, x2 + SEL_SHADE, y2 + SEL_SHADE, grFILL);
                                    result = grSetFillStyle (grFSOLID, selColor, grOPAQUE);
                                    result = grDrawRect (x1, y1, x2, y2, grFILL);
                                 }

                              grMoveTo (hFrom + (2 * CHAR_WIDTH), vBarPos + Y_SHADE + adjust);
                              grOutText (node->title);
                           }

                        else
                           {
                              /*  Draw a menu seperator bar  */

                              grSetLineStyle (grLSOLID, 1);
                              grSetColor (textColor);
                              offSet = (int) (height / 2) - 1;
                              grSetColor (grDARKGRAY);
                              grDrawLine (hFrom, vBarPos + offSet + adjust, hTo, vBarPos + offSet + adjust);
                              grSetColor (grWHITE);
                              ++offSet;
                              grDrawLine (hFrom, vBarPos + offSet + adjust, hTo, vBarPos + offSet + adjust);
                              node->disabled = TRUE;
                           }

                        vBarPos += height;
                     }


                  /*  Set the state of the parent menu and the offsping menus  */

                  found = FALSE;
                  parent = FindMenu (menuId, rootTree, &found, &i, &hold, MN_PARENT);
                  parent->active = FALSE;
                  branch->display = ON;
                  branch->active = TRUE;
               }

            else
               return (MN_ERR_DISPMENU);

         }

      if (result == gxSUCCESS)
         return (MN_VALID);

      else
         return (MN_ERR_DISPMENU);

   }



/*----------------------------------------------------------------------------

      This function refreshes a menu without redrawing the menu box or
   affecting the state of the menu tree structure.  The menu id is passed
   as a parameter and a success or error code is returned.

----------------------------------------------------------------------------*/

int mnUpdateMenu (menuId)
int      menuId;
   {
      MENU_TYPE      far *branch, far *node;
      int            adjust, found, mainPos, i, width, height, spacing, hBarPos;
      int            result = gxSUCCESS, hold, hFrom, vBarPos, charCt, maxTitle;


      /*  Determine initial spacing based upon video mode  */

      if (disp_info.vres >= V_VGA)
         {
            grSetTextStyle (grTXT8X16, grTRANS);
            height = (int) (V_RATIO * disp_info.vres);
            adjust = vBarPos = ((height - VGA_BOX) / 2);
         }

      else if (disp_info.vres >= V_EGA)
              {
                 grSetTextStyle (grTXT8X14, grTRANS);
                 height = (int) (V_RATIO * disp_info.vres);
                 adjust = vBarPos = ((height - EGA_BOX) / 2);
               }

           else
              {
                 grSetTextStyle (grTXT8X8, grTRANS);
                 height = (int) (V_EXT_RATIO * disp_info.vres);
                 adjust = vBarPos = ((height - CGA_BOX) / 2);
              }

      if (menuId == M_MAIN)
         {
            branch = rootTree;
            width = disp_info.hres - 1;
            TitleInfo (branch, &charCt, &maxTitle);
            spacing = (int) (((disp_info.hres - (charCt * CHAR_WIDTH)) - (2 * LEFT_OPT)) / (branch->num - 1));
            if (spacing > MAX_SPACE)
               spacing = MAX_SPACE;

            hBarPos = LEFT_OPT;


            /*  For each option display the title  */

            for (i = 0; i < branch->num; ++i)
               {
                  node = branch->next [i];
                  SetTextColor (grBLACK, grWHITE, node->disabled);
                  if (_fstrcmp (node->title, "_"))
                     {
                        grMoveTo (hBarPos + (2 * X_SHADE), vBarPos + 1);
                        grOutText (node->title);
                     }

                  else
                     return (MN_ERR_SEPLINE);


                  /*  Adjust the text position  */

                  hBarPos = hBarPos + (_fstrlen(node->title) * CHAR_WIDTH) + spacing;
                  if (hBarPos > width)
                     hBarPos = width;

               }

         }

      else
         {
            branch = FindMenu (menuId, rootTree, &found, &mainPos, &hold, MN_CHILD);
            result = GetMenuPos (branch, mainPos, &hFrom, &width, &vBarPos);
            if (result != MN_VALID)
               return (MN_ERR_UPDMENU);


            /*  For each submenu option, display text and assign mouse 
                selection regions                                           */

            for (i = 0; i < branch->num; ++i)
               {
                  node = branch->next [i];
                  SetTextColor (grBLACK, grWHITE, node->disabled);


                  /*  Output option text  */

                  if (_fstrcmp (node->title, "_"))
                     {
                        grMoveTo (hFrom + (2 * CHAR_WIDTH), vBarPos + Y_SHADE + adjust);
                        grOutText (node->title);
                     }


                  /*  Adjust the text position  */

                  vBarPos += height;
               }

         }

      if (result == gxSUCCESS)
         return (MN_VALID);

      else
         return (MN_ERR_UPDMENU);

   }




/*----------------------------------------------------------------------------

      This function erases an actively displayed menu.  The argument list 
   consists of a menu id to be cleared and a clear screen boolean which
   "blacks out" the previously drawn menu.  A success or error code is
   returned.

----------------------------------------------------------------------------*/

int mnClearMenu (menuId, clrScreen)
int    menuId, clrScreen;
   {
      MENU_TYPE      far *parent, far *branch, far *node;
      int            i, tHeight, height, width, spacing, colorMode, hBarPos;
      int            mainPos, valid, vBarPos, found, hFrom, vFrom, hTo, vTo;
      int            hold;


      /*  Determine the menu bar height  */

      if (disp_info.vres == V_CGA)
         height = (int) (V_EXT_RATIO * disp_info.vres);

      else
         height = (int) (V_RATIO * disp_info.vres);

      hold = found = FALSE;
      if (menuId == M_MAIN)
         {
            /*  Clear the main menu bar  */

            width = disp_info.hres - 1;
            if (clrScreen)
               {
                  grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
                  grDrawRect (X_ORIGIN, Y_ORIGIN, width, height, grFILL); 
               }

            branch = FindMenu (menuId, rootTree, &found, &mainPos, &hold, MN_CHILD);
            branch->display = OFF;
         }

      else
         {
            /*  Traverse the menu tree to the correct menu node  */
         
            branch = FindMenu (menuId, rootTree, &found, &mainPos, &hold, MN_CHILD);
            if (branch->display == ON && branch->menu_id == menuId)
               {
                  found = FALSE;


                  /*  Get the screen coordinates for the current menu  */

                  valid = GetMenuPos (branch, mainPos, &hFrom, &width, &vFrom);
                  if (valid != MN_VALID)
                     return (MN_ERR_CLRMISS);

                  tHeight = height * branch->num;
                  hTo = hFrom + width;
                  vTo = vFrom + tHeight;


                  /*  Clear the screen of the currrent menu  */

                  if (clrScreen)
                     {
                        grSetFillStyle (grFSOLID, grBLACK, grOPAQUE);
                        grDrawRect (hFrom, vFrom, hTo, vTo, grFILL); 
                     }

                  branch->display = OFF;
                  branch->active = FALSE;


                  /*  Find and redisplay the parent menu  */

                  parent = FindMenu (menuId, rootTree, &found, &i, &hold, MN_PARENT);
                  mnDispMenu (parent->menu_id, mnGRAY, mnBLACK, mnWHITE, grGREEN);
               }

            else
               return (MN_ERR_CLRMENU);

         }

      return (MN_VALID);
   }



/*----------------------------------------------------------------------------

      This function enables option selection by putting an indicator
   square next to the selected option.  The function has several 
   arguments.  The menu id and the selected option number are 
   supplied, as well as the selection color and the menu background
   color.  In addition, A menu state variable and a displya status variable
   are passed in the argument list as well.  The function returns a
   success or failure code.

----------------------------------------------------------------------------*/

int mnMarkOpt (menuId, selectColor, backColor, option, select, display)
int   menuId, selectColor, backColor, option, select, display;
   {
      MENU_TYPE      far *branch, far *node;
      int            i, adjust, height, width, hBarPos, vBarPos;
      int            x1, y1, x2, y2, hFrom, vFrom, hold = FALSE;
      int            state, mainPos, valid, result = MN_VALID, found = FALSE;


      /*  Determine whether the mark is on or off  */

      if (select)
         state = TRUE;

      else
         state = FALSE;


      /*  Calculate menu bar height  */

      if (disp_info.vres == V_CGA)
         height = (int) (V_EXT_RATIO * disp_info.vres);

      else
         height = (int) (V_RATIO * disp_info.vres);


      /*  Find the menu and determine a valid state  */

      branch = FindMenu (menuId, rootTree, &found, &mainPos, &hold, MN_CHILD);
      if ((branch->display == ON && branch->active) || !display)
         {
            /*  Determine the correct menu position  */

            valid = GetMenuPos (branch, mainPos, &hFrom, &width, &vFrom);
            if (valid != MN_VALID)
               return (MN_ERR_DISPMISS);


            /*  Calculate positioning values for menu indicator  */

            if (disp_info.vres >= V_VGA)
               adjust = (height - VGA_BOX) / 2;

            else if (disp_info.vres >= V_EGA)
                    adjust = (height - EGA_BOX) / 2;

                 else
                    adjust = (height - CGA_BOX) / 2;

            vBarPos = vFrom;

            
            /*  Check each option of the vertically oriented menu  */

            for (i = 0; i < branch->num; ++i)
               {
                  node = branch->next [i];
                  if (_fstrcmp (node->title, "_") && option == i)
                     {
                        node->selected = state;
                        if (display)
                           {
                              /*  Mark sizing varibales  */

                              x1 = (hFrom + CHAR_WIDTH) - X_SHADE;
                              y1 = vBarPos + Y_SHADE + adjust + Y_SHADE;
                              x2 = x1 + SEL_SIZE;
                              y2 = y1 + SEL_SIZE;
                              if (state)
                                 {
                                    /*  Show the mark  */

                                    result = grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
                                    result = grDrawRect (x1 - SEL_SHADE, y1 - SEL_SHADE, x2 - SEL_SHADE, y2 - SEL_SHADE, grFILL);
                                    result = grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
                                    result = grDrawRect (x1 + SEL_SHADE, y1 + SEL_SHADE, x2 + SEL_SHADE, y2 + SEL_SHADE, grFILL);
                                    result = grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
                                    result = grDrawRect (x1, y1, x2, y2, grFILL);
                                 }

                              else
                                 {
                                    /*  Hide the mark  */

                                    result = grSetFillStyle (grFSOLID, backColor, grOPAQUE);
                                    result = grDrawRect (x1 - SEL_SHADE, y1 - SEL_SHADE, x2 + SEL_SHADE, y2 + SEL_SHADE, grFILL);
                                 }

                           }

                     }


                  /*  Check the next menu option  */

                  vBarPos += height;
               }

         }

      return (result);
   }



/*----------------------------------------------------------------------------

      This function enables or disables a menu option.  The argument list
   consists of the menu id, the option number and an enable/disable
   toggle variable.  The function returns a success or error code.

----------------------------------------------------------------------------*/

int mnEnableOpt (menuId, option, enable)
int   menuId, option, enable;
   {
      MENU_TYPE   far *node, far *branch;
      int         temp, found, hold;


      /*  Find the menu and corresponding option  */

      hold = found = FALSE;
      branch = FindMenu (menuId, rootTree, &found, &temp, &hold, MN_CHILD);
      node = branch->next [option];
      if (node == NULL)
         return (MN_ERR_ENABLE);

      else
         {
            /*  Assign the requested option state  */

            if (enable)
               node->disabled = FALSE;

            else
               node->disabled = TRUE;

            return (MN_VALID);
         }

   }


/*----------------------------------------------------------------------------

      This function takes coordinate values supplied by the mouse tracking
   routines and routines the option number of the current menu based upon
   predefined menu regions.  These regions are defined as part of the
   menu tree structure and are assigned durinig the initial menu 
   definition.  An integer value greater to or equal to zero is returned
   if a valid option was selected or a negative value (no option) is
   returned if a valid option was not selected.

----------------------------------------------------------------------------*/

int mnSelectOpt (xPos, yPos, menuId)
int              xPos, yPos, menuId;
   {
      MENU_TYPE   far *branch, far *node;
      int         i, found, hold;


      /*  Find the current menu  */

      hold = found = FALSE;
      branch = FindMenu (menuId, rootTree, &found, &i, &hold, MN_CHILD);
      i = 0;
      found = NO_OPTION;
      while (i < branch->num && (found == NO_OPTION))
         {
            /*  Assign the corresponding menu node  */

            node = branch->next [i];


            /*  Check to see if the mouse click occured in this node's 
                predefined region                                          */

            if (xPos >= node->boundary.upper_left.x &&
                xPos <= node->boundary.lower_right.x &&
                yPos >= node->boundary.upper_left.y &&
                yPos <= node->boundary.lower_right.y)
               if (!node->disabled)
                  found = i;

               else
                  found = DIS_OPTION;


            /*  On to the next node  */

            ++i;
         }

      return (found);
   }




/*----------------------------------------------------------------------------

      This function simulates the animated pressing and release of a menu 
   option.  The function takes a menu id and an option to determine the
   correct menu item.  In addition, the menu color and text color are
   supplied.  Finally, the last argument can take on any one of three
   values: push, flush and pop.  The push value will simulate the 
   the depressing of a menu option.  Both the flush and pop values 
   indicate a deselection of the option, with the pop value simulating
   an elevated condition of the menu option.

----------------------------------------------------------------------------*/

int mnChooseOpt (menuId, option, selectColor, textColor, pushed)
int              menuId, option, selectColor, textColor, pushed;
   {
      MENU_TYPE   far *branch, far *node;
      int         i, found, hold, uplefty, lowrighty, errorCond;


      /*  Disabled option means no further action  */

      if (option == DIS_OPTION)
         return (MN_VALID);


      /*  An invalid menu option has been passed  */

      if (option < NONE || option >= MAX_OPTIONS)
         return (MN_ERR_OPTION);


      /*  Find the current sub-menu node  */

      hold = found = FALSE;
      branch = FindMenu (menuId, rootTree, &found, &i, &hold, MN_CHILD);


      /*  Sub-menu not found  */

      if (branch == NULL)
         return (MN_ERR_CHOICE);

      node = branch->next [option];


      /*  Look for a push or pop by drawing the upper left edge of the menu
          option                                                            */

      if (pushed != DOWN)
         grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);

      else
         grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);

      grDrawRect (node->boundary.upper_left.x, node->boundary.upper_left.y,
                  node->boundary.lower_right.x, node->boundary.lower_right.y, grFILL);


      /*  Look for a push or pop by drawing the lower right edge of the 
          menu option                                                       */

      if (pushed != DOWN)
         grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);

      else
         grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);

      if (!pushed)
         grDrawRect (node->boundary.upper_left.x + X_SHADE, node->boundary.upper_left.y,
                     node->boundary.lower_right.x, node->boundary.lower_right.y, grFILL);

      else
         grDrawRect (node->boundary.upper_left.x + X_SHADE, node->boundary.upper_left.y + Y_SHADE,
                     node->boundary.lower_right.x, node->boundary.lower_right.y, grFILL);


      /*  Look for a flushed option  */

      grSetFillStyle (grFSOLID, selectColor, grOPAQUE);
      if (!pushed)
         {
            if (option == 0)
               uplefty = node->boundary.upper_left.y + Y_SHADE;

            else
               uplefty = node->boundary.upper_left.y;

            if (option == branch->num - 1)
               lowrighty = node->boundary.lower_right.y - Y_SHADE;

            else
               lowrighty = node->boundary.lower_right.y;

            grDrawRect (node->boundary.upper_left.x + X_SHADE, uplefty,
                        node->boundary.lower_right.x - X_SHADE, lowrighty, grFILL);
         }

      else
         grDrawRect (node->boundary.upper_left.x + X_SHADE, node->boundary.upper_left.y + Y_SHADE,
                     node->boundary.lower_right.x - X_SHADE, node->boundary.lower_right.y - Y_SHADE, grFILL);


      /*  Set the text style based upon video mode  */

      if (disp_info.vres >= V_VGA)
         grSetTextStyle (grTXT8X16, grTRANS);

      else if (disp_info.vres >= V_EGA)
              grSetTextStyle (grTXT8X14, grTRANS);

           else
              grSetTextStyle (grTXT8X8, grTRANS);


      /*  Locate position for text  */

      grSetColor (textColor);
      if (pushed != DOWN)
         grMoveTo (node->boundary.upper_left.x + (2 * CHAR_WIDTH),
                   node->boundary.upper_left.y + Y_SHADE);

      else
         grMoveTo (node->boundary.upper_left.x + (2 * CHAR_WIDTH) + X_SHADE,
                   node->boundary.upper_left.y + (Y_SHADE + 1));

      grOutText (node->title);


      /*  If there was a mark, redisplay it  */

      if (node->selected)
         errorCond = mnMarkOpt (menuId, grGREEN, grGRAY, option, ON, ON);

      else
         errorCond = mnMarkOpt (menuId, grGREEN, grGRAY, option, OFF, ON);

      return (errorCond);
   }




/*----------------------------------------------------------------------------

      This function searches the menu tree recursively on a depth first
   basis and returns the active menu.  The starting node for the tree search
   and a boolean value to control recursion are passed as arguments.  The
   function makes recursive calls with the child of the original node.

----------------------------------------------------------------------------*/

int mnFindActive (node, found)
MENU_TYPE far *node;
int       far *found;
   {
      int         active, i = NONE;

      if (node->active)
         {
            /*  Found the active node, return the menu id  */

            *found = TRUE;
            return (node->menu_id);
         }

      else
         {
            /*  Look at each child node or until we find an active menu  */

            i = 0;
            while (i < node->num && !*found)
               active = mnFindActive (node->next [i++], found);

         }


      /*  All Done!  Return the menu id or no menu found  */

      if (*found)
         return (active);

      else
         return (NONE);

   }



/*----------------------------------------------------------------------------

      This function opens and reads a file and builds a valid menu string.
   Both the file name for reading and the menu string are supplied as 
   arguments.  The function returns a success or error code.

----------------------------------------------------------------------------*/

int mnReadMenu (menuString, fileName)
char     far *menuString, far *fileName;
   {
      FILE     far *fileImage;
      char     far *temp;
      int      empty = FALSE;


      /*  Allocation and verification of string variable  */

      temp = (char far *) farcalloc (80, sizeof (char));
      if (temp == NULL)
         return (MN_ERR_ALLOC);

      if (_fstrlen (menuString) == NONE)
         empty = TRUE;


      /*  Open and verify the file  */

      fileImage = fopen (fileName, "r");
      if (fileImage == NULL)
         return (MN_ERR_READ);


      /*  Read each file record and build the menu string  */

      while (!feof (fileImage))
         {
            fscanf (fileImage, "%s\n", temp);
            _fstrcat (menuString, ",");
            _fstrcat (menuString, temp);
         }

      if (empty)
         _fstrcpy (menuString, menuString + 1);


      /*  House Cleaning  */

      fclose (fileImage);
      farfree (temp);
      return (MN_VALID);
   }



/*----------------------------------------------------------------------------

      This function recursively erases the complete menu structure by
   doing a depth first search on the menu tree for terminal nodes.  

----------------------------------------------------------------------------*/

int mnEraseMenu (node)
MENU_TYPE   far *node;
   {
      int         found, active, i = NONE;


      /*  Found a terminal node  */

      if (node->menu_id == MN_EMPTY)
         farfree (node);

      else
         {
            /*  Traverse the tree  */

            i = 0;
            while (i < node->num && !found)
               mnEraseMenu (node->next [i++]);


            /*  Free the root node  */

            farfree (node);
         }

      return (MN_VALID);
   }



/*----------------------------------------------------------------------------

      This procedure determines the length of the current menu option string
   and the largest option string for the specified menu.  This is useful for
   calculating horizontal menu sizes.

----------------------------------------------------------------------------*/

void  TitleInfo (menu, length, max)
int          far *length, far *max;
MENU_TYPE    far *menu;
   {
      int      i;


      *length = *max = NONE;
      for (i = 0; i < menu->num; ++i)
         {
            /*  Find the largest title  */

            if (_fstrlen (menu->next [i]->title) > *max)
               *max = _fstrlen (menu->next [i]->title);

            *length += _fstrlen (menu->next [i]->title);
         }

      return;
   }


/*----------------------------------------------------------------------------

      This function locates the screen coordinates for specified menu.
   The first argument is menu pointer. The next argument is the pre-
   determined menu ancestor.  The final three arguments are vertical
   and horizontal menu positions which are modified in the function
   body.  Finally, the function returns a success or error code.   

----------------------------------------------------------------------------*/

int GetMenuPos (node, ancestor, hstart, hsize, vstart)
MENU_TYPE      far *node;
int            ancestor;
int            far *hstart, far *hsize, far *vstart;
   {
      MENU_TYPE      far *parent;
      int            i, charCt, maxTitle, offspring;


      /*  Find the ancestor node to determine the offset position from
          the main menu                                                   */

      offspring = FALSE;
      for (i = 0; i < rootTree->num; ++i)
         if (rootTree->next [i] == node)
            offspring = TRUE;

      if (offspring)
         {
            /*  Determine horizontal size from the longest menu title and
                vertical size from the number of options                 */

            TitleInfo (rootTree, &charCt, &maxTitle);
            *hstart = rootTree->next[ancestor]->boundary.upper_left.x;
            *hsize = (maxTitle * CHAR_WIDTH) + MENU_SPACE;
            if (*hstart + *hsize > disp_info.hres - 1)
               *hstart = (disp_info.hres - 1) - *hsize;

            *vstart = rootTree->next[ancestor]->boundary.lower_right.y + VERT_MENU;
         }

      else
         {
            /*  Not a direct child of the main menu  */

            if (node->assigned)
               {
                  /*  Determine horizontal size from the longest menu
                      title and vertical size from the number of option  */

                  TitleInfo (node, &charCt, &maxTitle);
                  *hstart = node->boundary.lower_right.x + HORIZ_MENU;
                  *hsize = (maxTitle * CHAR_WIDTH) + MENU_SPACE;
                  if (*hstart + *hsize > disp_info.hres - 1)
                     *hstart = (disp_info.hres - 1) - *hsize;

                  *vstart = node->boundary.upper_left.y;
               }

            else
               return (MN_ERR_DISORD);

         }

      return (MN_VALID);
   }



/*----------------------------------------------------------------------------

      This function extracts the title from the given menu string and 
   modifies the menu string.  A true or false value is returned indicating
   that the corresponding menu option "selected" attribute is to initialized
   to the returned value.  A special character at the beginning of the menu
   string title indicates an "ON" status.

----------------------------------------------------------------------------*/

int ParseString (menuString, title)
char  far *menuString, far *title;
   {
      char   far *cdr;
      int    menuSize, cdrSize;


      /*  Assign the title string and the remainder of the menu string  */

      cdr = _fstrchr (menuString, ',');
      cdrSize = _fstrlen(cdr);
      menuSize = _fstrlen (menuString);
      _fstrcpy (title, ""); 
      if (cdrSize == 0)
         {
            _fstrcpy (title, menuString);
            _fstrcpy (menuString, "");
         }

      else
         {
            /*  The last title in this menu string  */

            _fstrncpy (title, menuString, menuSize - cdrSize);
            _fstrcpy (menuString, (cdr) + 1); 
         }


      /*  The selected field of the menu structure is set ON or OFF  */

      if (title [0] == '!')
        {
          _fstrcpy (title, (title) + 1);
          return (TRUE);
        }

      else
        return (FALSE);

   }



/*----------------------------------------------------------------------------

      This function returns the number of occurences of a character within
   a specified string.  Both the string and the character are passed as
   parameters.  

----------------------------------------------------------------------------*/

int Strct (srchString, chr)
char     far *srchString, chr;
   {
      char      far *target, far *temp;
      int       strSize, i, count = NONE;


      /*  Allocation and verification of string variables  */

      target = (char far *) farcalloc (LARGE_STR, sizeof (char));
      temp = (char far *) farcalloc (LARGE_STR, sizeof (char));
      if (target == NULL || temp == NULL)
         return (MN_ERR_ALLOC);


      /*  Begin counting  */

      strSize = _fstrlen (srchString);
      for (i = 0; i < strSize; ++i)
         {
            if (srchString [i] == chr)
               ++count;

         }


      /*  House cleaning  */

      farfree (target);
      farfree (temp);
      return (count);
   }



/*----------------------------------------------------------------------------

      This function locates a menu node based upon requested options 
   supplied by the calling routine.  The first argument is the menu id
   to be searched for.  The second argument is the root of the menu tree.
   the third argument is a boolean indicating completion of the recursive
   process.  The ancestor argument is used to determine which main menu
   option a menu descendent is related to.  The placehol variable is a
   boolean which maintains the menu pointer throughout the recursion.
   Finally, the type argument is used to determine whether the target or
   the target's parent menu pointer will be returned.  It should be noted
   that this recursive process is also a depth first search of the menu
   tree.


----------------------------------------------------------------------------*/

MENU_TYPE far * FindMenu (menu, node, found, ancestor, placehold, type)
int         menu;
MENU_TYPE   far *node;
int         far *found, far *ancestor, far *placehold;
int         type;
   {
      MENU_TYPE   far *temp_node;
      int         i = NONE;


      /*  The node is found, return a pointer  */

      if (node->menu_id == menu)
         {
            *found = TRUE;
            return (node);
         }

      else
         {
            /*  Start depth-first menu tree traversal  */

            i = 0;
            while (i < node->num && !*found)
               {
                  temp_node = FindMenu (menu, node->next [i], found, ancestor, placehold, type);


                  /*  Save the node address if we have a match!  */

                  if (*found && !*placehold && type == MN_PARENT)
                     {
                        temp_node = node;
                        *placehold = TRUE;
                     }

                  ++i;
               }

         }


      /*  All done, no more recursion, assign the ancestor and return the 
          pointer                                                          */

      *ancestor = i - 1;
      if (*found)
         return (temp_node);

      else
         return (NULL);

   }




/*----------------------------------------------------------------------------

      This function draws the menu region.  The menu height and color are
   passed to the function with the upper left and lower right menu box
   corners.  The function draws the appropriate sized and positioned menu
   box with a three dimensioned effect.  The function returns an adjust
   ment value based upon the menu height for subsequent text positioning.

----------------------------------------------------------------------------*/

int  DrawMenuBox (height, menuColor, x1, y1, x2, y2)
int      height, menuColor, x1, y1, x2, y2;
   {
      /*  Draw the menu box  */

      grSetFillStyle (grFSOLID, grWHITE, grOPAQUE);
      grDrawRect (x1, y1, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, grDARKGRAY, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2, y2, grFILL);
      grSetFillStyle (grFSOLID, menuColor, grOPAQUE);
      grDrawRect (x1 + X_SHADE, y1 + Y_SHADE, x2 - X_SHADE, y2 - Y_SHADE, grFILL);


      /*  Determine the adjustment based upon video mode  */

      if (disp_info.vres >= V_VGA)
         {
            grSetTextStyle (grTXT8X16, grTRANS);
            return ((height - VGA_BOX) / 2);
         }

      else if (disp_info.vres >= V_EGA)
              {
                 grSetTextStyle (grTXT8X14, grTRANS);
                 return ((height - EGA_BOX) / 2);
              }

           else
              {
                 grSetTextStyle (grTXT8X8, grTRANS);
                 return ((height - CGA_BOX) / 2);
              }

   }



/*----------------------------------------------------------------------------

      This procedure sets the appropriate text color for enabled or 
   disabled menu options.  The two text color choices are supplied, along
   with a boolean variable for determining disabled status.

----------------------------------------------------------------------------*/

void SetTextColor (textColor, disColor, disabled)
int      textColor, disColor, disabled;
   {
      if (!disabled)
         grSetColor (textColor);

      else
         grSetColor (disColor);

      return;
   }
