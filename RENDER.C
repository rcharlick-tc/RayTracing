/*----------------------------------------------------------------------------

Project:                Fast Image Rendering for Ray Tracing Algebraic
                        Surfaces Using a Fibonacci Based Psuedo-Random Number
                        Generator.

Course:                 ICSG 891, Masters Project
Author:                 Randall W. Charlick
Project Supervisor:     Dr. Peter G. Anderson

File:                   Render.c
Date Started:           March 31, 1992
Last Modified           October 17, 1992

Source Files:           Render.c, Menu.c, Display.c, Surface.c, Eval.c, 
                        Select.c
Include Files:          Render.h, Globals.h, Surface.h, Display.h, Menu.h, 
                        Select.h, Eval.h, Errmsg.h
Data Files:             Image.mnu, Render.cfg, *.img, *.pcx, *.cfg
Documentation:          Render.doc, Read.me, Data.dct, Packing.lst
Executables:            Render.exe
Batch Files:            Scenes.bat, Show.bat

Libraries:              Borland C++, Genus Microprogramming.

System
Requirements:
                        This project was written on an Intel 80386 based
                        system running under MS-DOS 5.0.  Although not
                        required, it is highly recommended that this
                        executable be run on an 80386 machine.  A math
                        coprocessor will substantially increase performance,
                        but is not required.  Since this application runs
                        exclusively in 256 color mode, a "super" VGA
                        graphics card and monitor are required.  Minimum
                        acceptable resolution is 640 x 400.  The software
                        will support common graphics adapters from 
                        Paradise, ATI and most Tseng chip set based boards.


Compiling
Instructions:
                        This project has been compiled under Borland C++
                        versions 2.0 and 3.0.  The above source files need to
                        be added to the project file in addition to two
                        Genus graphics libraries, GX_CL.LIB and GR_CL.LIB.
                        The project should be compiled using the large memory
                        model.

Description:

         This program was written as a Master's Project for Dr. Peter Anderson
      at the Rochester Institute of Technology.  This project consists of a
      scene rendering package that displays algebraic surfaces using ray
      tracing and a variety of display enhancing techniques.  These techniques
      are not methods for increasing throughput of the image display, but are
      instead used for making scenes more readily recognizable to the user.
      This project is not intended to be a flexible application for graphics
      programmers, but rather to demonstrate the effectiveness of these 
      techniques and to provide for a comparative anylsis.

         Information on the theory of operation as well as an in depth
      discussion on data flow and data structures applicable to this project
      can be found in the file RENDER.DOC.  Directions for running this
      package, including a detailed decription of command line arguments and
      menu options, can be found in the READ.ME file.

         This project was written using two library sets.  The first were the
      standard libraries supplied with the Borland C++ compiler.  The second 
      library was supplied by genus microprogramming, which supports high
      resolution graphics beyond the standard VGA graphics supplied by 
      Borland.

         This file is a collection of generic main routine support functions 
      and procedures.  This file contains an error handling routine, menu 
      definition function, configuration procedure, help messages and
      command line argument assignment.  The functions in this file
      determine which of the two modes, command or menu the package will
      run under.  The program global variables are initialized in this
      file.  

----------------------------------------------------------------------------*/



/*    Standard and Graphics Include files                                   */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>
#include <io.h>
#include <string.h>
#include <gxlib.h>
#include <grlib.h>
#include <pcxlib.h>


/*  Project include files                                                   */

#include "render.h"
#include "eval.h"
#include "surface.h"
#include "globals.h"
#include "display.h"
#include "menu.h"
#include "errmsg.h"



/*    Internal function prototypes                                          */

void  ErrMsg (int);
int   GetConfig (char far *);
void  HelpMsg (void);
int   CreateMenu (void);
int   Initialize (int far *, int, int);
int   GetArgs (int, char far *[], int far *, int far *, int far *, int far *, char far *,
               char far *, char far *, char far *, char far *);



/*    External function prototypes                                          */

extern void   StatusText (int);
extern int    AssignPalette (int);
extern int    MenuChoice (int, int);
extern void   DrawSep (int, int, int);
extern void   SetRegion (int, int, int);
extern int    DisplayStatus (int, int, int);
extern IMAGE_TYPE far * AssignImage (char far *);
extern int    ShowStatus (int, int, char *, char *, int, int, int);
extern int    Driver (IMAGE_TYPE far *, IMAGE_TYPE far *, int, int, int, int, 
                      int, COORD_TYPE, short);



/*    Global variables                                                      */

GXDINFO         disp_info;
GXHEADER        pcxPtr;
MENU_TYPE       far *rootTree;
POINT_TYPE      statPos [MAX_FIELD];
FILE            far *filein, far *fileout;
REGION_TYPE     leftRegion, rightRegion;
COORD_TYPE      sun = {SUN_XPOS, SUN_YPOS, SUN_ZPOS}, planeRotation = {NONE, NONE, NONE};
int             checkRoot = FALSE, saturate = SAT_ANGLE, backColor = grGRAY,
                imageNumber;


