#requires -Version 5.1
<#
ALENCAR LASER 19.9 - Qt DeviceCoordinateCache A/B, offline.
Uses a single .lbrn2 source, protects it with SHA256, and compares
NoCache versus DeviceCoordinateCache with the same Qt binary.
Requires local Qt6/MSYS2 and machine disconnected.
#>
[CmdletBinding()]
param([string]$ArquivoLbrn2 = '',[switch]$SemCompilar)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$runner = Join-Path $PSScriptRoot 'BENCHMARK_BEZIER_REAL_19_9.ps1'
$gate = Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
$original = ''
$sourceHash = ''
try {
 if (-not (Test-Path -LiteralPath $runner -PathType Leaf)) {
  throw "Runner Qt ausente: $runner"
 }
 if ([string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
  Add-Type -AssemblyName System.Windows.Forms
  $dialog = New-Object System.Windows.Forms.OpenFileDialog
  $dialog.Title = 'Selecione uma origem LightBurn para ambas as provas de cache'
  $dialog.Filter = 'Projeto LightBurn (*.lbrn2)|*.lbrn2'
  $dialog.CheckFileExists = $true
  try {
   if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
    Write-Host 'Benchmark cancelado. Nenhum arquivo modificado.' -ForegroundColor Yellow
    return
   }
   $ArquivoLbrn2 = $dialog.FileName
  }
  finally { $dialog.Dispose() }
 }
 $original = (Resolve-Path -LiteralPath $ArquivoLbrn2).Path
 if ([IO.Path]::GetExtension($original) -ine '.lbrn2') { throw 'Extensao invalida' }
 if ((Get-Item -LiteralPath $original).Length -gt 64MB) { throw 'Arquivo muito grande' }
 $sourceHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
 Write-Host "Origem protegida: $sourceHash" -ForegroundColor Cyan
 if (-not $SemCompilar) {
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $gate -SomenteBezierNativoUI
  if ($LASTEXITCODE -ne 0) { throw 'Gate Qt sintetico falhou. Teste nao iniciado.' }
 }
 $cases = @(
  [pscustomobject]@{ Name = 'SEM_CACHE'; Args = @() },
  [pscustomobject]@{ Name = 'CACHE_DEVICE'; Args = @('-CacheDispositivo') }
 )
 $results = @{}
 foreach ($case in $cases) {
  if ((Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash -ne $sourceHash) {
   throw 'Origem alterada antes do teste'
  }
  Write-Host "TESTE $($case.Name)" -ForegroundColor Cyan
  $cmd = @('-NoProfile','-ExecutionPolicy','Bypass','-File',
           $runner,'-ArquivoLbrn2',$original,'-SemCompilar') + $case.Args
  $lines = @(& powershell.exe @cmd)
  foreach ($line in $lines) { Write-Host ([string]$line) }
  if ($LASTEXITCODE -ne 0) { throw "Runner $($case.Name) falhou" }
  $paths = @($lines | ForEach-Object {
   $lineText = [string]$_
   if ($lineText -match '^JSON de diagnostico:\s*(.+)$') { $Matches[1].Trim() }
  })
  if ($paths.Count -ne 1 -or -not (Test-Path -LiteralPath $paths[0] -PathType Leaf)) {
   throw "JSON nao localizado em $($case.Name)"
  }
  $j = Get-Content -LiteralPath $paths[0] -Raw | ConvertFrom-Json
  if ($j.status -ne 'PASS' -or -not $j.geometry_unchanged -or
      -not $j.source_sha256_unchanged -or $j.machine_commands_sent -or
      $j.fabrication_enabled -or $j.legacy_selection_rebuild) {
   throw "Falha de integridade, seguranca ou modo seletivo em $($case.Name)"
  }
  $cacheWanted = ($case.Name -eq 'CACHE_DEVICE')
  if ([bool]$j.native_device_cache_benchmark -ne $cacheWanted) {
   throw "Cache flag nao aplicado em $($case.Name)"
  }
  if ((Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash -ne $sourceHash) {
   throw 'Origem alterada depois do teste'
  }
  $results[$case.Name] = [pscustomobject]@{
   report_path = [string]$paths[0]
   source_sha256 = $sourceHash
   contours = [int]$j.contours
   qt = [string]$j.qt
   dpr = [double]$j.dpr
   viewport_width_px = [int]$j.viewport_width_px
   viewport_height_px = [int]$j.viewport_height_px
   paint_samples = [int]$j.paint_samples
   paint_p50_ms = [double]$j.paint_p50_ms
   paint_p95_ms = [double]$j.paint_p95_ms
   paint_p99_ms = [double]$j.paint_p99_ms
   paint_without_native_items_p95_ms = [double]$j.paint_without_native_items_p95_ms
   native_device_cache_items = [double]$j.native_device_cache_items
   native_device_cache_estimated_bytes = [double]$j.native_device_cache_estimated_bytes
   cache_visual_pixels_over8_percent = [double]$j.cache_visual_pixels_over8_percent
   cache_visual_max_channel_difference = [int]$j.cache_visual_max_channel_difference
   rss_bytes_after_benchmark = [double]$j.rss_bytes_after_benchmark
  }
 }
 $a = $results['SEM_CACHE']
 $b = $results['CACHE_DEVICE']
 foreach ($field in @('source_sha256','contours','qt','dpr',
                       'viewport_width_px','viewport_height_px','paint_samples')) {
  if ($a.$field -ne $b.$field) { throw "A/B invalido: $field divergente" }
 }
 if ($b.native_device_cache_items -lt 1) {
  throw 'Nenhum item recebeu cache. Alterar limites somente apos auditar geometria.'
 }
 $reduction = if ($a.paint_p95_ms -gt 0) {
  100.0 * ($a.paint_p95_ms - $b.paint_p95_ms) / $a.paint_p95_ms
 } else { 0.0 }
 $report = [pscustomobject]@{
  status = 'PASS'
  experiment = 'native-device-coordinate-cache-19.9'
  source_sha256 = $sourceHash
  matching_source = $true
  no_cache = $a
  device_cache = $b
  paint_p95_reduction_percent = [math]::Round($reduction,2)
  source_intact = $true
  machine_commands_sent = $false
 }
 $destination = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9'
 New-Item -ItemType Directory -Path $destination -Force | Out-Null
 $summary = Join-Path $destination ("COMPARACAO_CACHE_19_9_" + (Get-Date -Format 'yyyyMMdd_HHmmss') + '.json')
 [IO.File]::WriteAllText($summary,($report | ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
 Write-Host ("CACHE A/B PASS: paint P95 sem {0:N2} ms; com {1:N2} ms; delta {2:N2}%" -f
    $a.paint_p95_ms,$b.paint_p95_ms,$reduction) -ForegroundColor Green
 Write-Host "Itens cacheados: $($b.native_device_cache_items); RSS (bytes) $($b.rss_bytes_after_benchmark)"
 Write-Host "Diferenca visual (pixels >8): $($b.cache_visual_pixels_over8_percent)% ; max canal $($b.cache_visual_max_channel_difference)"
 Write-Host "Relatorio: $summary"
}
finally {
 if ($original -and $sourceHash) {
  if ((Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash -ne $sourceHash) {
   throw 'ALERTA: hash da origem mudou; interrompa imediatamente.'
  }
  Write-Host 'Origem SHA256 preservada.' -ForegroundColor Green
 }
}
