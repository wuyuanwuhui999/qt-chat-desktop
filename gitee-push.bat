@echo off
chcp 65001 >nul
echo ================================
echo Gitee 代码推送脚本（SSH方式）
echo 专用仓库：qt-chat-desktop
echo ================================
echo.

REM 强制设置为SSH地址
set "SSH_REPO=git@gitee.com:wuyuanwuhui99/qt-chat-desktop.git"
echo 设置远程仓库为SSH地址...
git remote remove origin 2>nul
git remote add origin "%SSH_REPO%"
echo 远程地址已设置为：%SSH_REPO%
echo.

REM 启动SSH代理并添加密钥
echo 初始化SSH连接...
ssh-add "%USERPROFILE%\.ssh\id_ed25519" 2>nul
if errorlevel 1 (
    echo SSH密钥未加载，启动代理...
    start "" "%ProgramFiles%\Git\bin\ssh-agent.exe"
    timeout /t 2 >nul
    ssh-add "%USERPROFILE%\.ssh\id_ed25519" 2>nul
)

REM 测试SSH连接
echo.
echo 测试SSH连接...
ssh -T git@gitee.com 2>nul
if errorlevel 1 (
    echo ⚠ SSH连接测试失败
    echo 请确认：
    echo 1. SSH公钥已添加到Gitee
    echo 2. 公钥：%USERPROFILE%\.ssh\id_ed25519.pub
    echo 3. 在 https://gitee.com/profile/sshkeys 添加公钥
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

REM 推送代码（带重试机制）
echo.
echo 正在推送到远程仓库 %BRANCH_TO_PUSH% 分支...
set MAX_RETRIES=5
set RETRY_DELAY=5
set retry_count=0
set success=0

:push_retry
set /a retry_count+=1
echo.
echo ================================
echo 第 !retry_count! 次尝试推送 [%time%]
echo ================================

git push -u origin %BRANCH_TO_PUSH%

if errorlevel 1 (
    echo.
    echo 推送失败！错误代码：%errorlevel%
    
    if !retry_count! lss %MAX_RETRIES% (
        echo 将在 %RETRY_DELAY% 秒后重试...
        echo 剩余重试次数：%MAX_RETRIES%-!retry_count!
        
        REM 倒计时
        for /l %%i in (%RETRY_DELAY%, -1, 1) do (
            echo 等待 %%i 秒...
            timeout /t 1 >nul
        )
        goto push_retry
    ) else (
        echo.
        echo ================================
        echo 推送失败！
        echo 已达到最大重试次数 (%MAX_RETRIES% 次)
        echo ================================
        echo 可能原因：
        echo 1. 没有提交内容（先运行 git add 和 git commit）
        echo 2. 分支名称不匹配
        echo 3. 网络问题
        echo.
        echo 解决方案：
        echo 1. 检查是否有提交：git status
        echo 2. 创建并提交更改：
        echo    git add .
        echo    git commit -m "提交说明"
        echo 3. 或切换到HTTPS方式
        pause
        exit /b 1
    )
) else (
    set success=1
)

if !success! equ 1 (
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