/*---------------------------------------------------------------------------

      The primary task of this main line is to determine the operating
   mode for scene rendition, either menu mode or command line mode. This
   module can initiate a help screen display or display a variety of error
   messages.

---------------------------------------------------------------------------*/

void main (argc, argv)
int    argc;
char   far *argv[];
   {
      int          mode, result, method, status, drawMode, numColor;
      int          reduction, dtype, video = NO_DISP;
      char         far *leftScene, far *rightScene, far *restoreStr, far *saveStr, far *cfgStr;
      IMAGE_TYPE   far *leftImage, far *rightImage;


      /*  Allocation and verification of string variables  */

      leftScene = (char far *) farcalloc (LARGE_STR, sizeof (char));
      rightScene = (char far *) farcalloc (LARGE_STR, sizeof (char));
      restoreStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      saveStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      cfgStr = (char far *) farcalloc (SMALL_STR, sizeof (char));
      if (leftScene == NULL || rightScene == NULL || restoreStr == NULL || saveStr == NULL || cfgStr == NULL)
         {
            ErrMsg (m_ERR_ALLOC);
            return;
         }


      /*  Get command line arguments  */

      mode = GetArgs (argc, argv, &method, &status, &drawMode, &reduction, leftScene, rightScene, restoreStr, saveStr, cfgStr);
      if (mode != m_HELP)
         result = Initialize (&video, mode, reduction);
      else
         {
            HelpMsg ();
            return;
         }

      if (result != VALID)
         {
            ErrMsg (result);
            return;
         }


      /*  Get video mode display information  */

      result = gxGetDisplayInfo (video, &disp_info);
      if (result != VALID)
         {
            ErrMsg (result);
            return;
         }


      /*  Read RENDER.CFG file for configuration data  */

      result = GetConfig (cfgStr);
      if (result != VALID)
         {
            ErrMsg (m_INV_CONF);
            return;
         }


      /*  Get the number of colors and assign palette  */

      numColor = atoi (_fstrrchr (disp_info.descrip, 'x') + 1);
      result = AssignPalette (numColor);
      if (mode == m_MENU)
         {

            /*  Menu mode selected */

            farfree (leftScene);
            farfree (rightScene);
            result = CreateMenu ();
            if (result == VALID)
               {
                  result = MenuChoice (numColor, video);
                  if (result == VALID)
                     {
                        gxSetMode (gxTEXT);
                        printf ("Normal program termination: ");
                        printf ("Video mode: %d  Res: %s\n", video, 
                                 disp_info.descrip);
                     }
                  else
                     ErrMsg (result);

               }

         }

      else if (mode == m_COMMAND)
              {
                 /*  Command line mode selected */

                 /*  Restore a previously rendered scene if the restire option
                     was selected at the command line.                         */

                 if (_fstrlen (restoreStr) > 0)
                    {
                       /*  Auto detect the video mode used by the PCX file  */

                       dtype = pcxGetFileType (restoreStr);
                       result = gxVerifyDisplayType (dtype);
                       if (result == VALID)
                          {
                             /*  Reset the video mode  */

                             result = gxSetDisplay (dtype);
                             result = gxSetMode (gxGRAPHICS);


                             /*  Get video mode display information  */

                             result = gxGetDisplayInfo (dtype, &disp_info);
                             result = AssignPalette (numColor);
                             result = gxCreateVirtual (gxEMM, &pcxPtr, video, disp_info.hres, disp_info.vres);
                             result = pcxFileVirtual (restoreStr, &pcxPtr, VIRT_X, VIRT_Y);
                             if (result == VALID)
                                result = gxVirtualDisplay (&pcxPtr, VIRT_X, VIRT_Y, VIRT_X, VIRT_Y, disp_info.hres - 1, 
                                         disp_info.vres - 1, PAGE_0);

                             getch ();
                             if (result == VALID)
                                {
                                   gxSetMode (gxTEXT);
                                   printf ("Normal program termination: ");
                                   printf ("Video mode: %d  Res: %s\n", dtype, 
                                            disp_info.descrip);
                                }

                             else
                                ErrMsg (m_ERR_PCX);

                          }

                       else
                          ErrMsg (m_ERR_SUP);

                       result = gxDestroyVirtual (&pcxPtr);
                       return;
                    }


                 leftImage = (IMAGE_TYPE far *) farcalloc (SINGLE, 
                              sizeof (IMAGE_TYPE));
                 rightImage = (IMAGE_TYPE far *) farcalloc (SINGLE, 
                              sizeof (IMAGE_TYPE));
                 if (leftImage == NULL || rightImage == NULL)
                    {
                       ErrMsg (m_ERR_ALLOC);
                       return;
                    }

                 if (status)
                    {
                       status = DisplayStatus (numColor, mnGRAY, drawMode);
                       StatusText (drawMode);
                       SetRegion (drawMode, status, TRUE);
                       result = ShowStatus (drawMode, method, leftScene, 
                                            rightScene, TRUE, saturate, FULL);
                    }

                 else
                    SetRegion (drawMode, status, TRUE);

                 if (drawMode == DUAL)
                    DrawSep (grYELLOW, status, TRUE);

         
                 /*  Create Image lists for appropriate display mode  */

                 if (drawMode == MONO)
                    leftImage = AssignImage (leftScene);

                 else
                    {
                       leftImage = AssignImage (leftScene);
                       rightImage = AssignImage (rightScene);
                    }

                 /*  Begin rendering!  */
                  
                 result = Driver (leftImage, rightImage, method, drawMode,
                                  saturate, backColor, status, sun, TRUE);


                 /*  Save image to PCX file format  */

                 if (_fstrlen (saveStr) > 0)
                    {
                       result = gxCreateVirtual (gxEMM, &pcxPtr, video, disp_info.hres, disp_info.vres);
                       result = gxDisplayVirtual (VIRT_X, VIRT_Y, disp_info.hres - 1, disp_info.vres - 1, PAGE_0, &pcxPtr, 
                                                  VIRT_X, VIRT_Y);
                       result = pcxVirtualFile (&pcxPtr, VIRT_X, VIRT_Y, disp_info.hres, disp_info.vres, saveStr);
                       result = gxDestroyVirtual (&pcxPtr);
                    }

                 else if (result != m_STOP)
                         getch ();

                 if (result == VALID)
                    {
                       gxSetMode (gxTEXT);
                       printf ("Normal program termination: ");
                       printf ("Video mode: %d  Res: %s\n", video,
                                disp_info.descrip);
                    }

                 else if (result == m_STOP)
                         {
                            gxSetMode (gxTEXT);
                            printf ("User terminated program from keyboard.\n\n");
                         }

                      else
                         ErrMsg (result);


                 /*  House Cleaning  */

                 farfree (leftImage);
                 farfree (rightImage);
                 farfree (leftScene);
                 farfree (rightScene);
                 farfree (restoreStr);
                 farfree (saveStr);
                 farfree (cfgStr);
              }

           else
              ErrMsg (mode);

      return;
   }




