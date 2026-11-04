#requires -Version 5.1
<#
ALENCAR LASER 19.9 - benchmark QT 6 Bezier com projeto LightBurn real.
OFFLINE: usa copia temporaria, nao transmite comandos e nao altera o original.
Default: selecao otimizada; -ModoLegado ativa rebuild global apenas na selecao.
A/B: usar o MESMO arquivo de origem, janela/DPI e condicoes do Windows.
Exemplos:
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\BENCHMARK_BEZIER_REAL_19_9.ps1"
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\BENCHMARK_BEZIER_REAL_19_9.ps1" -ModoLegado -SemCompilar
#>
[CmdletBinding()]
param(
 [string]$ArquivoLbrn2 = '',
 [switch]$ModoLegado,
 [switch]$SemCompilar
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$envPathBefore = $env:PATH
$legacyBefore = [Environment]::GetEnvironmentVariable(
    'ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9','Process')
$sessionCopy = ''
$originalSource = ''
$originalHash = ''
try {
 if ([string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
  Add-Type -AssemblyName System.Windows.Forms
  $dialog = New-Object System.Windows.Forms.OpenFileDialog
  $dialog.Title = 'Selecione o projeto LightBurn para benchmark offline (sera criada uma copia)'
  $dialog.Filter = 'Projetos LightBurn (*.lbrn2)|*.lbrn2'
  $dialog.CheckFileExists = $true
  $dialog.Multiselect = $false
  try {
   if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
    Write-Host 'Cancelado. Nada foi executado ou modificado.' -ForegroundColor Yellow
    return
   }
   $ArquivoLbrn2 = $dialog.FileName
  }
  finally { $dialog.Dispose() }
 }
 $originalSource = (Resolve-Path -LiteralPath $ArquivoLbrn2).Path
 if (-not (Test-Path -LiteralPath $originalSource -PathType Leaf) -or
     [System.IO.Path]::GetExtension($originalSource) -ine '.lbrn2') {
  throw 'Arquivo de origem deve existir e possuir extensao .lbrn2'
 }
 $originalHash = (Get-FileHash -LiteralPath $originalSource -Algorithm SHA256).Hash
 $sourceSize = (Get-Item -LiteralPath $originalSource).Length
 if ($sourceSize -le 0 -or $sourceSize -gt 64MB) {
  throw 'Arquivo de origem fora do limite de 64 MiB'
 }
 $runtime = 'C:\msys64\ucrt64\bin'
 $exe = Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_GIT_19_8\alencar_test7_qt6.exe'
 $gate = Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
 if (-not $SemCompilar) {
  if (-not (Test-Path -LiteralPath $gate -PathType Leaf)) {
   throw "Gate Qt ausente: $gate"
  }
  # The synthetic test requires the optimized policy, regardless of the
  # desired A/B mode used afterwards by the real project benchmark.
  $env:ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9 = '0'
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $gate -SomenteBezierNativoUI
  if ($LASTEXITCODE -ne 0) { throw "Compilacao/gate Qt falhou: $LASTEXITCODE" }
 }
 if (-not (Test-Path -LiteralPath $exe -PathType Leaf) -or
     -not (Test-Path -LiteralPath (Join-Path $runtime 'libminizip-1.dll') -PathType Leaf)) {
  throw 'Binario Qt ou runtime MSYS2 UCRT64 ausente; compile o teste antes'
 }
 $env:PATH = $runtime + ';' + $envPathBefore
 $env:ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9 = if ($ModoLegado) { '1' } else { '0' }
 $mode = if ($ModoLegado) { 'LEGADO' } else { 'OTIMIZADO' }
 $timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'
 $dest = Join-Path $root "RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9\$mode-$timestamp"
 New-Item -Path $dest -ItemType Directory -Force | Out-Null
 $sessionCopy = Join-Path $dest 'ENTRADA_TEMPORARIA_BENCHMARK.lbrn2'
 Copy-Item -LiteralPath $originalSource -Destination $sessionCopy -ErrorAction Stop
 $copyHash = (Get-FileHash -LiteralPath $sessionCopy -Algorithm SHA256).Hash
 if ($copyHash -ne $originalHash) { throw 'Copia temporaria nao confere com origem' }
 Write-Host "Benchmark QT offline — modo $mode" -ForegroundColor Cyan
 Write-Host "Origem protegida por SHA256: $originalHash"
 Write-Host 'Nenhuma conexao com laser sera iniciada.' -ForegroundColor Yellow
 & $exe --native-paint-bench-19-9-test $sessionCopy $dest
 $exitCode = $LASTEXITCODE
 $reportPath = Join-Path $dest 'RESULTADO_PERFORMANCE_BEZIER_REAL_19_9.json'
 if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
  throw "Benchmark nao produziu JSON: $reportPath (exit $exitCode)"
 }
 $report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
 if ($exitCode -ne 0 -or $report.status -ne 'PASS') {
  throw "Benchmark falhou (exit $exitCode): $($report.error)"
 }
 if (-not $report.source_sha256_unchanged -or
     -not $report.geometry_unchanged -or
     $report.machine_commands_sent -or $report.fabrication_enabled) {
  throw 'Benchmark nao satisfez as travas de integridade / seguranca'
 }
 Write-Host "PASS $mode : $($report.contours) contornos, paint P95 $($report.paint_p95_ms) ms" -ForegroundColor Green
 Write-Host "Selecao: $($report.selection_total_ms) ms / $($report.selection_cycles) ciclos"
 Write-Host "Full path builds: $($report.native_full_path_builds_delta) / selective: $($report.native_selective_path_builds_delta)"
 Write-Host "JSON de diagnostico: $reportPath"
}
finally {
 if ($sessionCopy -and (Test-Path -LiteralPath $sessionCopy -PathType Leaf)) {
  Remove-Item -LiteralPath $sessionCopy -Force
 }
 $env:PATH = $envPathBefore
 [Environment]::SetEnvironmentVariable(
  'ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9',$legacyBefore,'Process')
 if ($originalSource -and $originalHash) {
  $afterHash = (Get-FileHash -LiteralPath $originalSource -Algorithm SHA256).Hash
  if ($afterHash -ne $originalHash) {
   throw 'ALERTA CRITICO: hash de origem mudou. Interrompa os testes.'
  }
  Write-Host 'Integridade da origem: SHA256 preservado.' -ForegroundColor Green
 }
}
