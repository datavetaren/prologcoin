@ECHO off
SET ROOT=..\..
SET SUBDIR=wallet
SET DOLIB=wallet
SET DOEXE=
SET DEPENDS=secp256k1 common interp terminal ec coin global
CALL ..\..\env\make.bat %*

