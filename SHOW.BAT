ECHO OFF
REM  File:   Show.bat

REM          This file is used to display PCX files with the render executable

REM  original lighting direction and flooding
render -pscene01.pcx 

REM  Variation of lighting direction 
render -pscene02.pcx 

REM  Variation of direction and saturation 
render -pscene03.pcx 

REM  Variation of direction and flood lighting
render -pscene04.pcx 

REM  Rotation of image plane 
render -pscene05.pcx 

REM  Additional images using original model
render -pscene06.pcx 

REM  New image original lighting
render -pscene07.pcx 

REM  Original lighting standard incremental single dimensional rotation
render -pscene08.pcx 

REM  Original lighting two plane rotation
render -pscene09.pcx 

REM  Original lighting three plane rotation
render -pscene10.pcx 

REM  Combination images Original lighting three plane rotation
render -pscene11.pcx 

REM  New images Original lighting
render -pscene12.pcx 

REM  Dual mode original lighting same image
render -pscene13.pcx 

REM  Dual mode original lighting different images
render -pscene14.pcx 

REM  Dual mode original lighting same image new background color
render -pscene15.pcx 

REM  Original lighting of new image
render -pscene16.pcx 

