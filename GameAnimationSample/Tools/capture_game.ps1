param(
  [string]$Name = "cap",
  [string]$ExecCmds = "rr.test.autoplay",
  [string]$WaitFor = "Match state -> 1",
  [int]$ExtraSeconds = 7,
  [int]$TimeoutSeconds = 240,
  [string]$OutDir = "",
  [string]$ExtraArgs = "",
  [string]$ShotTimes = ""   # e.g. "2,5,9": seconds after the wait pattern; one PNG per entry (Name_1.png ...). Overrides ExtraSeconds.
)
# Launches the game windowed, waits for a log line, screenshots the primary monitor (once or at several times), kills the game.
$ws = Split-Path -Parent $PSScriptRoot
$log = "$ws\Docs\reports\$Name.log"
Get-Process UnrealEditor* -ErrorAction SilentlyContinue | Stop-Process -Force
Remove-Item $log -ErrorAction SilentlyContinue
$args = @("`"$ws\GameAnimationSample.uproject`"", "-game", "-windowed", "-ResX=960", "-ResY=540", "-WinX=0", "-WinY=0", "-nosound", "-nosplash", "-log", "-forcelogflush", "-ABSLOG=`"$log`"")
if ($ExtraArgs) { $args += $ExtraArgs.Split(" ") }
if ($ExecCmds) { $args += "-ExecCmds=`"$ExecCmds`"" }
$p = Start-Process -FilePath "A:\UNREAL\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList $args -PassThru
$deadline = (Get-Date).AddSeconds($TimeoutSeconds)
while ((Get-Date) -lt $deadline) {
  Start-Sleep 2
  if ((Test-Path $log) -and (Select-String -Path $log -Pattern $WaitFor -Quiet)) { break }
}
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
if (-not $OutDir) { $OutDir = "$env:TEMP" }
function Shot($suffix) {
  $b = [System.Windows.Forms.SystemInformation]::PrimaryMonitorSize
  $bmp = New-Object Drawing.Bitmap $b.Width, $b.Height
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen(0, 0, 0, 0, $bmp.Size)
  $out = Join-Path $OutDir "$Name$suffix.png"
  $bmp.Save($out, [Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  $out
}
if ($ShotTimes) {
  $t0 = Get-Date
  $i = 0
  foreach ($t in $ShotTimes.Split(",")) {
    $i++
    $wait = [double]$t - ((Get-Date) - $t0).TotalSeconds
    if ($wait -gt 0) { Start-Sleep -Milliseconds ([int]($wait * 1000)) }
    Shot "_$i"
  }
} else {
  Start-Sleep $ExtraSeconds
  Shot ""
}
Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
