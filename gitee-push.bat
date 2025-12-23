@echo off
chcp 65001 >nul
echo ================================
echo Gitee 推送脚本（简化版）
echo ================================
echo.

REM 设置Git路径
set "GIT_PATH=E:\Git\bin\git.exe"
set "SSH_PATH=E:\Git\usr\bin\ssh.exe"

REM 设置仓库
set "GITEE_REPO=git@gitee.com:wuyuanwuhui99/qt-chat-desktop.git"

echo 配置远程仓库...
"%GIT_PATH%" remote remove origin 2>nul
"%GIT_PATH%" remote add origin "%GITEE_REPO%"
echo 远程仓库：%GITEE_REPO%
echo.

echo 测试SSH连接...
"%SSH_PATH%" -T git@gitee.com
if errorlevel 1 (
    echo ❌ SSH连接失败！
    echo.
    echo 解决方案：
    echo 1. 确保SSH密钥已添加到Gitee
    echo 2. 手动在Git Bash中运行：
    echo    ssh -T git@gitee.com
    echo 3. 或切换为HTTPS方式：
    echo    git remote set-url origin https://gitee.com/wuyuanwuhui99/qt-chat-desktop.git
    echo.
    pause
    exit /b 1
)

echo ✓ SSH连接成功
echo.

echo 推送代码到Gitee...
set RETRY_COUNT=3
set DELAY=5

:push_loop
"%GIT_PATH%" push -u origin main
if errorlevel 1 (
    set /a RETRY_COUNT-=1
    if %RETRY_COUNT% gtr 0 (
        echo 推送失败，%DELAY%秒后重试...
        timeout /t %DELAY% >nul
        goto push_loop
    ) else (
        echo.
        echo ❌ 推送失败！
        echo 请检查：git status
        pause
        exit /b 1
    )
)

echo.
echo ✅ 推送成功！
echo 仓库：%GITEE_REPO%
echo 分支：main
timeout /t 2 >nul