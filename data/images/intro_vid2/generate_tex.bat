@echo off
setlocal enabledelayedexpansion

:: Loop through the first 39 .t3s files in the folder
for /l %%G in (1,1,39) do (
    set "InputFile=C:\Users\Khaleel\source\repos\PapasPizzeria\data\images\intro_vid\introvideo_part%%G.t3s"
    set "OutputFile=C:\Users\Khaleel\source\repos\PapasPizzeria\data\images\intro_vid\t3x\introvideo_part%%G.t3x"

    :: Print the input and output files
    echo Processing Input: !InputFile!
    echo Output: !OutputFile!

    :: Run the tex3ds command
    C:\Users\Khaleel\source\repos\PapasPizzeria\data\images\tex3ds.exe -i "!InputFile!" -o "!OutputFile!"
)

pause