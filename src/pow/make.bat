@ECHO off
SET ROOT=..\..
SET SUBDIR=pow
SET DOLIB=pow
SET DOEXE=
SET DEPENDS=
SET CC_EXTRA=/arch:AVX2
CALL ..\..\env\make.bat %*
