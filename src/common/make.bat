@ECHO off
SET ROOT=..\..
SET SUBDIR=common
SET DOLIB=common
SET DOEXE=
SET DEPENDS=
CALL ..\..\env\make.bat %*
