#requires -Version 5.1
<#
ALENCAR LASER 19.9 - Qt DeviceCoordinateCache A/B, offline.
Uses a single .lbrn2 source, protects it with SHA256, and compares
NoCache versus DeviceCoordinateCache with the same Qt binary.
Requires local Qt6/MSYS2 and machine disconnected.
#>
[CmdletBinding()]
param([string]$ArquivoLbrn2 = '',[switch]$SemCompilar,[switch]$AuditarZoom)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$runner = Join-Path $PSScriptRoot 'BENCHMARK_BEZIER_REAL_19_9.ps1'
$gate = Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
$original = ''
$sourceHash = ''
# Streaming, read-only topology guard. The editable native importer does NOT
# preserve nested groups: reject early instead of silently flattening them.
function Test-Lbrn2Groups199 {
 param([string]$FilePath)
 $settings = New-Object System.Xml.XmlReaderSettings
 $settings.DtdProcessing = [System.Xml.DtdProcessing]::Prohibit
 $settings.XmlResolver = $null
 $settings.MaxCharactersInDocument = [long](32MB)
 $xmlReader = [System.Xml.XmlReader]::Create($FilePath,$settings)
 $shapes = 0
 $groups = 0
 $nested = 0
 $insideShapes = 0
 $validRoot = $false
 try {
  while ($xmlReader.Read()) {
   if ($xmlReader.NodeType -eq [System.Xml.XmlNodeType]::Element) {
    if ($xmlReader.Depth -eq 0) {
     if ($xmlReader.LocalName -ne 'LightBurnProject') {
      throw 'Nao e um LightBurnProject XML valido'
     }
     $validRoot = $true
    }
    if ($xmlReader.LocalName -eq 'Shape') {
     $shapes++
     if ($insideShapes -gt 0) { $nested++ }
     if ($xmlReader.GetAttribute('Type') -eq 'Group') { $groups++ }
     if (-not $xmlReader.IsEmptyElement) { $insideShapes++ }
    }
   } elseif ($xmlReader.NodeType -eq [System.Xml.XmlNodeType]::EndElement -and
             $xmlReader.LocalName -eq 'Shape') {
    $insideShapes--
   }
  }
 } finally { $xmlReader.Dispose() }
 if (-not $validRoot) { throw 'LightBurnProject XML nao encontrado' }
 return [pscustomobject]@{ Shapes=$shapes; Groups=$groups; NestedShapes=$nested }
}
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
 $size = (Get-Item -LiteralPath $original).Length
 if ($size -le 0 -or $size -gt 32MB) {
  throw 'Arquivo fora do limite de 32 MiB do importador nativo'
 }
 $sourceHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
 Write-Host "Origem protegida: $sourceHash" -ForegroundColor Cyan
 $topology = Test-Lbrn2Groups199 -FilePath $original
 Write-Host ("Inspecao offline: {0} Shape(s), {1} Group(s), {2} Shape(s) aninhados" -f
   $topology.Shapes,$topology.Groups,$topology.NestedShapes) -ForegroundColor Cyan
 if ($topology.Groups -gt 0 -or $topology.NestedShapes -gt 0) {
  Write-Warning 'Arquivo com grupos: suporte nativo de hierarquia ainda nao homologado.'
  throw 'Benchmark cancelado antes de compilar. Selecione a mesma copia .lbrn2 SEM grupos usada no teste anterior; NAO desagrupe o original.'
 }
 if ($topology.Shapes -lt 1) { throw 'Projeto LightBurn sem Shape para benchmark' }
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
  if ($AuditarZoom -and $case.Name -eq 'CACHE_DEVICE') { $cmd += '-AuditarZoom' }
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
  $zoomWanted = $cacheWanted -and [bool]$AuditarZoom
  if ([bool]$j.native_cache_zoom_audit_enabled -ne $zoomWanted) {
   throw "Zoom audit flag nao aplicado em $($case.Name)"
  }
  if ($zoomWanted -and @($j.native_cache_zoom_audit).Count -ne 3) {
   throw 'Auditoria de zoom nao gerou as tres escalas previstas'
  }
  if ($zoomWanted) {
   foreach ($z in $j.native_cache_zoom_audit) {
    if ([int]$z.visible_native_items -le 0 -or
        [double]$z.native_ink_pixels -lt 32) {
     throw ("Zoom {0}x sem arte vetorial suficiente na viewport" -f $z.relative_zoom_factor)
    }
   }
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
   zoom_audit = @($j.native_cache_zoom_audit)
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
 $zoomCacheCoverage = $true
 if ($AuditarZoom) {
  foreach ($z in $b.zoom_audit) {
   if ([double]$z.cache_items -le 0) { $zoomCacheCoverage = $false }
  }
 }
 $zoomStatus = 'NOT_RUN'
 if ($AuditarZoom) {
  if ($zoomCacheCoverage) { $zoomStatus = 'MEASURED_VISUAL_REVIEW_PENDING' }
  else { $zoomStatus = 'INCONCLUSIVE_NO_CACHE_AT_SOME_ZOOMS' }
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
  zoom_cache_coverage_complete = [bool]$zoomCacheCoverage
  zoom_audit_status = $zoomStatus
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
 if ($AuditarZoom -and -not $zoomCacheCoverage) {
  Write-Warning 'Cache ausente em pelo menos uma escala; nao homologar a fluidez em todos os zooms.'
 }
 Write-Host "Diferenca visual (pixels >8): $($b.cache_visual_pixels_over8_percent)% ; max canal $($b.cache_visual_max_channel_difference)"
 if ($AuditarZoom) {
  foreach ($z in $b.zoom_audit) {
   Write-Host ("Zoom {0}x: vetores visiveis {1}, arte pixels {2}, cache itens {3}, P95 sem cache {4} ms / com cache {5} ms, pixels >8 {6}%, revisar {7}" -f
     $z.relative_zoom_factor,$z.visible_native_items,$z.native_ink_pixels,
     $z.cache_items,$z.no_cache_paint_p95_ms,$z.cached_paint_p95_ms,
     $z.visual_pixels_over8_percent,$z.requires_visual_review)
  }
 }
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
