@echo off

REM -- Change settings here -------------------------------------------

SET CHOOSEVC=
REM To set a specific VC: SET CHOOSEVC=VS110 (VS120, VS130, ...)

SET CCFLAGS=/nologo /c /EHsc
SET LINKFLAGS=/nologo

REM -------------------------------------------------------------------

REM
REM Check if cl.exe is present on path
REM
WHERE /q cl.exe
IF NOT ERRORLEVEL 1 (
GOTO :MAIN
)

SETLOCAL ENABLEDELAYEDEXPANSION
set VCHOME=
call :SELECTVC "%CHOOSEVC%" VCHOME VCNAME
ECHO !VCHOME! >%TEMP%\vchome.txt
ECHO !VCNAME! >%TEMP%\vcname.txt
ENDLOCAL

FOR /f "delims=" %%A IN (%TEMP%\vchome.txt) DO SET VCHOME=%%A
FOR /f "delims=" %%A IN (%TEMP%\vcname.txt) DO SET VCNAME=%%A
CALL :trim VCHOME %VCHOME%
CALL :trim VCNAME %VCNAME%

CALL "%VCHOME%..\..\vc\bin\vcvars32.bat"

GOTO:MAIN

:trim
SETLOCAL ENABLEDELAYEDEXPANSION
SET params=%*
FOR /f "tokens=1*" %%a IN ("!params!") DO ENDLOCAL & SET %1=%%b
GOTO :EOF

:CHECKVC
SET _VCLIST=%2
SET _VCLIST=%_VCLIST:"=%
IF DEFINED %1 (
   IF NOT "!_VCLIST!"=="" SET _VCLIST=!_VCLIST!,
   SET _VCLIST=!_VCLIST!%1
)
SET %3=!_VCLIST!
GOTO:EOF

:SELECTVC
set CHOOSEVC=%1
set CHOOSEVC=%CHOOSEVC:"=%
set VCLIST=
call :CHECKVC VS150COMNTOOLS "!VCLIST!" VCLIST
call :CHECKVC VS140COMNTOOLS "!VCLIST!" VCLIST
call :CHECKVC VS130COMNTOOLS "!VCLIST!" VCLIST
call :CHECKVC VS120COMNTOOLS "!VCLIST!" VCLIST
call :CHECKVC VS110COMNTOOLS "!VCLIST!" VCLIST

SET VCARG=
FOR %%X IN (%VCLIST%) DO (
   IF "%%X"=="!CHOOSEVC!COMNTOOLS" SET VCARG=%%X
)

IF "!VCARG!"=="" (
  FOR %%X IN (!VCLIST!) DO (
     IF "!VCARG!"=="" SET VCARG=%%X
  )
)
set %2=!%VCARG%!
set %3=!VCARG!

GOTO :EOF

REM ----------------------------------------------------
REM  MAIN
REM ----------------------------------------------------

:MAIN

IF "%1"=="help" (
GOTO :HELP
)

SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION

REM
REM It's hopeless to use gmake and fix gmake so it works under Windows.
REM Normally, users would use Visual Studio and not the command line.
REM However, this provides a simple way of building without relying
REM on external tools.
REM

set ROOT=..
set ABSROOT=%~dp0

set ENV=%ROOT%\env
set SRC=%ROOT%\src
set OUT=%ROOT%\out
set BIN=%ROOT%\bin

IF "%1"=="clean" (
GOTO :CLEAN
)

IF "%1"=="vcxproj" (
GOTO :VCXPROJ
)

REM
REM Main goal file
REM
set GOAL=%BIN%\omegal.exe

REM
REM Get all .cpp files
REM
set CPP_FILES=
FOR %%S IN (%SRC%\*.cpp) DO (
    set CPP_FILES=!CPP_FILES! %%S
)

REM
REM Ensure that out directory is present
REM
IF NOT EXIST %OUT% (
mkdir %OUT%
)

REM
REM Compile all .cpp files into .obj
REM
set OBJ_FILES=
for %%i in (!CPP_FILES!) DO (
    set CPPFILE=%%i
    set OBJFILE=!CPPFILE:.cpp=.obj!
    set OBJFILE=!OBJFILE:%SRC%=%OUT%!
    IF NOT EXIST !OBJFILE! (
        cl %CCFLAGS% /Fo:!OBJFILE! !CPPFILE!
        IF ERRORLEVEL 1 GOTO :EOF
    )
    set OBJ_FILES=!OBJ_FILES! "!OBJFILE!"
)