/*----------------------------------------------------------------------------

      This Function initializes the video to the highest possible display
   mode supported by the hardware in 256 color mode.  In addition, this
   function performs all the required mouse initializations.

----------------------------------------------------------------------------*/

int Initialize (video, mode, reduction)
int    far *video, mode, reduction;
   {
      int   status = VALID, index = START, skip = NONE;
      int   video256 [MODES] = {gxVESA_105, gxVESA_103, gxVESA_101, gxVESA_100,
                                gxTRI_62,   gxTRI_5E,   gxTRI_5D,   gxTRI_5C,
                                gxATI_64,   gxATI_63,   gxATI_62,   gxATI_61,
                                gxV7_6A,    gxV7_69,    gxV7_67,    gxV7_66,
                                gxPAR_60,   gxPAR_5C,   gxPAR_5F,   gxPAR_5E,
                                gxTS_38,    gxTS_30,    gxTS_2E,    gxTS_2D};

      if (*video == NO_DISP || (status != gxSUCCESS))
         while (index < MODES - 1 && skip <= reduction)
            {
               status = gxVerifyDisplayType (video256 [++index]);
               if (status == gxSUCCESS)
                  ++skip;

            }

      *video = video256 [index];
      if (status == gxSUCCESS)
         status = gxSetDisplay (*video);
      else
         return (m_ERR_SETDISP);

      if (status == gxSUCCESS)
         status = gxSetMode (gxGRAPHICS);
      else
         return (m_ERR_SETMODE);


      /*  Initialize and activate mouse.  */

      if (mode == m_MENU)
         {
            if (status == gxSUCCESS)
               status = grInitMouse ();
            else
               return (m_ERR_INITM);

            if (status == gxSUCCESS)
               status = grTrackMouse (grTRACK);
            else
               return (m_ERR_TRACK);

            grDisplayMouse (grSHOW);
         }

      return (gxSUCCESS);
   }



/*----------------------------------------------------------------------------

      This function completely defines the menu tree and displays the main
   menu.  A success or failure code is returned.

----------------------------------------------------------------------------*/

