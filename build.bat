@echo off

if not exist build mkdir build

pushd build

set INCLUDE_DIR=/I..\include /I..\include\imgui /I..\include\imgui\backends
set IMGUI_DIR=..\include\imgui
set IMGUI_SRC=%IMGUI_DIR%\imgui.cpp %IMGUI_DIR%/imgui_demo.cpp %IMGUI_DIR%/imgui_draw.cpp %IMGUI_DIR%/imgui_tables.cpp %IMGUI_DIR%/imgui_widgets.cpp %IMGUI_DIR%/backends/imgui_impl_sdlrenderer3.cpp %IMGUI_DIR%/backends/imgui_impl_sdl3.cpp

set COMPILE_FLAGS=%INCLUDE_DIR% /EHsc /DPLATFORM_WIN32 /MT
set LINK_FLAGS=/LIBPATH:..\lib\win32\SDL3 /LIBPATH:..\lib\win32\opencv SDL3.lib opencv_world4120.lib

cl %COMPILE_FLAGS% ..\src\narcissus.cpp %IMGUI_SRC% /link %LINK_FLAGS%

copy ..\lib\win32\SDL3\SDL3.dll .
copy ..\lib\win32\opencv\opencv_world4120.dll .

popd