ECHO OFF
REM  File:   Scene.bat

REM          This file creates a variety of image scenes for demonstrating the
REM          various special effects and images include in this package.

REM  Original lighting direction and flooding
render -a1 -Lsphere -S -Ceffect01.cfg -Xscene01.pcx 

REM  Variation of lighting direction 
render -a1 -Lsphere -S -Ceffect02.cfg -Xscene02.pcx 

REM  Variation of direction and saturation 
render -a2 -Lsphere -S -Ceffect03.cfg -Xscene03.pcx 

REM  Variation of lighting direction
render -A3 -Lsphere -S -Ceffect04.cfg -Xscene04.pcx 

REM  Rotation of image plane 
render -A4 -Lsphere -Ceffect05.cfg -Xscene05.pcx 

REM  Additional images using original model
render -a5 -Lspheres -S -Ceffect01.cfg -Xscene06.pcx 

REM  New image original lighting
render -a5 -LTorus -S -Ceffect01.cfg -Xscene07.pcx 

REM  Original lighting standard incremental single dimensional rotation
render -A3 -LTorus -S -Ceffect05.cfg -Xscene08.pcx 

REM  Original lighting two plane rotation
render -a3 -LTorus -S -Ceffect06.cfg -Xscene09.pcx 

REM  Original lighting three plane rotation
render -a3 -LTorus -S -Ceffect07.cfg -Xscene10.pcx 

REM  Combination images Original lighting three plane rotation
render -A5 -LMedley -Ceffect07.cfg -Xscene11.pcx 

REM  New images Original lighting
render -a3 -LMedley -Ceffect01.cfg -Xscene12.pcx 

REM  Dual mode original lighting same image
render -D -A5 -LMedley -b1 -Rsphere -S -Ceffect01.cfg -Xscene13.pcx 

REM  Dual mode original lighting different images
render -D -a4 -LTori -B5 -RSpheres -Ceffect01.cfg -Xscene14.pcx 

REM  Dual mode original lighting same image new background color
render -D -a4 -LTori -B5 -RSpheres -Ceffect08.cfg -Xscene15.pcx 

REM  Original lighting of new image
render -a4 -LGeneric -Ceffect01.cfg -Xscene16.pcx
