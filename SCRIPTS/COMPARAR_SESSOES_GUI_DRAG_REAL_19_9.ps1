#requires -Version 5.1
<#
ALENCAR LASER - comparador offline de sessoes reais Qt 19.9.
Le SOMENTE JSONs de telemetria do launcher A/B; nao abre o editor,
nao acessa laser e nao publica no GitHub.
ATENCAO: tempos de sessao, por si, nao medem ganho de desempenho.
#>
[CmdletBinding()]
param(
 [string]$OffJson = '',
 [string]$OnJson = ''
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$sessionRoot = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\sessao'
$outRoot = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
function Campo($obj,[string]$key) {
 if ($null -eq $obj) { return $null }
 $p = $obj.PSObject.Properties[$key]
 if ($null -eq $p) { return $null }
 return $p.Value
}
function Numero($obj,[string]$key) {
 $value = Campo $obj $key
 if ($null -eq $value -or $value -is [bool] -or $value -is [string]) { return $null }
 try {
  $n = [double]$value
  if ([double]::IsNaN($n) -or [double]::IsInfinity($n) -or $n -lt 0) { return $null }
  return [math]::Round($n,4)
 } catch { return $null }
}
function Escolher([string]$mode,[string]$requested) {
 if (-not [string]::IsNullOrWhiteSpace($requested)) {
  return (Resolve-Path -LiteralPath $requested -ErrorAction Stop).Path
 }
 if (-not (Test-Path -LiteralPath $sessionRoot -PathType Container)) {
  throw "Pasta de sessoes ausente: $sessionRoot"
 }
 $pastas = @(Get-ChildItem -LiteralPath $sessionRoot -Directory -Filter ("DRAG_AB_19_9_" + $mode + "_*") |
     Sort-Object LastWriteTimeUtc -Descending)
 foreach ($p in $pastas) {
  $f = Join-Path $p.FullName 'SESSAO_INTERATIVA_TESTE_6.json'
  if (Test-Path -LiteralPath $f -PathType Leaf) { return $f }
 }
 throw "Nenhuma sessao completa encontrada no modo $mode"
}
function Ler([string]$path,[string]$mode) {
 $file = Get-Item -LiteralPath $path -ErrorAction Stop
 if ($file.Extension -ine '.json' -or $file.Length -le 0 -or $file.Length -gt 2MB) {
  throw "JSON de sessao invalido para o modo $mode"
 }
 $j = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
 if ($null -eq (Campo $j 'paint_count') -or
     $null -eq (Campo $j 'memory') -or
     $null -eq (Campo $j 'paint_p95_ms')) {
  throw "Telemetria incompleta no modo $mode"
 }
 if ((Campo $j 'hardware_connected') -ne $false) {
  throw "Hardware nao esta confirmado como desconectado no modo $mode"
 }
 return $j
}
function Delta([object]$a,[object]$b) {
 if ($null -eq $a -or $null -eq $b -or [double]$a -le 0) { return $null }
 return [math]::Round((([double]$a-[double]$b)/[double]$a)*100,2)
}
$offPath = Escolher 'OFF' $OffJson
$onPath = Escolher 'ON' $OnJson
if ($offPath -eq $onPath) { throw 'OFF e ON apontam para o mesmo arquivo.' }
$off = Ler $offPath 'OFF'
$on  = Ler $onPath 'ON'
$issues = New-Object 'System.Collections.Generic.List[string]'
foreach ($key in @('qt','platform','os','dpr')) {
 $o = Campo $off $key
 $n = Campo $on $key
 if ($null -eq $o -or $null -eq $n -or "$o" -ne "$n") {
  $issues.Add("Ambiente diferente ou ausente: $key")
 }
}
$pOff = Numero $off 'paint_p95_ms'
$pOn = Numero $on 'paint_p95_ms'
$inputOff = Numero $off 'input_to_paint_p95_ms'
$inputOn = Numero $on 'input_to_paint_p95_ms'
$paintSamplesOff = Numero $off 'paint_count'
$paintSamplesOn = Numero $on 'paint_count'
$inputSamplesOff = Numero $off 'input_to_paint_samples'
$inputSamplesOn = Numero $on 'input_to_paint_samples'
if ($null -eq $pOff -or $null -eq $pOn -or $pOff -eq 0 -or
    $null -eq $paintSamplesOff -or $null -eq $paintSamplesOn) {
 throw 'Metrica de pintura incompleta: nao e possivel comparar as sessoes.'
}
if ($paintSamplesOff -lt 30 -or $paintSamplesOn -lt 30) {
 $issues.Add('Poucas amostras de pintura; p95 potencialmente instavel.')
}
if ($null -eq $inputOff -or $null -eq $inputOn -or
    $null -eq $inputSamplesOff -or $null -eq $inputSamplesOn -or
    $inputSamplesOff -lt 30 -or $inputSamplesOn -lt 30) {
 $issues.Add('Amostras de entrada ate pintura insuficientes para conclusao.')
}
$ratio = [math]::Min($paintSamplesOff,$paintSamplesOn)/
         [math]::Max($paintSamplesOff,$paintSamplesOn)
if ($ratio -lt 0.7) {
 $issues.Add('Quantidade de amostras de pintura difere mais de 30% entre OFF e ON.')
}
$memOff = Campo $off 'memory'
$memOn  = Campo $on 'memory'
$result = [ordered]@{
 schema = 'alencar-laser-manual-drag-ab-summary-19.9'
 status = 'OBSERVATIONAL_NOT_HOMOLOGATED'
 source = 'two-local-interactive-qt-sessions'
 baseline_off = [ordered]@{
  paint_samples = $paintSamplesOff
  paint_p95_ms = $pOff
  input_to_paint_samples = $inputSamplesOff
  input_to_paint_p95_ms = $inputOff
  rss_bytes_exit = NumERO $memOff 'rss_bytes'
  peak_rss_bytes = Numero $memOff 'peak_rss_bytes'
  localized_node_updates = Numero $off 'localized_node_updates'
 }
 candidate_on = [ordered]@{
  paint_samples = $paintSamplesOn
  paint_p95_ms = $pOn
  input_to_paint_samples = $inputSamplesOn
  input_to_paint_p95_ms = $inputOn
  rss_bytes_exit = Numero $memOn 'rss_bytes'
  peak_rss_bytes = Numero $memOn 'peak_rss_bytes'
  localized_node_updates = Numero $on 'localized_node_updates'
 }
 paint_p95_reduction_percent = Delta $pOff $pOn
 input_to_paint_p95_reduction_percent = Delta $inputOff $inputOn
 environment_matching = (@($issues | Where-Object { $_ -like 'Ambiente*' }).Count -eq 0)
 validation_warnings = @($issues.ToArray())
 limitations = @(
  'The launcher JSON does not contain drag overlay flag/counters, paint P99, or viewport paint bbox percentiles.',
  'OFF/ON label is inferred from the launcher folder name, not independently asserted by the JSON.',
  'Unequal user gestures, sample counts, zoom, and document state can confound this comparison.',
  'Session elapsed durations are deliberately excluded from the speed calculation.'
 )
 hardware_connected = $false
 production_homologated = $false
}
[void](New-Item -ItemType Directory -Path $outRoot -Force)
$out = Join-Path $outRoot ('COMPARACAO_GUI_REAL_DRAG_19_9_' +
     (Get-Date -Format 'yyyyMMdd_HHmmss') + '.json')
[IO.File]::WriteAllText($out,(($result | ConvertTo-Json -Depth 8)+[Environment]::NewLine),
 [Text.UTF8Encoding]::new($false))
Write-Host 'ALENCAR LASER - comparacao observacional OFF/ON' -ForegroundColor Cyan
Write-Host ('OFF pintura P95: {0} ms ({1} amostras)' -f $pOff,$paintSamplesOff)
Write-Host ('ON  pintura P95: {0} ms ({1} amostras)' -f $pOn,$paintSamplesOn)
Write-Host ('Delta de pintura P95 (positivo favorece ON): {0}%' -f (Delta $pOff $pOn))
Write-Host ('OFF entrada->pintura P95: {0} ms / ON: {1} ms' -f $inputOff,$inputOn)
Write-Host ('Delta entrada->pintura P95 (positivo favorece ON): {0}%' -f (Delta $inputOff $inputOn))
Write-Host ('OFF RSS ao sair: {0} bytes / ON: {1} bytes' -f (Numero $memOff 'rss_bytes'),(Numero $memOn 'rss_bytes'))
foreach ($warning in $issues) { Write-Warning $warning }
Write-Warning 'Comparacao observacional: sem garantia de gestos/zoom iguais, sem P99 e sem homologacao.'
Write-Host "JSON local: $out"
Write-Host 'SEM ENVIO. Nenhuma maquina conectada ou acionada.' -ForegroundColor Green
