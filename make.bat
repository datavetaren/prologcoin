@echo off
SET MODULES=common db interp terminal secp256k1 pow ec coin global node wallet main

IF "%1"=="all" (
GOTO :ALL
)

IF "%1"=="clean" (
GOTO :CLEAN
)

IF "%1"=="vcxproj" (
GOTO :VCXPROJ
)

IF "%1"=="vssln" (
GOTO :VSSLN
)

IF "%1"=="count" (
GOTO :COUNT
)

echo ------------------------------------------------------
echo  make all     to build everything from command prompt
echo ------------------------------------------------------
echo  or:
echo.
echo  make vcxproj to build Visual Studio vcxproj files
echo  make vssln   to build Visual Studio Solution file
echo.
echo  Then load bin/prologcoin.sln and issue rebuild in
echo  Visual Studio to rebuild everything.
echo  You can then launch with (Ctrl)-F5 to start the
echo  prologcoind client in interactive mode.
echo ------------------------------------------------------

GOTO :EXIT

:ALL

FOR %%X IN (%MODULES%) DO (
   SET ROOT=
   SET SUBDIR=
   SET DOLIB=
   SET DOEXE=
   SET DEPENDS=
   SET CINCLUDE1=
   SET CINCLUDE2=

   cmd /c "cd src\%%X && make.bat check"
   REM If error level is 1, then we need to build this module
   IF ERRORLEVEL 1 (
       echo ------------------------------------------------------
       echo  Build %%X
       echo ------------------------------------------------------
       cmd /c "cd src\%%X && make.bat"
       IF ERRORLEVEL 1 GOTO :EOF
       IF EXIST src\%%X\script (
       echo ------------------------------------------------------
       echo  Script %%X
       echo ------------------------------------------------------
       cmd /c "cd src\%%X && make.bat script"
       )
       IF ERRORLEVEL 1 GOTO :EOF       
       IF EXIST src\%%X\test (
       echo ------------------------------------------------------
       echo  Test %%X
       echo ------------------------------------------------------
       cmd /c "cd src\%%X && make.bat test"
       )
       IF ERRORLEVEL 1 GOTO :EOF
   )
)

GOTO :EXIT

:CLEAN

IF EXIST out rd /s /q out
IF EXIST bin rd /s /q bin

GOTO :EXIT

:VCXPROJ
:VSSLN

pushd
cd src\common
make.bat %1
popd
GOTO :EXIT

:COUNT
setlocal EnableDelayedExpansion
set total=0
for /f "tokens=*" %%S in ('dir /b /s /a:d "src"') do (
    set cmd=`"find /v /c "" %%S\*.hpp 2>NUL"`
    FOR /f "usebackq tokens=3" %%X IN (!cmd!) DO (
        set /A total=!total!+%%X
    )
    set cmd=`"find /v /c "" %%S\*.cpp 2>NUL"`
    FOR /f "usebackq tokens=3" %%X IN (!cmd!) DO (
        set /A total=!total!+%%X
    )
    set cmd=`"find /v /c "" %%S\*.pl 2>NUL"`
    FOR /f "usebackq tokens=3" %%X IN (!cmd!) DO (
        set /A total=!total!+%%X
    )
)
set cmd=`"find /v /c "" src\common\term_parser_gen.hpp"`
FOR /f "usebackq tokens=3" %%X IN (!cmd!) DO (
    set /A total=!total!-%%X
)
echo Lines of code: !total!

GOTO :EXIT

:EXIT
