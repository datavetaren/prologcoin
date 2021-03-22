@ECHO off
SET ROOT=..\..
SET SUBDIR=main
SET DOLIB=
SET DOEXE=prologcoind
SET DEPENDS=secp256k1 wallet node ec coin global terminal interp db pow common
SET SCRIPTARGS=%ROOT%\bin\prologcoind.exe
CALL ..\..\env\make.bat %*
