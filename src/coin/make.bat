@ECHO off
SET ROOT=..\..
SET SUBDIR=coin
SET DOLIB=coin
SET DOEXE=
SET DEPENDS=common interp
CALL ..\..\env\make.bat %*

