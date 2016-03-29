@echo off
for /f "tokens=*" %%i in ("make") do set /p "var=%%i " <nul >temp.bat
for /f "eol=; tokens=*" %%i in (command.txt) do set /p "var=%%i " <nul >>temp.bat
for /f "tokens=*" %%i in ("2>error.txt") do set /p "var=%%i " <nul >>temp.bat
echo.>>temp.bat
echo.pause>>temp.bat
echo.del temp.bat>>temp.bat
temp.bat
