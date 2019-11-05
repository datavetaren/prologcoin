@ECHO off
SET ROOT=..\..
SET SUBDIR=secp256k1
SET DOLIB=secp256k1
SET DOEXE=
SET DEPENDS=
SET CINCLUDE1=%ROOT%\..\secp256k1-zkp
SET CINCLUDE2=%ROOT%\..\secp256k1-zkp\src
CALL ..\..\env\make.bat %*