int  CreateMenu (void)
   {
      int      result = MN_VALID;
      char     far *menuString;

      menuString = (char far *) farcalloc (LARGE_STR, sizeof (char));
      if (menuString == NULL)
         return (m_ERR_ALLOC);

      _fstrcpy (menuString, "");
      grDisplayMouse (grHIDE);
      result = mnDefMenu (M_MAIN, ORPHAN, NONE, "Execute,Display,Options,Image");
      if (result == MN_VALID)
         result = mnDefMenu (SUB1, M_MAIN, OPT1, "Run,Info,Clear,_,Save,Restore,_,Quit");

      if (result == MN_VALID)
         result = mnDefMenu (SUB2, M_MAIN, OPT2, "!Single,Dual,_,Select");

      if (result == MN_VALID)
         result = mnDefMenu (SUB3, M_MAIN, OPT3, "Status,Menu Off,_,Technique,!Flood");

      if (result == MN_VALID)
         result = mnDefMenu (SUB4, SUB3, OPT4, "!Iterative,Uniform Scan,Uniform Pixel,Fat Uniform Scan,Fat Uniform Pixel");

      if (result == MN_VALID)
         {
            _fstrcpy (menuString, "");
            result = mnReadMenu (menuString, "image.mnu");
            result = mnDefMenu (SUB5, M_MAIN, OPT4, menuString);
         }

      if (result == MN_VALID)
         result = mnDefMenu (SUB6, SUB2, OPT4, "!Left,Right");

      if (result == MN_VALID)
         result = mnDefMenu (SUB7, SUB4, OPT1, "!Horizontal,Vertical");

      if (result == MN_VALID)
         result = mnDefMenu (SUB8, SUB4, OPT2, "Horizontal,Vertical");

      if (result == MN_VALID)
         result = mnDefMenu (SUB9, SUB4, OPT4, "Horizontal,Vertical");

      if (result == MN_VALID)
         result = mnDispMenu (M_MAIN, mnGRAY, mnBLACK, mnWHITE, grGREEN);

      farfree (menuString);

      return (result);
   }




/*----------------------------------------------------------------------------

      This procedure identifies an error code, sets the display to text
   mode and outputs an error message.  An integer argument is supplied
   corresponding to valid error codes assigned thoughout the program and
   defined in the menu.h and errmsg.h include files.

----------------------------------------------------------------------------*/

