@ECHO off
SET ROOT=..\..
SET SUBDIR=main
SET DOLIB=
SET DOEXE=prologcoind
SET DEPENDS=secp256k1 node ec interp common
CALL ..\..\env\make.bat %*

