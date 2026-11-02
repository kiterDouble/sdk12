@echo off
setlocal enabledelayedexpansion
chcp 936 >nul

:: -------------------------
:: 配置区（按需修改）
:: -------------------------
set ADB=adb
set LOGSTORE_REMOTE=/mnt/sdcard/log/logstore
set SIRUN_REMOTE=/oemdata/log/sirunlog
set SIRUN_DIR=/oemdata/log
set PARAM_DIR=/oemdata/parameters
set PARAM_DB=paramsDb
set UPGRADE_SCRIPT=/oemapp/scripts/start_selfupgrade.sh
set NOTEPADPP_EXE=C:\Program Files\Notepad++\notepad++.exe

:: -------------------------
:: 主检测与菜单
:: -------------------------
:CHECK_DEVICE
:: 移除 cls，保留历史
echo.
echo 检测设备连接状态...
%ADB% devices | findstr /R /C:"device$" >nul
if %errorlevel% neq 0 (
    echo 未检测到设备，3 秒后重试...
    timeout /t 3 >nul
    goto CHECK_DEVICE
)
echo 设备已连接
timeout /t 1 >nul
goto MENU

:MENU
:: 移除 cls，只在第一次运行时清屏（可选），后续保留历史
echo.
echo ================== 设备工具菜单 ==================
echo 1. 清除所有历史 log (logstore)
echo 2. 仅删除 sirunlog 文件
echo 3. 提取 sirunlog 并按时间重命名
echo 4. 提取 sirunlog 并用 Notepad++ 打开
echo 5. 读取数据库（完整显示 + 提取关键字段）
echo 6. 一键升级 MCU
echo 7. 一键升级 MPU
echo 8. 一键升级 全部
echo 9. 导出最近 6 个日志（sirunlog + 最近 5 个 tgz 中的 log）
echo q. 退出
echo ==================================================
set /p CHOICE=请选择功能（输入序号或 q 退出）:
if /i "%CHOICE%"=="q" exit /b
if "%CHOICE%"=="1" goto FUNC_CLEAR_LOG
if "%CHOICE%"=="2" goto FUNC_DELETE_SIRUN_ONLY
if "%CHOICE%"=="3" goto FUNC_PULL_SIRUN_RENAME
if "%CHOICE%"=="4" goto FUNC_PULL_SIRUN_OPEN
if "%CHOICE%"=="5" goto FUNC_READ_DB
if "%CHOICE%"=="6" goto FUNC_UPGRADE_MCU
if "%CHOICE%"=="7" goto FUNC_UPGRADE_MPU
if "%CHOICE%"=="8" goto FUNC_UPGRADE_ALL
if "%CHOICE%"=="9" goto FUNC_EXPORT_6LOGS
echo 无效选择，请重新输入
timeout /t 2 >nul
goto MENU

:: =========================
:: 功能1 - 清除 logstore
:: =========================
:FUNC_CLEAR_LOG
echo.
echo [操作记录] 正在删除设备上的 %LOGSTORE_REMOTE% ...
%ADB% shell "rm -rf \"%LOGSTORE_REMOTE%/\"" 2>&1
echo [操作记录] 完成
timeout /t 2 >nul
goto MENU

:: =========================
:: 功能2 - 仅删除 sirunlog 文件
:: =========================
:FUNC_DELETE_SIRUN_ONLY
echo.
echo [操作记录] 正在删除设备上的 sirunlog 文件：%SIRUN_REMOTE%
%ADB% shell "rm -f \"%SIRUN_REMOTE%\"" 2>&1
if %errorlevel% equ 0 (
    echo [操作记录] 删除成功
) else (
    echo [操作记录] 删除失败，可能文件不存在或权限不足
)
timeout /t 2 >nul
goto MENU

:: =========================
:: 功能3 - 提取 sirunlog 并重命名
:: =========================
:FUNC_PULL_SIRUN_RENAME
echo.
call :MAKE_TIME TS
set "OUTNAME=sirunlog_%TS%.log"
echo [操作记录] 正在从设备拉取 %SIRUN_REMOTE% 到本地 %OUTNAME% ...
%ADB% pull "%SIRUN_REMOTE%" "%OUTNAME%" 2>&1
if %errorlevel% neq 0 (
    echo [操作记录] 拉取失败，请确认路径 %SIRUN_REMOTE% 是否存在
) else (
    echo [操作记录] 已保存为 %OUTNAME%
)
timeout /t 2 >nul
goto MENU

:: =========================
:: 功能4 - 提取并用 Notepad++ 打开
:: =========================
:FUNC_PULL_SIRUN_OPEN
echo.
call :MAKE_TIME TS
set "OUTNAME=sirunlog_%TS%.log"
echo [操作记录] 正在拉取 %SIRUN_REMOTE% ...
%ADB% pull "%SIRUN_REMOTE%" "%OUTNAME%" 2>&1
if %errorlevel% neq 0 (
    echo [操作记录] 拉取失败，请检查设备与路径
    timeout /t 2 >nul
    goto MENU
)
if exist "%NOTEPADPP_EXE%" (
    start "" "%NOTEPADPP_EXE%" "%OUTNAME%"
    echo [操作记录] 已使用 Notepad++ 打开 %OUTNAME%
) else (
    echo [操作记录] 未找到 Notepad++，使用系统记事本打开...
    start "" notepad "%OUTNAME%"
)
timeout /t 2 >nul
goto MENU

