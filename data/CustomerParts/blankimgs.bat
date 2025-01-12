@echo off

REM ---------------------------------------------
REM Set the root directory containing "CustomerParts"
REM ---------------------------------------------
set "ROOT=C:\Users\Khaleel\source\repos\PapasPizzeria_vsc\gfx"

REM ---------------------------------------------
REM Loop through each subfolder under CustomerParts
REM (these folders are your "X" folders).
REM ---------------------------------------------
for /D %%A in ("%ROOT%\CustomerParts\*") do (
    REM Does the folder actually exist?
    if exist "%%~A" (
        echo Processing folder: %%~A

        REM -----------------------------------------------------
        REM 1) Proceed only if "back_hair" folder is there
        REM -----------------------------------------------------
        if exist "%%~A\back_hair" (
            REM Create 1.png if it doesn't already exist
            if not exist "%%~A\back_hair\1.png" (
                magick -size 1x1 canvas:transparent "%%~A\back_hair\1.png"
                echo   Created 1.png in %%~A\back_hair
            ) else (
                echo   1.png already exists in %%~A\back_hair, skipping...
            )
        ) else (
            echo   No back_hair folder in %%~A, skipping...
        )

        REM -----------------------------------------------------
        REM 2) Check the "eyes" folder; create missing 1-10.png
        REM -----------------------------------------------------
        if exist "%%~A\eyes" (
            echo   Checking for missing eye images in %%~A\eyes...
            for /L %%i in (1,1,10) do (
                if not exist "%%~A\eyes\%%i.png" (
                    magick -size 1x1 canvas:transparent "%%~A\eyes\%%i.png"
                    echo       Created %%i.png in %%~A\eyes
                ) else (
                    echo       %%i.png already exists, skipping...
                )
            )
        ) else (
            echo   No eyes folder in %%~A, skipping...
        )
        echo.
    )
)

echo Done!
pause
