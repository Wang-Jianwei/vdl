@echo off
REM VDL 项目 Windows 编译脚本
REM 使用方法: build.bat [选项]

setlocal enabledelayedexpansion

REM ============================================================================
REM 变量初始化
REM ============================================================================

set BUILD_DIR=build
set BUILD_TYPE=Debug
set CLEAN=0
set REBUILD=0
set RUN_TESTS=0
set GENERATOR=

REM 检测可用的生成器
where /q ninja
if %ERRORLEVEL% equ 0 (
    set GENERATOR=-G Ninja
) else (
    where /q msbuild
    if %ERRORLEVEL% equ 0 (
        set GENERATOR=-G "Visual Studio 16 2019"
    )
)

REM ============================================================================
REM 参数解析
REM ============================================================================

:parse_args
if "%~1"=="" goto end_parse

if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="help" goto show_help

if /i "%~1"=="-c" set CLEAN=1 & shift & goto parse_args
if /i "%~1"=="--clean" set CLEAN=1 & shift & goto parse_args
if /i "%~1"=="clean" set CLEAN=1 & shift & goto parse_args

if /i "%~1"=="-r" set REBUILD=1 & shift & goto parse_args
if /i "%~1"=="--rebuild" set REBUILD=1 & shift & goto parse_args
if /i "%~1"=="rebuild" set REBUILD=1 & shift & goto parse_args

if /i "%~1"=="-d" set BUILD_TYPE=Debug & shift & goto parse_args
if /i "%~1"=="--debug" set BUILD_TYPE=Debug & shift & goto parse_args

if /i "%~1"=="-R" set BUILD_TYPE=Release & shift & goto parse_args
if /i "%~1"=="--release" set BUILD_TYPE=Release & shift & goto parse_args

if /i "%~1"=="-t" set RUN_TESTS=1 & shift & goto parse_args
if /i "%~1"=="--test" set RUN_TESTS=1 & shift & goto parse_args
if /i "%~1"=="test" set RUN_TESTS=1 & shift & goto parse_args

echo 未知选项: %~1
echo 使用 --help 查看帮助信息
exit /b 1

:end_parse

REM ============================================================================
REM 主逻辑
REM ============================================================================

echo.
echo ==========================================
echo   VDL 构建配置
echo ==========================================
echo 构建模式: %BUILD_TYPE%
echo 构建目录: %BUILD_DIR%
echo.

REM 清理构建
if %CLEAN%==1 (
    echo [清理] 删除构建目录...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
        echo [完成] 构建目录已清理
    ) else (
        echo [信息] 构建目录不存在
    )
    
    if %REBUILD%==0 (
        echo [完成] 清理完成
        exit /b 0
    )
)

if %REBUILD%==1 (
    echo [重建] 清理构建目录...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
    )
)

REM 创建构建目录
if not exist "%BUILD_DIR%" (
    echo [配置] 创建构建目录...
    mkdir "%BUILD_DIR%"
)

REM 配置项目
echo.
echo ==========================================
echo   配置项目 (CMake)
echo ==========================================
cd "%BUILD_DIR%"

cmake %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
if %ERRORLEVEL% neq 0 (
    echo [错误] CMake 配置失败
    exit /b 1
)
echo [完成] CMake 配置完成

REM 编译项目
echo.
echo ==========================================
echo   编译项目
echo ==========================================

cmake --build . --config %BUILD_TYPE%
if %ERRORLEVEL% neq 0 (
    echo [错误] 编译失败
    exit /b 1
)

echo.
echo [成功] 编译完成！

REM 显示编译结果
echo.
echo [信息] 可执行文件:
if exist "bin\%BUILD_TYPE%" (
    dir /b "bin\%BUILD_TYPE%\*.exe"
) else if exist "bin" (
    dir /b "bin\*.exe"
)

REM 运行测试
if %RUN_TESTS%==1 (
    echo.
    echo ==========================================
    echo   运行测试
    echo ==========================================
    
    if exist "bin\%BUILD_TYPE%\vdl_tests.exe" (
        "bin\%BUILD_TYPE%\vdl_tests.exe"
    ) else if exist "bin\vdl_tests.exe" (
        "bin\vdl_tests.exe"
    ) else (
        echo [警告] 测试程序不存在
    )
)

cd ..

echo.
echo ==========================================
echo   构建完成
echo ==========================================
echo VDL 项目编译成功！(%BUILD_TYPE% 模式)
echo.

exit /b 0

:show_help
echo.
echo VDL 项目编译脚本 (Windows)
echo.
echo 用法:
echo     build.bat [选项]
echo.
echo 选项:
echo     -h, --help          显示此帮助信息
echo     -c, --clean         清理构建目录
echo     -r, --rebuild       清理后重新构建
echo     -d, --debug         Debug 模式编译（默认）
echo     -R, --release       Release 模式编译
echo     -t, --test          编译后运行测试
echo.
echo 示例:
echo     build.bat                # 默认 Debug 模式编译
echo     build.bat -R -t          # Release 模式编译并运行测试
echo     build.bat -c             # 清理构建目录
echo     build.bat -r             # 清理后重新编译
echo.
echo 快捷方式:
echo     build.bat clean          # 等同于 --clean
echo     build.bat test           # 等同于 --test
echo     build.bat rebuild        # 等同于 --rebuild
echo.
exit /b 0
