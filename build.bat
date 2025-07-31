@echo off

if "%VSCMD_VER%"=="" (
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not exist build mkdir build

pushd build

set INCLUDE_DIR=/I..\src\ext /I..\src\ext\imgui /I..\src\ext\imgui\backends
set IMGUI_DIR=..\src\ext\imgui
set IMGUI_SRC=%IMGUI_DIR%\imgui.cpp %IMGUI_DIR%/imgui_demo.cpp %IMGUI_DIR%/imgui_draw.cpp %IMGUI_DIR%/imgui_tables.cpp %IMGUI_DIR%/imgui_widgets.cpp %IMGUI_DIR%/backends/imgui_impl_sdlrenderer3.cpp %IMGUI_DIR%/backends/imgui_impl_sdl3.cpp

set COMPILE_FLAGS=/Z7 /EHsc /DPLATFORM_WIN32
set LINK_FLAGS=/LIBPATH:..\lib\win32\SDL3 /LIBPATH:..\lib\win32\opencv SDL3.lib opencv_world4120.lib

REM cl %INCLUDE_DIR% %COMPILE_FLAGS% ..\src\narcissus.cpp %IMGUI_SRC% /link %LINK_FLAGS%

cl %INCLUDE_DIR% %COMPILE_FLAGS% ..\src\narcissus.cpp /link %LINK_FLAGS%

copy ..\lib\win32\SDL3\SDL3.dll .
copy ..\lib\win32\opencv\opencv_world4120.dll .

popd