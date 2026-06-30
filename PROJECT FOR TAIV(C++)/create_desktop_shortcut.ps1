$ErrorActionPreference = "Stop"

$ProjectFolder = Split-Path -Parent $MyInvocation.MyCommand.Path
$ExePath = Join-Path $ProjectFolder "TaivAccountTracker.exe"

if (!(Test-Path $ExePath)) {
    Write-Host "TaivAccountTracker.exe was not found."
    Write-Host "Build the app first using build_msvc.bat or build_mingw.bat."
    exit 1
}

$Desktop = [Environment]::GetFolderPath("Desktop")
$ShortcutPath = Join-Path $Desktop "Taiv Account Tracker.lnk"

$Shell = New-Object -ComObject WScript.Shell
$Shortcut = $Shell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $ExePath
$Shortcut.WorkingDirectory = $ProjectFolder
$Shortcut.Description = "Taiv Account Tracker - C++ desktop app by Andrii Pavlenko"
$Shortcut.IconLocation = "$ExePath,0"
$Shortcut.Save()

Write-Host "Desktop shortcut created:"
Write-Host $ShortcutPath
