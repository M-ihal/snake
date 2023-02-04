@echo off

where /q cl.exe
if ERRORLEVEL 1 (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)

pushd %~dp0

set exe_name=p_win32
set include_dirs=/I../include/
set lib_dirs=/LIBPATH:../lib

REM O2 - optimizations
REM Od - no optimizations
REM /wd4100
REM /wd4101
REM /wd4189
REM /wd4715
REM /wd4505

REM -Zi
set common_compile_flags=-MT -nologo -Gm- -GR- -EHa- -O2 -WX -W4 -FC -Z7

set compile_flags=%common_compile_flags% /wd4201 /wd4057 /wd4213 /wd4100 /wd4101 /wd4189 /wd4702 /wd4715 /wd4505
set link_flags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console opengl32.lib glew32.lib glew32s.lib glfw3dll.lib OpenAL32.lib gdi32.lib user32.lib winmm.lib Shlwapi.lib -opt:ref -incremental:no /Debug:fastlink

set dll_name=p_game
set dll_pdb=p_game_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set dll_pdb=p_game_pdb
set dll_compile_flags=%common_compile_flags% /wd4201 /wd4057 /wd4213 /wd4100 /wd4101 /wd4189 /wd4702 /wd4715 /wd4505 /LD
set dll_link_flags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console -opt:ref -incremental:no /Debug:fastlink

if not exist "../bin" mkdir "../bin"
pushd "../bin/"

del *.pdb > NUL 2> NUL

REM game dll
echo COMPILING GAME DLL > p_game_dll.lock
CALL cl %include_dirs% %dll_compile_flags% "../source/p_game_main.cpp" /link /PDB:%dll_pdb%.pdb %lib_dirs% %dll_link_flags% /out:%dll_name%.dll
del p_game_dll.lock

REM platform
CALL cl %include_dirs% %compile_flags% "../source/p_win32_main.cpp" /link %lib_dirs% %link_flags% /out:%exe_name%.exe

echo f | xcopy /f /y "..\dll\*.dll" "..\bin\*.dll" > NUL
REM echo f | xcopy /f /y "..\data" "..\bin\data\" > NUL

popd

PAUSE