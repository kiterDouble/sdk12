@echo off
:: 设置终端字符集为 UTF-8，防止中文乱码
chcp 65001 >nul
title 固件合并构建工具

echo =========================================
echo       开始执行固件处理自动化脚本
echo =========================================

:: 提示用户输入版本号，防空回车保护
set "version="
set /p version=请输入版本号 (例如 001，直接回车默认001): 
if "%version%"=="" set "version=001"
echo [当前版本号设为] V%version%
echo.

:: 切换到脚本实际所在的目录
cd /d "%~dp0"
echo [当前工作路径] %cd%
echo.

:: ==========================================
:: 步骤 1：删除脚本路径下的所有 hex, bin, srec 文件
:: ==========================================
echo [步骤 1] 正在清理旧的固件文件...
del /q /f *.hex *.bin *.srec 2>nul
echo - 清理完成。
echo.

:: ==========================================
:: 步骤 2：拷贝 APP 固件到脚本路径并重命名为 app.hex
:: ==========================================
echo [步骤 2] 正在获取 APP 固件...
set "APP_SRC=..\AC78428_APP\Release\Exe\AC78428_APP.hex"

if not exist "%APP_SRC%" (
    echo [错误] 找不到 APP 固件文件！
    echo 检查路径: "%APP_SRC%"
    goto :ErrorExit
)
:: 拷贝并重命名，确保下游 Python 脚本能读到 app.hex
copy /y "%APP_SRC%" .\app.hex >nul
echo - APP 固件拷贝成功。
echo.

:: ==========================================
:: 步骤 3：拷贝 BOOT 固件到脚本路径并重命名为 boot.hex
:: ==========================================
echo [步骤 3] 正在获取 BOOT 固件...
set "BOOT_SRC=..\AC78428_BOOT\Debug\Exe\AC78428_BOOT.hex"

if not exist "%BOOT_SRC%" (
    echo [错误] 找不到 BOOT 固件文件！
    echo 检查路径: "%BOOT_SRC%"
    goto :ErrorExit
)
:: 拷贝并重命名，确保下游 Python 脚本能读到 boot.hex
copy /y "%BOOT_SRC%" .\boot.hex >nul
echo - BOOT 固件拷贝成功。
echo.

:: ==========================================
:: 步骤 4：执行 getboot_app.py 脚本
:: ==========================================
echo [步骤 4] 开始执行 Python 合并脚本 (getboot_app.py)...
if not exist "getboot_app.py" (
    echo [错误] 在当前目录下找不到 getboot_app.py ！
    goto :ErrorExit
)

:: 直接调用 python 执行（已移除密钥参数）
python getboot_app.py

:: 检查 Python 脚本的返回值，如果不是 0 说明 py 脚本内部报错了
if %errorlevel% neq 0 (
    echo.
    echo [错误] Python 脚本执行异常中止！错误码: %errorlevel%
    goto :ErrorExit
)

echo.

:: ==========================================
:: 步骤 5：清理中间过程产生的 Hex 文件
:: ==========================================
echo [步骤 5] 正在清理中间文件...
del /q /f app.hex bankA.hex bankB.hex boot.hex boot_signed.hex app_signed.hex 2>nul
echo - 中间文件清理完成。
echo.

:: ==========================================
:: 步骤 6：使用 WMIC 获取标准时间并重命名 boot_app.hex
:: ==========================================
echo [步骤 6] 正在重命名 boot_app.hex 为时间戳版本...

:: 使用 wmic 获取不受语言和系统设置影响的纯数字时间 (格式: YYYYMMDDHHMMSS.xxxxxx±xxx)
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set datetime=%%I

set "year=%datetime:~0,4%"
set "month=%datetime:~4,2%"
set "day=%datetime:~6,2%"
set "hour=%datetime:~8,2%"
set "minute=%datetime:~10,2%"

:: 拼接最终的文件名，例如 boot_app_L23_2026_05_09_14_17_V001
set "datever=boot_app_T08_%year%_%month%_%day%_%hour%_%minute%_V%version%"

if exist "boot_app.hex" (
    ren boot_app.hex "%datever%.hex"
    echo - 成功重命名为: %datever%.hex
) else (
    echo [警告] 找不到 boot_app.hex 文件，跳过重命名步骤。
)
echo.

:: ==========================================
:: 步骤 7：将 app.srec 重命名为 mcu_1.bin 复制一份为 mcu_2.bin 移动到动态时间戳 mcu 文件夹下
:: ==========================================
echo [步骤 7] 正在处理并打包固件到 mcu 文件夹...

set "mcubin=mcu_bin_%year%_%month%_%day%_%hour%_%minute%_V%version%"

if exist "app.srec" (
    echo - 正在重命名 app.srec 为 mcu_1.bin...
    ren app.srec mcu_1.bin
    
    echo - 正在复制 mcu_1.bin 为 mcu_2.bin...
    copy mcu_1.bin mcu_2.bin >nul
    
    echo - 正在创建带时间戳和版本号的 mcu 文件夹...
    if not exist "%mcubin%\" mkdir "%mcubin%"
    
    echo - 正在将 bin 文件移动到 %mcubin% 文件夹下...
    move mcu_1.bin "%mcubin%\" >nul
    move mcu_2.bin "%mcubin%\" >nul
    
    echo - 操作成功：所有文件已归档到 %mcubin% 文件夹。
) else (
    echo [警告] 找不到 app.srec 文件，跳过步骤 7。
)
echo.

echo =========================================
echo          所有步骤执行完毕，成功！
echo =========================================
pause
exit /b 0

:ErrorExit
echo.
echo =========================================
echo          流程因错误中断，请检查！
echo =========================================
pause
exit /b 1