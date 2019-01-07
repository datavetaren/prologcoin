@ECHO off
SET ROOT=..\..
SET SUBDIR=ec
SET DOLIB=ec
SET DOEXE=
SET DEPENDS=common interp secp256k1
CALL ..\..\env\make.bat %*

