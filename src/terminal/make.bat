@ECHO off
SET ROOT=..\..
SET SUBDIR=terminal
SET DOLIB=terminal
SET DOEXE=
SET DEPENDS=common interp
CALL ..\..\env\make.bat %*

