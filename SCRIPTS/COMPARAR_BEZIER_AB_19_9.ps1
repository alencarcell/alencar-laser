#requires -Version 5.1
<#
Alencar Laser 19.9 - read-only paired benchmark on ONE LightBurn source.
Select .lbrn2 once; invoke both render modes with the identical source,
record source SHA256 and compare only matching geometry/viewport/Qt.
All user geometry remains local; no controller, GRBL or laser commands.
#>
[CmdletBinding()]
param(
 [string]$ArquivoLbrn2 = '',
 [ValidateRange(1,5)][int]$Repeticoes = 1,
 [switch]$SemCompilar
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$runner = Join-Path $PSScriptRoot 'BENCHMARK_BEZIER_REAL_19_9.ps1'
$original = ''
$initialHash = ''
if (-not (Test-Path -LiteralPath $runner -PathType Leaf)) {
 throw "Runner experimental nao encontrado: $runner"
}
try {
 if ([string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
  Add-Type -AssemblyName System.Windows.Forms
  $dialog = New-Object System.Windows.Forms.OpenFileDialog
  $dialog.Title = 'Selecione UMA origem .lbrn2 para ambos os modos A/B'
  $dialog.Filter = 'Projetos LightBurn (*.lbrn2)|*.lbrn2'
  $dialog.CheckFileExists = $true
  try {
   if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
    Write-Host 'Cancelado sem executar os benchmarks.' -ForegroundColor Yellow
    return
   }
   $ArquivoLbrn2 = $dialog.FileName
  }
  finally { $dialog.Dispose() }
 }
 $original = (Resolve-Path -LiteralPath $ArquivoLbrn2).Path
 if ([IO.Path]::GetExtension($original) -ine '.lbrn2') {
  throw 'Origem deve ser um arquivo .lbrn2'
 }
 $initialHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
 $sourceSize = (Get-Item -LiteralPath $original).Length
 if ($sourceSize -le 0 -or $sourceSize -gt 64MB) {
  throw 'Origem fora do limite de 64 MiB'
 }
 Write-Host "Fonte A/B fixada. SHA256: $initialHash" -ForegroundColor Cyan
 if (-not $SemCompilar) {
  $gate = Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
  if (-not (Test-Path -LiteralPath $gate -PathType Leaf)) {
   throw "Gate de compilacao nao encontrado: $gate"
  }
  $previousLegacy = [Environment]::GetEnvironmentVariable(
      'ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9','Process')
  try {
   $env:ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9 = '0'
   & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $gate -SomenteBezierNativoUI
   if ($LASTEXITCODE -ne 0) {
    throw "Compilacao/teste sintetico falhou (codigo $LASTEXITCODE)"
   }
  }
  finally {
   [Environment]::SetEnvironmentVariable(
     'ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9',$previousLegacy,'Process')
  }
 }
 $allPairs = New-Object System.Collections.Generic.List[object]
 $stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
 $rootReports = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9'
 New-Item -ItemType Directory -Path $rootReports -Force | Out-Null
 $summaryPath = Join-Path $rootReports ("COMPARACAO_AB_19_9_" + $stamp + '.json')
 for ($round = 1; $round -le $Repeticoes; $round++) {
  # Alternate execution order to reduce warm-cache and scheduling bias.
  $modes = if (($round % 2) -eq 1) {
   @('OTIMIZADO','LEGADO')
  } else {
   @('LEGADO','OTIMIZADO')
  }
  $recorded = @{}
  foreach ($mode in $modes) {
   $hashBefore = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
   if ($hashBefore -ne $initialHash) {
    throw "Origem alterada antes do modo $mode. Comparacao cancelada."
   }
   $params = @('-NoProfile','-ExecutionPolicy','Bypass','-File',
               $runner,'-ArquivoLbrn2',$original,'-SemCompilar')
   if ($mode -eq 'LEGADO') { $params += '-ModoLegado' }
   Write-Host ("RODADA {0}/{1}: {2}" -f $round,$Repeticoes,$mode) -ForegroundColor Cyan
   $lines = @(& powershell.exe @params)
   $exit = $LASTEXITCODE
   foreach ($line in $lines) { Write-Host ([string]$line) }
   if ($exit -ne 0) {
    throw "Runner $mode falhou com codigo de saida $exit"
   }
   $reports = @($lines | ForEach-Object {
    $lineText = [string]$_
    if ($lineText -match '^JSON de diagnostico:\s*(.+)$') {
     $Matches[1].Trim()
    }
   })
   if ($reports.Count -ne 1 -or
       -not (Test-Path -LiteralPath $reports[0] -PathType Leaf)) {
    throw "Relatorio JSON ausente ou ambiguo para $mode"
   }
   $data = Get-Content -LiteralPath $reports[0] -Raw | ConvertFrom-Json
   if ($data.status -ne 'PASS' -or
       -not $data.geometry_unchanged -or
       -not $data.source_sha256_unchanged -or
       $data.machine_commands_sent -or $data.fabrication_enabled) {
    throw "Falha na verificacao de integridade e seguranca: $mode"
   }
   if ([bool]$data.legacy_selection_rebuild -ne ($mode -eq 'LEGADO')) {
    throw "O executavel nao usou o modo $mode solicitado"
   }
   if ((Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash -ne $initialHash) {
    throw "SHA256 de origem mudou apos o modo $mode"
   }
   $recorded[$mode] = [pscustomobject]@{
    mode = $mode
    source_sha256 = $initialHash
    contours = [int]$data.contours
    qt = [string]$data.qt
    viewport_width_px = [int]$data.viewport_width_px
    viewport_height_px = [int]$data.viewport_height_px
    dpr = [double]$data.dpr
    paint_samples = [int]$data.paint_samples
    paint_p50_ms = [double]$data.paint_p50_ms
    paint_p95_ms = [double]$data.paint_p95_ms
    paint_p99_ms = [double]$data.paint_p99_ms
    background_p95_ms = [double]$data.background_p95_ms
    foreground_p95_ms = [double]$data.foreground_p95_ms
    paint_without_native_items_p95_ms = if ($data.PSObject.Properties.Name -contains 'paint_without_native_items_p95_ms') { [double]$data.paint_without_native_items_p95_ms } else { $null }
    selection_total_ms = [double]$data.selection_total_ms
    selection_cycles = [int]$data.selection_cycles
    native_full_path_builds_delta = [double]$data.native_full_path_builds_delta
    native_selective_path_builds_delta = [double]$data.native_selective_path_builds_delta
    report_path = [string]$reports[0]
   }
  }
  $a = $recorded['OTIMIZADO']
  $b = $recorded['LEGADO']
  if ($null -eq $a -or $null -eq $b) {
   throw 'Par A/B incompleto'
  }
  foreach ($field in @('source_sha256','contours','qt','viewport_width_px',
                       'viewport_height_px','dpr','paint_samples','selection_cycles')) {
   if ($a.$field -ne $b.$field) {
    throw "Comparacao A/B recusada: $field difere entre os dois testes"
   }
  }
  if ($a.native_full_path_builds_delta -ne 0 -or
      $b.native_full_path_builds_delta -le 0) {
   throw 'Politica de rebuild inesperada no par A/B'
  }
  $selectionSaved = if ($b.selection_total_ms -gt 0) {
   100.0 * ($b.selection_total_ms - $a.selection_total_ms) / $b.selection_total_ms
  } else { 0.0 }
  $paintSaved = if ($b.paint_p95_ms -gt 0) {
   100.0 * ($b.paint_p95_ms - $a.paint_p95_ms) / $b.paint_p95_ms
  } else { 0.0 }
  $allPairs.Add([pscustomobject]@{
    round = $round
    optimized = $a
    legacy = $b
    selection_time_reduction_percent = [math]::Round($selectionSaved,2)
    paint_p95_reduction_percent = [math]::Round($paintSaved,2)
  })
  Write-Host ("PAR {0} PASS: selecao otimizada {1:N2} ms vs legado {2:N2} ms (reducao {3:N2}%); pintura P95 otimizada {4:N2} ms vs legado {5:N2} ms (reducao {6:N2}%)" -f
              $round,$a.selection_total_ms,$b.selection_total_ms,$selectionSaved,
              $a.paint_p95_ms,$b.paint_p95_ms,$paintSaved) -ForegroundColor Green
  if ($null -ne $a.paint_without_native_items_p95_ms) {
   Write-Host ("PAR {0} viewport sem curvas: otimizado {1:N2} ms; legado {2:N2} ms" -f
                $round,$a.paint_without_native_items_p95_ms,$b.paint_without_native_items_p95_ms)
  }
 }
 $result = [pscustomobject]@{
  status = 'PASS'
  experiment = 'paired-native-bezier-19.9'
  source_sha256 = $initialHash
  matching_source = $true
  rounds = $Repeticoes
  pairs = @($allPairs.ToArray())
  machine_commands_sent = $false
  geometry_unchanged = $true
 }
 $json = $result | ConvertTo-Json -Depth 10
 [IO.File]::WriteAllText($summaryPath,$json,[Text.UTF8Encoding]::new($false))
 Write-Host "COMPARACAO A/B CONCLUIDA: $summaryPath" -ForegroundColor Green
}
finally {
 if ($original -and $initialHash) {
  $endHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
  if ($endHash -ne $initialHash) {
   throw 'ALERTA: SHA256 do arquivo original mudou. Nao prosseguir.'
  }
  Write-Host 'SHA256 do arquivo original preservado.' -ForegroundColor Green
 }
}
