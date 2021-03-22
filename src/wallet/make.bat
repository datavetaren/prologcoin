@ECHO off
SET ROOT=..\..
SET SUBDIR=wallet
SET DOLIB=wallet
SET DOEXE=
SET DEPENDS=secp256k1 common db interp terminal ec coin global node pow
CALL ..\..\env\make.bat %*

