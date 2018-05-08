@ECHO off
SET ROOT=..\..
SET SUBDIR=ec
SET DOLIB=ec
SET DOEXE=
SET DEPENDS=common interp
CALL ..\..\env\make.bat %*

