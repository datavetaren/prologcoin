@ECHO off
SET ROOT=..\..
SET SUBDIR=main
SET DOLIB=
SET DOEXE=prologcoind
SET DEPENDS=secp256k1 node ec coin global interp common
SET SCRIPTARGS=%ROOT%\bin\prologcoind.exe
CALL ..\..\env\make.bat %*
