# Change to the parent directory that contains "CustomerParts"
Set-Location "C:\Users\Khaleel\source\repos\PapasPizzeria_vsc\gfx"

# Collect all subfolders inside "CustomerParts" (these are the "X" folders)
$folders = Get-ChildItem -Path ".\CustomerParts" -Directory

# Define the relative paths you want inside each .t3s file
$paths = @(
    "back_hair/1.png",
    "body/1.png",
    "eyes/1.png",
    "eyes/2.png",
    "eyes/3.png",
    "eyes/4.png",
    "eyes/5.png",
    "eyes/6.png",
    "eyes/7.png",
    "eyes/8.png",
    "eyes/9.png",
    "eyes/10.png",
    "foot/1.png",
    "foot/2.png",
    "foot/3.png",
    "forearm/1.png",
    "hair/1.png",
    "hand/1.png",
    "hand/2.png",
    "hand2/1.png",
    "hand2/2.png",
    "hand2/3.png",
    "head/1.png",
    "mouth/1.png",
    "mouth/2.png",
    "mouth/3.png",
    "mouth/4.png",
    "mouth/5.png",
    "mouth/6.png",
    "mouth/7.png",
    "mouth/8.png",
    "mouth/9.png",
    "mouth/10.png",
    "mouth/11.png",
    "mouth/12.png",
    "mouth/13.png",
    "mouth/14.png",
    "mouth/15.png",
    "mouth/16.png",
    "mouth/17.png",
    "mouth/18.png",
    "mouth/19.png",
    "mouth/20.png",
    "mouth/21.png",
    "neck/1.png",
    "upperarm/1.png"
)

foreach ($folder in $folders) {
    # folder.Name is the "X" in CustomerParts\X

    # Create the .t3s filename
    $t3sFile = "customer$($folder.Name).t3s"

    # Start building content for the t3s file
    $content = @()
    # 1) Add the top line(s)
    $content += "--atlas -f rgba -z auto"
    $content += ""  # blank line for readability

    # 2) Add each line referencing the images with the correct path
    foreach ($p in $paths) {
        $content += "./CustomerParts/$($folder.Name)/$p"
    }

    # Write all lines to the .t3s file
    Set-Content -Path $t3sFile -Value $content

    Write-Host "Created $t3sFile"
}

Write-Host "All .t3s files created!"