void ErrMsg (err_num)
int      err_num;
   {
      gxSetMode (gxTEXT);
      printf ("The following ");
      if (err_num >= MN_ERR_ENABLE)
         {
            printf ("Menu System Error has occured: \n\n");
            switch (err_num)
               {
                  case MN_ERR_EMPTY:
                     printf ("   The menu string specified in the menu definition\n");
                     printf ("   function (mnDefMenu) is empty, NULL or invalid. \n\n");
                     break;

                  case MN_ERR_OPTION:
                     printf ("   The option number specified in the menu definition function\n");
                     printf ("   (mnDefMenu) or the choice selection function (mnChooseOpt) was\n");
                     printf ("   greater than 10 or less than 0. \n\n");
                     break;

                  case MN_ERR_ALLOC:
                     printf ("   An error allocating memory occured in one of the following\n");
                     printf ("   modules:  mnDefMenu, mnReadMenu or StrCt.\n\n");
                     break;

                  case MN_ERR_NEWALLOC:
                     printf ("   An error occured while allocating memory for a menu node in\n");
                     printf ("   the menu definition module (mnDefMenu).\n\n");
                     break;

                  case MN_ERR_DISPMENU:
                     printf ("   An attempt was made to redisplay a menu that was already\n");
                     printf ("   currently displayed.\n\n");
                     break;

                  case MN_ERR_CLRMISS:
                     printf ("   Invalid menu id referenced in the menu clear function\n");
                     printf ("   (mnClearMenu).\n\n");
                     break;

                  case MN_ERR_DISPMISS:
                     printf ("   Invalid menu id referenced in the menu display function\n");
                     printf ("   (mnDispMenu).\n\n");
                     break;

                  case MN_ERR_SEPLINE:
                     printf ("   An invalid reference was made to a menu option. The option\n");
                     printf ("   specified referenced the seperation line of the menu in mnDispMenu.\n\n");
                     break;

                  case MN_ERR_DISORD:
                     printf ("   The option specified is not a member of the menu tree.  This\n");
                     printf ("   error occured in the GetMenuPos function.\n\n");
                     break;

                  case MN_ERR_CHOICE:
                     printf ("   The option selected in the choose option function (mnChooseOpt)\n");
                     printf ("   does not exist.\n\n");
                     break;

                  case MN_ERR_READ:
                     printf ("   An error occured during an attempt to open the menu file in the\n");
                     printf ("   module mnReadMenu.\n\n");
                     break;

                  case MN_ERR_CLRMENU:
                     printf ("   An attempt was made to clear an invalid menu.\n\n");
                     break;

                  case MN_ERR_UPDMENU:
                     printf ("   An attempt was made to update an inactive menu.\n\n");
                     break;

                  case MN_ERR_ENABLE:
                     printf ("   An error occured during the enable option execution.\n\n");
                     break;

                  default:
                     printf ("   An undetermined Menu error has occured: code %d\n\n", err_num);
                     break;

               }

         }

      else
         {
            printf ("General error has occured: \n\n");
            switch (err_num)
               {
                  case m_MIS_IMAGE:
                     printf ("   Missing image name in GetArgs module.\n\n");
                     break;

                  case m_UNK_IMAGE:
                     printf ("   Unknown image name specified in GetArgs module.\n\n");
                     break;

                  case m_MIS_OPT:
                     printf ("   Missing method option argument in GetArgs module.\n\n");
                     break;

                  case m_INV_OPT:
                     printf ("   Invalid method option specified in GetArgs module.\n\n");
                     break;

                  case m_EXT_DISP:
                     printf ("   An extraneous argument was supplied with the -D switch.\n\n");
                     break;

                  case m_EXT_HELP:
                     printf ("   Extraneous arguments supplied with the help option.\n\n");
                     break;

                  case m_INV_PARM:
                     printf ("   An unknown or invalid parameter switch was specified at the command \n");
                     printf ("   line.\n\n");
                     break;

                  case m_SYN_ERR:
                     printf ("   An invalid syntax was encountered at the command line.\n\n");
                     break;

                  case m_ERR_ALLOC:
                     printf ("   An error has occured while attempting to allocate memory for \n");
                     printf ("   a character string.\n\n");
                     break;

                  case m_ERR_AVAIL:
                     printf ("   There is insufficient far core memory available.\n\n");
                     break;

                  case m_EXT_STAT:
                     printf ("   There was an extraneous argument supplied with the -S switch.\n\n");
                     break;

                  case m_ERR_FOPEN:
                     printf ("   Error opening render configuration file.\n\n");
                     break;

                  case m_ERR_SETDISP:
                     printf ("   An error was encountered when trying to set the display type.\n\n");
                     break;

                  case m_ERR_SETMODE:
                     printf ("   An error occured when trying to select the required video mode.\n\n");
                     break;

                  case m_ERR_INITM:
                     printf ("   An error occured while trying to initialize the mouse.\n\n");
                     break;

                  case m_ERR_TRACK:
                     printf ("   An error occured when mouse tracking was enabled.\n\n");
                     break;

                  case m_INV_MODE:
                     printf ("   An invalid combination of command arguments was selected.\n\n");
                     break;

                  case m_INV_CONF:
                     printf ("   Invalid configuration file.\n\n");
                     break;

                  case m_UNK_PCX:
                     printf ("   Invalid PCX file name.\n\n");
                     break;

                  case m_MIS_PCX:
                     printf ("   Missing PCX file name.\n\n");
                     break;

                  case m_EXT_PCX:
                     printf ("   An extraneous argument was supplied with the restore switch.\n\n");
                     break;

                  case m_ERR_PCX:
                     printf ("   An error occured while attempt to display the specified PCX file.\n\n");
                     break;

                  case m_MIS_RED:
                     printf ("   Missing reduction value in video reduction argument.\n\n");
                     break;

                  case m_INV_RED:
                     printf ("   An invalid value was supplied with the reduction argument.\n\n");
                     break;

                  case m_ERR_SUP:
                     printf ("   An unsupported video mode is required to display the specified PCX file.\n\n");
                     break;

                  case m_BAD_INTENS:
                     printf ("   Invalid Intensity value.\n\n");
                     break;

                  case m_MUTEX:
                     printf ("   Cannot use both arguments -P and -V.\n\n");
                     break;

                  case m_BAD_ROOT:
                     printf ("   The Check root verification has found an invalid root.\n\n");
                     break;

                  case m_MIS_CFG:
                     printf ("   Missing configuration file name.\n\n");
                     break;

                  default:
                     printf ("   An undetermined error has occured: code %d\n\n", err_num);
                     break;

               }

         }

      exit (err_num);
   }




/*----------------------------------------------------------------------------

      This function opens the RENDER.CFG file and assigns valid values to
   various global variables.  The function has no parameters and returns a
   success or error code.

----------------------------------------------------------------------------*/

