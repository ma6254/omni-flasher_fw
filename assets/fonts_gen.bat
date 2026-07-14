@echo off
setlocal

@rem 生成字体文件（基于项目根目录的绝对路径，避免工作目录导致路径错误）
chcp 65001 >nul

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%.."

where lv_font_conv >nul 2>nul
if errorlevel 1 (
    echo [ERROR] 未找到 lv_font_conv，请先 ' npm install -g lv_font_conv@latest '
    exit /b 1
)

call lv_font_conv ^
    --font "%ROOT_DIR%\assets\fonts\AlibabaPuHuiTi-3\AlibabaPuHuiTi-3-65-Medium\AlibabaPuHuiTi-3-65-Medium.ttf" ^
    --size 20 --bpp 4 --format lvgl --no-compress ^
    --output "%ROOT_DIR%\assets\fonts\c\main_menu_screen_item_font.c" ^
    --range 32-127 ^
    --symbols 串口系统设置信息
if errorlevel 1 (
    echo [ERROR] 生成 main_menu_screen_item_font.c 失败。
    exit /b 1
)

call lv_font_conv ^
    --font "%ROOT_DIR%\assets\fonts\AlibabaPuHuiTi-3\AlibabaPuHuiTi-3-65-Medium\AlibabaPuHuiTi-3-65-Medium.ttf" ^
    --size 20 --bpp 4 --format lvgl --no-compress ^
    --output "%ROOT_DIR%\assets\fonts\c\settings_screen_item_font.c" ^
    --range 32-127 ^
    --symbols 设置语言屏幕亮度蜂鸣器开机动画系统信息关于
if errorlevel 1 (
    echo [ERROR] 生成 settings_screen_item_font.c 失败。
    exit /b 1
)

call lv_font_conv ^
    --font "%ROOT_DIR%\assets\fonts\AlibabaPuHuiTi-3\AlibabaPuHuiTi-3-65-Medium\AlibabaPuHuiTi-3-65-Medium.ttf" ^
    --size 20 --bpp 4 --format lvgl --no-compress ^
    --output "%ROOT_DIR%\assets\fonts\c\serial_screen_item_font.c" ^
    --range 32-127 ^
    --symbols 转串口带硬件流控记录器双路三四
if errorlevel 1 (
    echo [ERROR] 生成 serial_screen_item_font.c 失败。
    exit /b 1
)

echo Fonts generation completed.
endlocal
