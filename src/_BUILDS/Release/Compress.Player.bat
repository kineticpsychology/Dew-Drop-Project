@@echo off
copy .\DewDropPlayer.exe .\DewDropPlayer.Compressed.exe
upx --ultra-brute DewDropPlayer.Compressed.exe
echo.
echo.
echo Done
pause
exit