int  GetConfig (cfgStr)
char   far *cfgStr;
   {
      FILE    *filePtr;
      char     far *buffer, far *temp;

      buffer = (char far *) farcalloc (MED_STR, sizeof (char));
      temp = (char far *) farcalloc (SMALL_STR, sizeof (char));
      if (buffer == NULL || temp == NULL)
         return (m_ERR_ALLOC);


      /*  Open specified config file or the default config file  */

      if (_fstrlen (cfgStr) > 0)
         filePtr = fopen (cfgStr, "r");

      else
         filePtr = fopen ("render.cfg", "r");

      if (filePtr == NULL)
         return (m_ERR_FOPEN);

      while (!feof (filePtr))
         {
            /*  Read fields from configuration file  */

            fscanf (filePtr, "%s", buffer);
            if (!_fstrncmp (buffer, "SUNX:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  sun.xPos = (int) (atof (temp) * disp_info.vres);
               }

            if (!_fstrncmp (buffer, "SUNY:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  sun.yPos = (int) (atof (temp) * disp_info.vres);
               }

            if (!_fstrncmp (buffer, "SUNZ:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  sun.zPos = (int) (atof (temp) * disp_info.vres);
               }

            if (!_fstrncmp (buffer, "BCKC:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  backColor = atoi (temp);
               }

            if (!_fstrncmp (buffer, "SATA:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  saturate = atoi (temp);
               }

            if (!_fstrncmp (buffer, "CHCK:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  checkRoot = atoi (temp);
               }

            if (!_fstrncmp (buffer, "XROT:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  planeRotation.xPos = atoi (temp);
               }

            if (!_fstrncmp (buffer, "YROT:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  planeRotation.yPos = atoi (temp);
               }

            if (!_fstrncmp (buffer, "ZROT:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  planeRotation.zPos = atoi (temp);
               }

            if (!_fstrncmp (buffer, "INUM:", LABEL_SIZE))
               {
                  fscanf (filePtr, "%s", temp);
                  imageNumber = atoi (temp);
               }

         }


      /*  House cleaning  */

      farfree (temp);
      farfree (buffer);

      return (VALID);
   }



/*----------------------------------------------------------------------------

      This function assigns values to program variables specified by the
   user at the command line.  The arguments are as follows:  The standard
   argc and argv variables containing the user specified command line
   arguments to be parsed;  The method variable which is binary encoded
   to specify a rendering method for single or dual modes.  The method
   variable is modified with the -a, -b, -A or -B switches.  The status
   variable specifies whether or not a status screen will be displayed
   with the rendering and is activated with the -S switch.  The drawing
   mode (single or dual) is assign to the drawMode variable by the -D
   switch.  Finally, the leftScene and rightScene variables are assigned
   specific images to render with the -L and -R switches respectively.

----------------------------------------------------------------------------*/

int GetArgs (argc, argv, method, status, drawMode, reduction, leftScene, rightScene, restoreStr, saveStr, cfgStr)
int    argc;
char  *argv [];
int    far *method, far *status, far *drawMode, far *reduction;
char   far *leftScene, far *rightScene, far *restoreStr, far *saveStr, far *cfgStr;
   {
      int     i, j, value, strSize, compMode = FALSE, compArgs = FALSE;
      int     reqMode = FALSE, reqArgs = FALSE, mutEx = FALSE;

      if (argc != FIRST_ARG)
         {
            value = m_COMMAND;
            reqMode = TRUE;
         }

      else
         value = m_MENU;


      /*  Initialization of unrequired command line argument variables  */

      *status = OFF;
      *drawMode = MONO;
      *method = NONE;
      *reduction = NONE;
      for (i = FIRST_ARG; i < argc; ++i)
         if (argv [i][0] == '-')
            switch (argv [i][FIRST_ARG])
               {

                  /*  The binary encoding method for the method variable for
                      the following 4 switches is described as follows:
                      The 8 least significant bits determine the rendering
                      method for both left and right rendering regions.  Bits
                      1 through 3 (from the right) are reserved for the
                      right region, bits 4 through 6 are for the left region.
                      Bit 7 specifies the orientation for the right region
                      and bit 8 specifies the orientation for the left region. */

                  /*  Left region rendering method using vertical orientation  */

                  case 'a':
                     if (_fstrlen(argv [i]) == SWITCH_SIZE)
                        value = m_MIS_OPT;
                     else
                        {
                           *method = (*method + ((atoi (argv [i] + SWITCH_SIZE)) - 1) << 3);
                           *method = *method | VL_MASK;
                        }

                     if (reqArgs == HALF)
                        reqArgs = TRUE;
                     else
                        reqArgs = HALF;

                     break;


                  /*  Right region rendering method using vertical orientation  */

                  case 'b':
                     if (_fstrlen(argv [i]) == SWITCH_SIZE)
                        value = m_MIS_OPT;
                     else
                        {
                           *method = (*method + (atoi (argv [i] + SWITCH_SIZE) - 1));
                           *method = *method | VR_MASK;
                        }

                     if (compArgs == HALF)
                        compArgs = TRUE;
                     else
                        compArgs = HALF;

                     break;


                  /*  Left region rendering method using horizontal orientation  */

                  case 'A':
                     if (_fstrlen(argv [i]) == SWITCH_SIZE)
                        value = m_MIS_OPT;
                     else
                        {
                           *method = (*method + ((atoi (argv [i] + SWITCH_SIZE)) - 1) << 3);
                           *method = *method & HL_MASK;
                        }

                     if (reqArgs == HALF)
                        reqArgs = TRUE;
                     else
                        reqArgs = HALF;

                     break;


                  /*  Right region rendering method using horizontal orientation  */

                  case 'B':
                     if (_fstrlen(argv [i]) == SWITCH_SIZE)
                        value = m_MIS_OPT;
                     else
                        {
                           *method = (*method + (atoi (argv [i] + SWITCH_SIZE) - 1));
                           *method = *method & HR_MASK;
                        }

                     if (compArgs == HALF)
                        compArgs = TRUE;
                     else
                        compArgs = HALF;

                     break;


                  /*  Specifies the name of the PCX save file  */
   
                  case 'c':
                  case 'C':
                     reqMode = FALSE;
                     _fstrcpy (cfgStr, argv [i] + SWITCH_SIZE);
                     strSize = _fstrlen (cfgStr);
                     for (j = NONE; j < strSize; ++j)
                        if (j == NONE)
                           cfgStr [j] = toupper (cfgStr [j]);
                        else
                           cfgStr [j] = tolower (cfgStr [j]);

                     if (_fstrlen(saveStr) == SWITCH_SIZE)
                        value = m_MIS_CFG;

                     break;


                  /*  Enables dual mode rendering.  Requires the user to specify
                      both the -B and -R switches.                                */

                  case 'd':
                  case 'D':
                     if (strlen(argv [i]) > SWITCH_SIZE)
                        value = m_EXT_DISP;
                     else
                        *drawMode = DUAL;

                     compMode = TRUE;
                     break;


                  /*  Specifies the name of the left image  */

                  case 'l':
                  case 'L':
                     _fstrcpy (leftScene, argv [i]+2);
                     strSize = _fstrlen (leftScene);
                     for (j = NONE; j < strSize; ++j)
                        if (j == NONE)
                           leftScene [j] = toupper (leftScene [j]);
                        else
                           leftScene [j] = tolower (leftScene [j]);

                     if (_fstrlen(leftScene) == SWITCH_SIZE)
                        value = m_MIS_IMAGE;

                     else
                        if (_fstrcmp(leftScene, "Spheres") && _fstrcmp(leftScene, "Sphere") &&
                            _fstrcmp(leftScene, "Torus") && _fstrcmp(leftScene, "Tori") &&
                            _fstrcmp(leftScene, "Generic") && _fstrcmp(leftScene, "Medley"))
                           value = m_UNK_IMAGE;

                     if (reqArgs == HALF)
                        reqArgs = TRUE;
                     else
                        reqArgs = HALF;

                     break;


                  /*  Specifies the name of the PCX restore file  */
   
                  case 'p':
                  case 'P':
                     reqMode = FALSE;
                     if (argc != PCX_ARG)
                        value = m_EXT_PCX;

                     _fstrcpy (restoreStr, argv [i] + SWITCH_SIZE);
                     strSize = _fstrlen (restoreStr);
                     for (j = NONE; j < strSize; ++j)
                        if (j == NONE)
                           restoreStr [j] = toupper (restoreStr [j]);
                        else
                           restoreStr [j] = tolower (restoreStr [j]);

                     if (_fstrlen(restoreStr) == SWITCH_SIZE)
                        value = m_MIS_PCX;

                     else
                        if (access (restoreStr, EXIST) == NOSUCHFILE)
                           value = m_UNK_PCX;

                     mutEx = TRUE;
                     break;


                  /*  Specifies the name of the right image  */

                  case 'r':
                  case 'R':
                     _fstrcpy (rightScene, argv [i] + SWITCH_SIZE);
                     strSize = _fstrlen (rightScene);
                     for (j = NONE; j < strSize; ++j)
                        if (j == NONE)
                           rightScene [j] = toupper (rightScene [j]);
                        else
                           rightScene [j] = tolower (rightScene [j]);

                     if (_fstrlen(rightScene) == SWITCH_SIZE)
                        value = m_MIS_IMAGE;

                     else
                        if (_fstrcmp(leftScene, "Spheres") && _fstrcmp(leftScene, "Sphere") &&
                            _fstrcmp(leftScene, "Torus") && _fstrcmp(leftScene, "Tori") &&
                            _fstrcmp(leftScene, "Generic") && _fstrcmp(leftScene, "Medley"))
                           value = m_UNK_IMAGE;

                     if (compArgs == HALF)
                        compArgs = TRUE;
                     else
                        compArgs = HALF;

                     break;


                  /*  Enables the display of the status bar in either single or
                      dual mode.                                                */

                  case 's':
                  case 'S':
                     if (_fstrlen(argv [i]) > SWITCH_SIZE)
                        value = m_EXT_STAT;
                     else
                        *status = ON;

                     break;


                  /*  Incrementally reduce 256 color resolution  */

                  case 'v':
                  case 'V':
                     reqMode = FALSE;
                     if (mutEx)
                        value = m_MUTEX;

                     else 
                        {
                           if (_fstrlen(argv [i]) == SWITCH_SIZE)
                              value = m_MIS_RED;

                           else
                              *reduction = atoi (argv [i] + SWITCH_SIZE);

                           if (*reduction > MAX_RED || *reduction < 1)
                              value = m_INV_RED;

                        }

                     if (argc == RED_ARG)
                        value = m_MENU;

                     break;


                  /*  Specifies the name of the PCX save file  */
   
                  case 'x':
                  case 'X':
                     reqMode = FALSE;
                     _fstrcpy (saveStr, argv [i] + SWITCH_SIZE);
                     strSize = _fstrlen (saveStr);
                     for (j = NONE; j < strSize; ++j)
                        if (j == NONE)
                           saveStr [j] = toupper (saveStr [j]);
                        else
                           saveStr [j] = tolower (saveStr [j]);

                     if (_fstrlen(saveStr) == SWITCH_SIZE)
                        value = m_MIS_PCX;

                     break;


                  /*  Help message request  */

                  case '?':
                  case 'h':
                  case 'H':
                     reqMode = FALSE;
                     if (argc == HELP_ARG)
                        value = m_HELP;
                     else
                        value = m_EXT_HELP;

                     break;

                  default:
                     value = m_INV_PARM;
               }

            else
               value = m_SYN_ERR;


      /*  Check for invalid argument combinations  */

      if (reqArgs == HALF)
         reqArgs = FALSE;

      if (compArgs == HALF)
         compArgs = FALSE;

      if ((compMode && !compArgs) || (!compMode && compArgs) || (!reqArgs && reqMode))
         value = m_INV_MODE;

      return (value);
   }



/*----------------------------------------------------------------------------

         This procedure displays a list of valid options for running this
      rendition project in various modes.  A complete list of valid arguments
      is detailed.

----------------------------------------------------------------------------*/

void HelpMsg (void)
   {
      gxSetMode (gxTEXT);
      printf ("Runs the rendering program for Ray Tracing Algebraic Surfaces.\n\n");
      printf ("render [[-L|l<image>] [-R|r<image>] [-A|a<method>] [-B|b<method>]\n");
      printf ("       [-D|d] [-S|s] [-V|v<n>] [-X|x<filename>] [-C|c<filename]] |\n");
      printf ("       [-H|h|?] | [-P|p<filename>]\n\n");
      printf ("-A determines a valid rendering method for the left side of a dual\n");
      printf ("   mode rendering.  Upper case specifies Horizontal orientation where\n");
      printf ("   appropriate.  Where:\n");
      printf ("        1 = Standard iterative pixel selection technique.\n");
      printf ("        2 = Fibonacci based scan line technique.\n");
      printf ("        3 = Uniform fibonacci pixel selection technique.\n");
      printf ("        4 = Fibonacci fat scan line technique.\n");
      printf ("        5 = Fibonacci based psuedo random rendering using fat pixels.\n\n");
      printf ("-B Determines a valid rendering technique for the right side of a dual\n");
      printf ("   mode rendering.\n\n");
      printf ("-C Specifies a valid configuration file.  Default is render.cfg.\n\n");
      printf ("Strike any key to continue.\n");
      getch ();
      printf ("-D Sets rendering mode to Dual mode.\n");
      printf ("-H Displays this Help screen.\n");
      printf ("-L Selects an image for display on the left side of a Dual mode\n");
      printf ("   rendering.  Valid options are:\n");
      printf ("        Spheres, Sphere, Torus, Tori, Generic, Medley\n\n");
      printf ("-P Selects a previously saved PCX picture file.  This feature is used\n");
      printf ("   to display a previously rendered image.\n");
      printf ("-R Selects an image for display on the right side of a Dual mode\n");
      printf ("   rendering.\n");
      printf ("-S displays the status bar which contains performance data for\n");
      printf ("   image renditions.\n");
      printf ("-V Reduces the resolution of the current 256 color board one \n");
      printf ("   mode at a time.  Can not be used with -P.  Valid values are\n");
      printf ("   from 1 to 8.\n");
      printf ("-X Saves the completed image to the specified file name.\n\n");
      printf ("Note:  square brackets indicate optional parameters.  Vertical\n");
      printf ("       bars indicate a choice among multiple options.  Case\n");
      printf ("       selections are insignificant except where noted.\n");
   }
