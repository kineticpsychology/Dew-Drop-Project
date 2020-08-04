@@echo off
copy .\DewDropThemer.exe .\DewDropThemer.Compressed.exe
upx --ultra-brute DewDropThemer.Compressed.exe
echo.
echo.
echo Done
pause
exit
