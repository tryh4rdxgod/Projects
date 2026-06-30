$ErrorActionPreference = "Stop"

$ProjectFolder = Split-Path -Parent $MyInvocation.MyCommand.Path
$IndexPath = Join-Path $ProjectFolder "index.html"

if (!(Test-Path $IndexPath)) {
    Write-Host "index.html was not found."
    exit 1
}

$Desktop = [Environment]::GetFolderPath("Desktop")
$ShortcutPath = Join-Path $Desktop "Taiv Account Tracker Studio.lnk"

$Shell = New-Object -ComObject WScript.Shell
$Shortcut = $Shell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $IndexPath
$Shortcut.WorkingDirectory = $ProjectFolder
$Shortcut.Description = "Taiv Account Tracker Studio - polished account coordination demo"
$Shortcut.Save()

Write-Host "Desktop shortcut created:"
Write-Host $ShortcutPath