REM
REM Ensure that bin directory is present
REM
IF NOT EXIST %BIN% (
mkdir %BIN%
)

REM
REM Dispatch to test if requestedd
REM
IF "%1"=="test" (
GOTO :TEST
)

IF EXIST !GOAL! (
echo !GOAL! already exists. Run make clean to first to recompile.
GOTO :EOF
)

REM
REM Link final goal
REM
link %LINKFLAGS% /out:"!GOAL!" !OBJ_FILES!

GOTO :EOF

REM ----------------------------------------------------
REM  TEST
REM ----------------------------------------------------

:TEST

REM
REM Ensure that out\test and bin\test directories are present
REM
IF NOT EXIST %OUT%\test (
mkdir %OUT%\test
)
IF NOT EXIST %BIN%\test (
mkdir %BIN%\test
)

REM
REM Compile and run tests
REM
set OBJ_FILES0=
for %%i in (!OBJ_FILES!) DO (
    IF NOT "%%~ni"=="main" (
        set OBJ_FILES0=!OBJ_FILES0! %%i
    )
)
for %%S in (%SRC%\test\*.cpp) DO (
    set CPPFILE=%%S
    set OBJFILE=!CPPFILE:.cpp=.obj!
    set OBJFILE=!OBJFILE:%SRC%=%OUT%!
    set BINOKFILE=!OBJFILE:%OUT%=%BIN%!
    set BINEXEFILE=!OBJFILE:%OUT%=%BIN%!
    for %%i in (!CPPFILE!) do set BINOKFILE=%BIN%\test\%%~ni.ok
    for %%i in (!CPPFILE!) do set BINEXEFILE=%BIN%\test\%%~ni.exe
    for %%i in (!CPPFILE!) do set BINLOGFILE=%BIN%\test\%%~ni.log
    IF NOT EXIST !BINOKFILE! (
        cl %CCFLAGS% /I%SRC% /Fo:!OBJFILE! !CPPFILE!
        IF ERRORLEVEL 1 GOTO :EOF
	link %LINKFLAGS% /out:!BINEXEFILE! !OBJ_FILES0! !OBJFILE!
        IF ERRORLEVEL 1 GOTO :EOF
	!BINEXEFILE! 1>!BINLOGFILE! 2>&1
        IF ERRORLEVEL 1 GOTO :EOF
	for %%i in (!BINLOGFILE!) do if %%~zi==0 (
	    echo Error while running !BINEXEFILE!
	    GOTO :EOF
	)
	copy /Y NUL !BINOKFILE! >NUL
    )
)

GOTO :EOF

REM ----------------------------------------------------
REM  CLEAN
REM ----------------------------------------------------

:CLEAN
del /S /F /Q %OUT%
del /S /F /Q %BIN%
rmdir /S /Q %OUT%
rmdir /S /Q %BIN%

GOTO :EOF

REM ----------------------------------------------------
REM  VCXPROJ
REM ----------------------------------------------------

:VCXPROJ

REM
REM Check if csc.exe is present on path
REM
WHERE /q csc.exe
IF ERRORLEVEL 1 (
echo Cannot find csc.exe
GOTO :EOF
)

REM
REM Ensure that bin directory is present
REM
IF NOT EXIST %BIN% (
mkdir %BIN%
)

echo Compiling make_vcxproj.cs
csc.exe /nologo /reference:Microsoft.Build.dll /reference:Microsoft.Build.Framework.dll /out:%BIN%\make_vcxproj.exe %ENV%\make_vcxproj.cs
IF ERRORLEVEL 1 GOTO :EOF
%BIN%\make_vcxproj.exe env=%VCNAME% src=%SRC% out=%OUT% bin=%BIN%

GOTO :EOF

:HELP
echo --------------------------------------------------------
echo  make help
echo --------------------------------------------------------
echo  make          To build the application
echo  make clean    To clean everything
echo  make test     To build and run tests
echo  make vcxproj  To build vcxproj file for Visual Studio
echo --------------------------------------------------------

GOTO :EOF