Add-Type -AssemblyName System.Windows.Forms
$wshell = New-Object -ComObject Wscript.Shell
$proc = Get-Process -Name UnrealEditor -ErrorAction SilentlyContinue | Select-Object -First 1
if ($proc) {
    Write-Output ("Found UnrealEditor with PID " + $proc.Id)
    $wshell.AppActivate($proc.Id)
    Start-Sleep -Milliseconds 500
    [System.Windows.Forms.SendKeys]::SendWait('^%{F11}')
    Write-Output "Successfully sent Ctrl+Alt+F11 to UnrealEditor!"
} else {
    Write-Output "UnrealEditor not running"
}
