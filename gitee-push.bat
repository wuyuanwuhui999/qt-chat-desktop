@echo off
chcp 65001 >nul
echo ================================
echo Gitee 代码推送脚本（SSH方式）
echo 专用仓库：qt-chat-desktop
echo ================================
echo.

REM 检查Git是否安装
where git >nul 2>nul
if errorlevel 1 (
    echo 错误：Git未安装或未添加到PATH环境变量！
    echo 请安装Git并确保在PATH中
    echo 您的Git安装目录：E:\Git
    pause
    exit /b 1
)

REM 强制设置为SSH地址
set "SSH_REPO=git@gitee.com:wuyuanwuhui99/qt-chat-desktop.git"
echo 设置远程仓库为SSH地址...
git remote remove origin 2>nul
git remote add origin "%SSH_REPO%"
echo 远程地址已设置为：%SSH_REPO%
echo.

REM 方法1：直接使用Git Bash中的ssh命令（推荐）
echo 初始化SSH连接...
echo 方法1：尝试直接使用SSH密钥...

REM 使用SSH命令测试连接
echo.
echo 测试SSH连接...
"E:\Git\bin\ssh.exe" -T git@gitee.com
if errorlevel 1 (
    echo.
    echo ⚠ SSH连接测试失败
    echo 请确认：
    echo 1. SSH公钥已添加到Gitee
    echo 2. 公钥位置：%USERPROFILE%\.ssh\id_ed25519.pub
    echo 3. 公钥已添加到：https://gitee.com/profile/sshkeys
    echo.
    
    REM 方法2：尝试使用Pageant（如果使用PuTTY）
    echo 尝试方法2：检查Pageant...
    tasklist | findstr /i pageant >nul
    if errorlevel 1 (
        echo Pageant未运行，尝试加载OpenSSH密钥...
        
        REM 方法3：使用Windows OpenSSH服务
        where ssh-add >nul 2>nul
        if not errorlevel 1 (
            echo 使用Windows OpenSSH...
            ssh-add "%USERPROFILE%\.ssh\id_ed25519"
        ) else (
            echo 请手动启动SSH代理：
            echo 1. 打开Git Bash
            echo 2. 运行: eval $(ssh-agent -s)
            echo 3. 运行: ssh-add ~/.ssh/id_ed25519
        )
    ) else (
        echo Pageant已运行，SSH密钥应该已加载
    )
    
    echo.
    REM 再次测试连接
    echo 再次测试SSH连接...
    "E:\Git\bin\ssh.exe" -T git@gitee.com
    if errorlevel 1 (
        echo ⚠ SSH连接仍然失败
        echo 建议使用Git Bash手动测试
    ) else (
        echo ✓ SSH连接测试通过
    )
) else (
    echo ✓ SSH连接测试通过
)

REM 设置分支（先检查当前分支）
echo.
echo 检查分支配置...
for /f "tokens=*" %%i in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%i"
if "%CURRENT_BRANCH%"=="" (
    echo 当前没有分支，使用默认分支 main
    git branch -M main 2>nul
    set "BRANCH_TO_PUSH=main"
) else (
    echo 当前分支：%CURRENT_BRANCH%
    set "BRANCH_TO_PUSH=%CURRENT_BRANCH%"
)

REM 检查是否有提交
echo.
echo 检查是否有待推送的提交...
git log --oneline origin/%BRANCH_TO_PUSH%..%BRANCH_TO_PUSH% >nul 2>nul
if errorlevel 1 (
    echo ⚠ 没有新的提交需要推送
    echo 请先执行：
    echo   git add .
    echo   git commit -m "提交说明"
    echo.
    set /p "CONTINUE=是否继续尝试推送？(y/n): "
    if /i not "!CONTINUE!"=="y" (
        echo 操作取消
        pause
        exit /b 0
    )
)

REM 推送代码（带重试机制）
echo.
echo 正在推送到远程仓库 %BRANCH_TO_PUSH% 分支...
set MAX_RETRIES=3
set RETRY_DELAY=5
set retry_count=0
set success=0

:push_retry
set /a retry_count+=1
echo.
echo ================================
echo 第 %retry_count% 次尝试推送 [%time%]
echo ================================

git push -u origin %BRANCH_TO_PUSH%

if errorlevel 1 (
    echo.
    echo 推送失败！错误代码：%errorlevel%
    
    if %retry_count% lss %MAX_RETRIES% (
        echo 将在 %RETRY_DELAY% 秒后重试...
        echo 剩余重试次数：%MAX_RETRIES%-%retry_count%
        
        REM 倒计时
        for /l %%i in (%RETRY_DELAY%, -1, 1) do (
            echo 等待 %%i 秒...
            ping -n 2 127.0.0.1 >nul
        )
        goto push_retry
    ) else (
        echo.
        echo ================================
        echo 推送失败！
        echo 已达到最大重试次数 (%MAX_RETRIES% 次)
        echo ================================
        echo.
        echo 常见解决方案：
        echo 1. 确保有提交内容：
        echo    git add .
        echo    git commit -m "提交说明"
        echo.
        echo 2. 检查分支名称：
        echo    当前分支: %BRANCH_TO_PUSH%
        echo    远程分支: origin/%BRANCH_TO_PUSH%
        echo.
        echo 3. 手动推送命令：
        echo    git push -u origin %BRANCH_TO_PUSH%
        echo.
        echo 4. 或使用HTTPS方式：
        echo    git remote set-url origin https://gitee.com/wuyuanwuhui99/qt-chat-desktop.git
        pause
        exit /b 1
    )
) else (
    set success=1
)

if %success% equ 1 (
    echo.
    echo ================================
    echo ✅ 推送成功！
    echo ================================
    echo 远程仓库：%SSH_REPO%
    echo 分    支：%BRANCH_TO_PUSH%
    echo 完成时间：%date% %time%
    echo ================================
    
    REM 等待3秒后自动关闭
    timeout /t 3 >nul
)