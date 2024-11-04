@echo off
setlocal

rem DSLiteのパスを設定
set DSLITE_PATH=C:\ti\ccs1280\ccs\ccs_base\DebugServer\bin\DSLite.exe

rem FTDIドライバーのパスを環境変数に追加
set PATH=%PATH%;C:\ti\ccs1280\ccs\ccs_base\emulation\windows\ftdi_drivers\i386

rem ターゲット設定ファイル
set CCXML_FILE=%~dp0blinky_smooth\target_config.ccxml

rem 出力ファイル
set OUT_FILE=%~dp0blinky_smooth\Debug\blinky_smooth.out

rem デバッグセッションの開始
echo Starting debug session...
"%DSLITE_PATH%" load -c "%CCXML_FILE%" "%OUT_FILE%" -g debug.log

rem デバッグセッションが終了したらクリーンアップ
echo Debug session ended
endlocal