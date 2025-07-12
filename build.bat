@echo off

mkdir .\bin
pushd .\bin

cl /MD /std:c++17 ^
	/EHsc /Zi ^
	/wd4005 ^
	/I"..\src" /I"..\libs" ^
	..\src\main.cc ^
	kernel32.lib shell32.lib user32.lib gdi32.lib comdlg32.lib d3d11.lib d3dcompiler.lib ^
	/link /LIBPATH:C:\projects\gpucalipse\third_party

popd