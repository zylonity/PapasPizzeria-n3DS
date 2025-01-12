# Change to the parent directory that contains "CustomerParts"
Set-Location "C:\Users\Khaleel\source\repos\PapasPizzeria_vsc\gfx"

# Collect all subfolders inside "CustomerParts" (these are the "X" folders)
$folders = Get-ChildItem -Path ".\CustomerParts" -Directory

foreach ($folder in $folders) {
    Write-Host "Processing folder $($folder.Name)..."

    # 1) Path to 'back_hair' subfolder
    $backHairPath = Join-Path $folder.FullName "back_hair"
    
    # 2) Create the 'back_hair' folder if it doesn't exist
    if (-not (Test-Path $backHairPath)) {
        New-Item -ItemType Directory -Path $backHairPath | Out-Null
        Write-Host "  Created folder: $backHairPath"
    }

    # 3) Create a 1x1 blank PNG using ImageMagick, overwriting if it exists
    $imageFile = Join-Path $backHairPath "1.png"
    
    # Make sure your path to magick.exe is correct; might be "C:\Program Files\ImageMagick-7.1.0-Q16-HDRI\magick.exe"
    & "C:\Program Files\ImageMagick\magick.exe" -size 1x1 canvas:transparent "$imageFile"
    
    Write-Host "  Created blank 1.png in: $backHairPath"
}

Write-Host "Done creating back_hair folder and blank images!"
