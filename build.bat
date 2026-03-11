@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >/dev/null 2>&1
cd /d C:\ws\OpenSceneGraph\cmake-build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug .. > C:\ws\OpenSceneGraph\build_output.txt 2>&1
if %errorlevel% neq 0 (
    echo CMAKE CONFIGURE FAILED >> C:\ws\OpenSceneGraph\build_output.txt
    exit /b 1
)
echo CMAKE CONFIGURE OK >> C:\ws\OpenSceneGraph\build_output.txt
cmake --build . --target example_osgatomiccounter >> C:\ws\OpenSceneGraph\build_output.txt 2>&1
if %errorlevel% neq 0 (
    echo BUILD FAILED >> C:\ws\OpenSceneGraph\build_output.txt
    exit /b 1
)
echo BUILD OK >> C:\ws\OpenSceneGraph\build_output.txt
