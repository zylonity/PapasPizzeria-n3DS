# Go to the root folder where "CustomerParts" resides
Set-Location "C:\Users\Khaleel\source\repos\PapasPizzeria_vsc\data"

# For each 'sprites' folder inside CustomerParts subfolders...
Get-ChildItem -Path ".\CustomerParts" -Recurse -Directory -Filter "sprites" | 
ForEach-Object {
    $spritesFolder = $_.FullName
    $parentFolder  = $_.Parent.FullName

    # Move contents of "sprites" to the parent folder
    Move-Item -Path (Join-Path $spritesFolder "*") -Destination $parentFolder -Force

    # Remove the (now empty) "sprites" folder
    Remove-Item $spritesFolder -Recurse -Force
}
