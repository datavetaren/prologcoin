@ECHO off
SET ROOT=..\..
SET SUBDIR=ec
SET DOLIB=ec
SET DOEXE=
SET DEPENDS=common interp secp256k1
SET CINCLUDE1=%ROOT%\..\secp256k1-zkp
SET CINCLUDE2=%ROOT%\..\secp256k1-zkp\src
CALL ..\..\env\make.bat %*

