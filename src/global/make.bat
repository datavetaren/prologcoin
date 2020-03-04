@ECHO off
SET ROOT=..\..
SET SUBDIR=global
SET DOLIB=global
SET DOEXE=
SET DEPENDS=secp256k1 common db interp ec coin
CALL ..\..\env\make.bat %*

