@echo off

:: Save the current folder name to a variable
for %%F in (.) do set TexFileName=%%~nF

::Print folder name
echo %TexFileName%

:: Navigate back to the parent directory
cd ..

:: Replace placeholders with the variable and execute the command
set "InputFile=.\%TexFileName%\%TexFileName%.t3s"
echo %InputFile%
set "OutputFile=..\..\romfs\gfx\%TexFileName%.t3x"
echo %OutputFile%

tex3ds.exe -i %InputFile% -o %OutputFile%

:: Pause to keep the console window open
pause
