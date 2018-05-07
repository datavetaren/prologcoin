@ECHO off
SET ROOT=..\..
SET SUBDIR=ec
SET DOLIB=ec
SET DEPENDS=common interp
rem
CALL ..\..\env\make.bat %*

