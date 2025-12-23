@echo off
chcp 65001 >nul
title GitHub Push with Retry
color 0A

setlocal enabledelayedexpansion

set REMOTE_URL=https://github.com/wuyuanwuhui999/qt-chat-desktop.git
set BRANCH=main
set MAX_RETRIES=10
set RETRY_DELAY=10
set retry_count=0
set success=0

echo ========================================
echo        GitHub Push with Retry
echo ========================================
echo.

:: 检查是否已初始化git仓库
if not exist ".git" (
    echo 错误：当前目录不是Git仓库！
    echo 请确保在正确的目录中运行此脚本。
    pause
    exit /b 1
)

:: 设置远程仓库
echo [1/4] 设置远程仓库...
git remote remove origin 2>nul
git remote add origin %REMOTE_URL%
if errorlevel 1 (
    echo 错误：设置远程仓库失败！
    pause
    exit /b 1
)
echo 远程仓库设置成功！

:: 重命名分支
echo [2/4] 重命名分支...
git branch -M %BRANCH%
if errorlevel 1 (
    echo 警告：重命名分支可能失败，继续执行...
)

echo [3/4] 开始推送代码到GitHub...
echo 最大重试次数：%MAX_RETRIES%
echo 重试间隔：%RETRY_DELAY%秒
echo.

:retry_loop
set /a retry_count+=1
echo ========================================
echo 尝试第 !retry_count! 次推送 [%time%]
echo ========================================

:: 执行推送
git push -u origin %BRANCH%

if errorlevel 1 (
    echo.
    echo 推送失败！错误代码：%errorlevel%
    
    if !retry_count! lss %MAX_RETRIES% (
        echo 将在 %RETRY_DELAY% 秒后重试...
        echo 剩余重试次数：%MAX_RETRIES%-!retry_count!
        
        :: 显示倒计时
        for /l %%i in (%RETRY_DELAY%, -1, 1) do (
            set /p ="等待 %%i 秒..." < nul
            ping -n 2 127.0.0.1 > nul
            set /p =< nul
        )
        echo.
        goto retry_loop
    ) else (
        echo.
        echo ========================================
        echo 推送失败！
        echo 已达到最大重试次数 (%MAX_RETRIES% 次)
        echo ========================================
        pause
        exit /b 1
    )
) else (
    set success=1
)

if !success! equ 1 (
    echo.
    echo ========================================
    echo 推送成功！✓
    echo ========================================
    echo 远程仓库：%REMOTE_URL%
    echo 分支：%BRANCH%
    echo 完成时间：%date% %time%
    echo ========================================
    
    :: 等待2秒后自动关闭
    timeout /t 2 /nobreak > nul
    exit
)