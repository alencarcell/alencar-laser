#requires -Version 5.1
<#
ALENCAR LASER 19.9 - Medidor de RAM/CPU do processo Windows.
Somente leitura. Nao modifica projeto, sistema ou equipamento laser.
Exemplo:
  powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\MEDIR_MEMORIA_WINDOWS_19_9.ps1 -DuracaoSegundos 180
  powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\MEDIR_MEMORIA_WINDOWS_19_9.ps1 -ProcessoId 12345
#>
[CmdletBinding()]
param(
 [string]$NomeProcesso = 'alencar_test7_qt6',
 [ValidateRange(0,2147483647)][int]$ProcessoId = 0,
 [ValidateRange(1,86400)][int]$DuracaoSegundos = 180,
 [ValidateRange(250,60000)][int]$IntervaloMs = 1000,
 [string]$SaidaCsv = ''
)
$ErrorActionPreference = 'Stop'
if ($ProcessoId -eq 0) {
 $candidatos = @(Get-Process -Name $NomeProcesso -ErrorAction SilentlyContinue)
 if ($candidatos.Count -eq 0) {
  throw "Processo $NomeProcesso nao encontrado. Abra a versao experimental antes de medir."
 }
 if ($candidatos.Count -gt 1) {
  throw "Mais de um processo $NomeProcesso aberto. Especifique -ProcessoId para evitar amostras incorretas."
 }
 $ProcessoId = [int]$candidatos[0].Id
}
$raiz = Split-Path -Parent $PSScriptRoot
$pasta = Join-Path $raiz 'RELATORIOS\DESEMPENHO'
[void](New-Item -Path $pasta -ItemType Directory -Force)
if ([string]::IsNullOrWhiteSpace($SaidaCsv)) {
 $SaidaCsv = Join-Path $pasta ("MEMORIA_CPU_19_9_{0}_PID{1}.csv" -f (Get-Date -Format 'yyyyMMdd_HHmmss'),$ProcessoId)
}
$arquivo = [System.IO.Path]::GetFullPath($SaidaCsv)
$diretorio = [System.IO.Path]::GetDirectoryName($arquivo)
if (!(Test-Path $diretorio)) {[void](New-Item -Path $diretorio -ItemType Directory -Force)}
$cultura = [System.Globalization.CultureInfo]::InvariantCulture
$codificacao = New-Object System.Text.UTF8Encoding($false)
$gravador = New-Object System.IO.StreamWriter($arquivo,$false,$codificacao)
$anteriorCpu = $null
$anteriorTempo = $null
$inicio = [DateTime]::UtcNow
$contador = 0
try {
 $gravador.WriteLine('timestamp_utc,pid,working_set_mb,private_bytes_mb,cpu_percent_total,threads,handles')
 while (([DateTime]::UtcNow-$inicio).TotalSeconds -lt $DuracaoSegundos) {
  $processo = Get-Process -Id $ProcessoId -ErrorAction SilentlyContinue
  if ($null -eq $processo) {Write-Warning 'Processo encerrado; coleta finalizada.';break}
  $agora = [DateTime]::UtcNow
  $cpuAtual = [double]$processo.CPU
  $usoCpu = 0.0
  if ($null -ne $anteriorCpu) {
   $delta = ($agora-$anteriorTempo).TotalSeconds
   if ($delta -gt 0) {
    $usoCpu = [Math]::Max(0,100.0*($cpuAtual-$anteriorCpu)/($delta*[Environment]::ProcessorCount))
   }
  }
  $working = [double]$processo.WorkingSet64/1MB
  $private = [double]$processo.PrivateMemorySize64/1MB
  $cols = @(
   $agora.ToString('o'),
   $ProcessoId.ToString($cultura),
   $working.ToString('F3',$cultura),
   $private.ToString('F3',$cultura),
   $usoCpu.ToString('F2',$cultura),
   $processo.Threads.Count.ToString($cultura),
   $processo.HandleCount.ToString($cultura)
  )
  $gravador.WriteLine(($cols -join ','))
  ++$contador
  $anteriorCpu=$cpuAtual
  $anteriorTempo=$agora
  Start-Sleep -Milliseconds $IntervaloMs
 }
}
finally {
 $gravador.Flush()
 $gravador.Dispose()
}
Write-Host "ALENCAR LASER: medicao concluida. Amostras: $contador" -ForegroundColor Green
Write-Host "CSV: $arquivo"
Write-Host 'Compare com diagnosticos JSON da mesma copia de projeto e os mesmos gestos.'