:: =========================
:: 功能5 - 读取数据库并提取关键字段
:: =========================
:FUNC_READ_DB
echo.
set "ALL_OUT=%TEMP%\params_all.txt"
set "FILTER=%TEMP%\params_filter.txt"
echo [操作记录] 正在从设备读取数据库...
%ADB% shell "cd \"%PARAM_DIR%\" && sqlite3 %PARAM_DB% 'select * from paramstbl;'" > "%ALL_OUT%" 2>&1
if %errorlevel% neq 0 (
    echo [操作记录] 读取数据库失败，请确认路径和 sqlite3 可用
    pause
    goto MENU
)
echo.
echo ====== 全量数据库内容 ======
type "%ALL_OUT%"
echo ============================
echo.
echo 正在筛选关键字段（ICCID IMEI IMSI SN VIN SW_Version）...
findstr /I "ICCID IMEI IMSI SN VIN SW_Version" "%ALL_OUT%" > "%FILTER%" 2>nul
if not exist "%FILTER%" (
    echo 未找到关键字段
    pause
    goto MENU
)
echo.
echo ====== 关键字段（美化显示） ======
for /f "usebackq delims=" %%L in ("%FILTER%") do (
    set "line=%%L"
    for /f "tokens=2,3 delims=|" %%a in ("!line!") do (
        set "name=%%a"
        set "value=%%b"
        set "name=!name:E_ParamId_=!"
        echo !name!: !value!
    )
)
echo ================================
pause
goto MENU

:: =========================
:: 功能6/7/8 - 升级
:: =========================
:FUNC_UPGRADE_MCU
call :DO_UPGRADE 1
goto MENU
:FUNC_UPGRADE_MPU
call :DO_UPGRADE 2
goto MENU
:FUNC_UPGRADE_ALL
call :DO_UPGRADE 3
goto MENU

:DO_UPGRADE
echo.
set "MODE=%1"
set "ZIPFILE="
for /f "delims=" %%Z in ('dir /b *.zip 2^>nul') do (
    set "ZIPFILE=%%Z"
    goto ZIP_FOUND
)
:ZIP_FOUND
if "%ZIPFILE%"=="" (
    echo [操作记录] 当前目录未找到 .zip 升级包，请放到脚本同目录
    pause
    goto MENU
)
echo [操作记录] 找到升级包：%ZIPFILE%
echo [操作记录] 正在推送至设备...
%ADB% push "%ZIPFILE%" "/sdcard/selfUpgrade/%ZIPFILE%" 2>&1
if %errorlevel% neq 0 (
    echo [操作记录] 推送失败
    pause
    goto MENU
)
echo [操作记录] 正在执行升级脚本（模式 %MODE%）...
%ADB% shell "%UPGRADE_SCRIPT% %MODE%" 2>&1
echo [操作记录] 升级命令已发送
pause
goto MENU

:: =========================
:: 功能9 - 导出最近6个日志（固定临时路径，避免权限bug）
:: =========================
:FUNC_EXPORT_6LOGS
echo.
call :MAKE_TIME TS
set "OUTDIR=logs_%TS%"
md "%OUTDIR%" >nul 2>&1
echo [操作记录] 创建本地文件夹：%OUTDIR%
echo.
echo [操作记录] 正在拉取 sirunlog ...
%ADB% pull "%SIRUN_REMOTE%" "%OUTDIR%\sirunlog_%TS%.log" >nul 2>&1
echo [操作记录] sirunlog 拉取完成
echo.
echo [操作记录] 正在拉取并自动解压最近5个tgz ...
set "TMP=C:\temp\tgz_list.txt"
md "C:\temp" >nul 2>&1  :: 确保 C:\temp 存在

%ADB% shell "ls -1t \"%LOGSTORE_REMOTE%\" | grep '.tgz$' | head -n 5" > "%TMP%"

:: 纯 batch 去除隐藏字符
set COUNT=0
pushd "%OUTDIR%" >nul
for /f "usebackq delims=" %%F in ("%TMP%") do (
    set "TGZ=%%F"
    call set "TGZ=!TGZ:~0,-1!"
    call set "TGZ=!TGZ:~0,-1!"
    for /f "tokens=* delims= " %%A in ("!TGZ!") do set "TGZ=%%A"

    set /a COUNT+=1
    if !COUNT! GTR 5 goto DONE_EXPORT

    echo     [!COUNT!] 拉取 !TGZ! ...
    %ADB% pull "%LOGSTORE_REMOTE%/!TGZ!" "!TGZ!" >nul 2>&1

    7z x "!TGZ!" -y >nul 2>&1
    for %%T in (*.tar) do 7z x "%%T" -y >nul 2>&1

    for /r %%L in (*.log) do (
        if not exist "!TGZ:~0,-4!.log" (
            move "%%L" "!TGZ:~0,-4!.log" >nul 2>&1
        )
    )

    del /q "!TGZ!" "*.tar" "*.gz" >nul 2>&1
)
:DONE_EXPORT

for /f "delims=" %%D in ('dir /ad /b 2^>nul') do rd /s /q "%%D" >nul 2>&1

popd >nul
del "%TMP%" >nul 2>&1

echo.
echo [操作记录] 全部完成！文件夹 %OUTDIR% 内现在有6个 .log 文件
dir "%OUTDIR%" /b
timeout /t 3 >nul
goto MENU
:: -------------------------
:: 子程序：生成时间字符串（格式 MMdd-HHmmss）
:: -------------------------
:MAKE_TIME
for /f "usebackq delims=" %%T in (`powershell -NoProfile -Command "Get-Date -Format 'MMdd-HHmmss'"`) do set "%1=%%T"
goto :EOF