@ECHO off
SET ROOT=..\..
SET SUBDIR=node
SET DOLIB=node
SET DOEXE=
SET DEPENDS=common interp ec
CALL ..\..\env\make.bat %*

