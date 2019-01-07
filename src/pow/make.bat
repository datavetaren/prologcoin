@ECHO off
SET ROOT=..\..
SET SUBDIR=pow
SET DOLIB=pow
SET DOEXE=
SET DEPENDS=common
SET CC_EXTRA=/arch:AVX2
CALL ..\..\env\make.bat %*
