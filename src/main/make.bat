@ECHO off
SET ROOT=..\..
SET SUBDIR=main
SET DOLIB=
SET DOEXE=prologcoind
SET DEPENDS=node ec interp common
CALL ..\..\env\make.bat %*

