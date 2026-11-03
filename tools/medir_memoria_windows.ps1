# Ferramenta de medicao passiva: nao envia comandos a maquina ou ao aplicativo.
# Windows PowerShell 5.1+
# Exemplo:
#   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\medir_memoria_windows.ps1 -TargetProcessIds 1234 -DurationSeconds 60
# Para arquitetura hibrida: -TargetProcessIds 1234,5678 (PID do Qt e do Python).
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [ValidateNotNullOrEmpty()]
    [int[]]$TargetProcessIds,
    [ValidateRange(2,3600)]
    [int]$DurationSeconds = 60,
    [ValidateRange(100,5000)]
    [int]$IntervalMs = 500,
    [string]$OutputCsv = ""
)
$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($OutputCsv)) {
    $OutputCsv = Join-Path (Get-Location).Path ("alencar_laser_memoria_" + (Get-Date -Format "yyyyMMdd_HHmmss") + ".csv")
}
$parent = Split-Path -Parent $OutputCsv
if ($parent -and -not (Test-Path -LiteralPath $parent)) { throw "A pasta do CSV nao existe: $parent" }
$ids = @($TargetProcessIds | Sort-Object -Unique)
$records = New-Object 'System.Collections.Generic.List[object]'
$watch = [Diagnostics.Stopwatch]::StartNew()
$missingCount = 0
while ($watch.Elapsed.TotalSeconds -lt $DurationSeconds) {
    $stamp = (Get-Date).ToUniversalTime().ToString("o")
    $elapsed = [Math]::Round($watch.Elapsed.TotalSeconds, 3)
    $totalPrivate = [int64]0
    $totalWorking = [int64]0
    $found = 0
    foreach ($id in $ids) {
        $p = Get-Process -Id $id -ErrorAction SilentlyContinue
        if ($null -eq $p) {
            $missingCount++
            continue
        }
        $found++
        # Refresh is essential when polling an existing Process object.
        $p.Refresh()
        $totalPrivate += [int64]$p.PrivateMemorySize64
        $totalWorking += [int64]$p.WorkingSet64
        $records.Add([pscustomobject]@{
            Utc=$stamp; ElapsedSeconds=$elapsed; Kind="Process"; PID=$p.Id; Name=$p.ProcessName
            WorkingSetMB=[Math]::Round($p.WorkingSet64/1MB,2)
            PrivateMB=[Math]::Round($p.PrivateMemorySize64/1MB,2)
            CPUSeconds=[Math]::Round($p.TotalProcessorTime.TotalSeconds,3)
            ThreadCount=$p.Threads.Count
        })
    }
    if ($found -eq 0) {
        if ($records.Count -eq 0) { throw "Nao encontrei nenhum dos PIDs informados." }
        break
    }
    $records.Add([pscustomobject]@{
        Utc=$stamp; ElapsedSeconds=$elapsed; Kind="Total"; PID=0; Name="Qt_mais_worker"
        WorkingSetMB=[Math]::Round($totalWorking/1MB,2)
        PrivateMB=[Math]::Round($totalPrivate/1MB,2)
        CPUSeconds=$null; ThreadCount=$null
    })
    Start-Sleep -Milliseconds $IntervalMs
}
$watch.Stop()
if ($records.Count -eq 0) { throw "Nenhuma amostra foi coletada." }
$records | Export-Csv -Path $OutputCsv -NoTypeInformation -Encoding UTF8
$totals = @($records | Where-Object { $_.Kind -eq "Total" })
$maxWorking = ($totals | Measure-Object -Property WorkingSetMB -Maximum).Maximum
$maxPrivate = ($totals | Measure-Object -Property PrivateMB -Maximum).Maximum
$avgPrivate = ($totals | Measure-Object -Property PrivateMB -Average).Average
Write-Host "Relatorio: $OutputCsv"
Write-Host ("Tempo amostrado: {0:N1}s | amostras totais: {1}" -f $watch.Elapsed.TotalSeconds,$totals.Count)
Write-Host ("RAM total pico working set: {0:N2} MB" -f $maxWorking)
Write-Host ("RAM total pico privada: {0:N2} MB | media privada: {1:N2} MB" -f $maxPrivate,$avgPrivate)
if ($missingCount -gt 0) { Write-Warning "Alguns PIDs encerraram ou nao foram encontrados; conferir CSV." }
Write-Host "Compare dois CSVs no mesmo computador, arquivo e sequencia de acoes."
