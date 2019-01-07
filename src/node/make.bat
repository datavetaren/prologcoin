@ECHO off
SET ROOT=..\..
SET SUBDIR=node
SET DOLIB=node
SET DOEXE=
SET DEPENDS=secp256k1 common interp ec
CALL ..\..\env\make.bat %*

