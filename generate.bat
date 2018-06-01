@echo off
set PATH=%PATH%;%cd%

mkdir output
pushd output

rem ### Build with ninja
cmake -G"Ninja" -DCMAKE_TOOLCHAIN_FILE=../Toolchain-stm32f4.cmake ../
rem ..\ninja.exe

rem ### Build with nmake
rem cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=../Toolchain-stm32f4.cmake ../
rem ..\nmake.exe

rem ### Build with make
rem cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../Toolchain-stm32f4.cmake ../
rem ..\gnumake-4.2.1-x64.exe

popd
