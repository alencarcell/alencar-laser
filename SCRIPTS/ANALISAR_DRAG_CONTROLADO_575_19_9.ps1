#requires -Version 5.1
<#
ALENCAR LASER - analise somente-leitura de um A/B controlado ja executado.
Nao recompila Qt, nao abre editor, nao acessa laser, nao publica GitHub.
Aceita JSON RESUMO_AB_CONTROLADO_575_19_9 gerado pela versao anterior
ou atual do comparador; recalcula gate e detalha bounds/contadores.
#>
[CmdletBinding()]
param([string]$Resumo = '')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$base = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
if ([string]::IsNullOrWhiteSpace($Resumo)) {
 if (-not (Test-Path -LiteralPath $base -PathType Container)) {
  throw 'Pasta de auditorias locais nao encontrada.'
 }
 $list=@(Get-ChildItem -LiteralPath $base -Directory -Filter 'CONTROLADO_575_*' |
   Sort-Object LastWriteTimeUtc -Descending)
 $file=$null
 foreach ($dir in $list) {
  $candidate=Join-Path $dir.FullName 'RESUMO_AB_CONTROLADO_575_19_9.json'
  if (Test-Path -LiteralPath $candidate -PathType Leaf) {
   $file=$candidate;break
  }
 }
 if (-not $file) { throw 'Resumo controlado de 575 contornos nao encontrado.' }
} else {
 $file=(Resolve-Path -LiteralPath $Resumo -ErrorAction Stop).Path
}
$fi=Get-Item -LiteralPath $file -ErrorAction Stop
if ($fi.Length -le 0 -or $fi.Length -gt 8MB -or $fi.Extension -ine '.json') {
 throw 'Resumo invalido ou grande demais.'
}
$r=Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
if ($r.schema -ne 'alencar-laser-controlled-drag-575-ab-19.9' -or
    $r.hardware_commands_sent -ne $false -or
    $r.production_homologated -ne $false) {
 throw 'Schema ou estados de seguranca invalidos.'
}
$rows=@($r.rounds)
$a=@($rows|Where-Object { $_.mode -eq 'OFF' })
$b=@($rows|Where-Object { $_.mode -eq 'ON' })
if ($rows.Count -ne 6 -or $a.Count -ne 3 -or $b.Count -ne 3) {
 throw 'Esperadas 3 rodadas OFF e 3 ON.'
}
foreach ($row in $rows) {
 foreach ($key in @('paint_samples','paint_p95_ms','paint_p99_ms',
      'input_to_paint_p95_ms','viewport_bbox_p95_fraction',
      'local_requests','full_fallbacks')) {
  $v=$row.PSObject.Properties[$key]
  if ($null -eq $v -or $null -eq $v.Value) { throw "Campo ausente: $key" }
  $number=[double]$v.Value
  if ([double]::IsNaN($number) -or [double]::IsInfinity($number) -or $number -lt 0) {
   throw "Medida invalida: $key"
  }
 }
}
function Mediana([object[]]$values) {
 $sorted=@($values | ForEach-Object { [double]$_ } | Sort-Object)
 return $sorted[1]
}
function Med([object[]]$group,[string]$key) {
 return Mediana @($group | ForEach-Object { $_.$key })
}
$off95=Med $a 'paint_p95_ms'
$on95=Med $b 'paint_p95_ms'
$off99=Med $a 'paint_p99_ms'
$on99=Med $b 'paint_p99_ms'
$bboxOff=Med $a 'viewport_bbox_p95_fraction'
$bboxOn=Med $b 'viewport_bbox_p95_fraction'
$reg95=100*($on95-$off95)/$off95
$reg99=100*($on99-$off99)/$off99
$paintCount=@($rows | ForEach-Object { [double]$_.paint_samples })
$minSamples=[double](($paintCount | Measure-Object -Minimum).Minimum)
$maxSamples=[double](($paintCount | Measure-Object -Maximum).Maximum)
if ($maxSamples -le 0 -or $off95 -le 0 -or $off99 -le 0) { throw 'Pintura/amostras invalidas no resumo.' }
$balance=$minSamples/$maxSamples
$gate=if ($balance -lt 0.70) { 'INCONCLUSIVE_SAMPLE_IMBALANCE' }
      elseif ($reg95 -ge 10 -and $reg99 -ge 10) { 'REGRESSION_OBSERVED_DO_NOT_PROMOTE' }
      else { 'SYNTHETIC_RESULTS_MANUAL_REVIEW_REQUIRED' }
Write-Host 'ALENCAR LASER — revisao de seis rodadas ja existentes' -ForegroundColor Cyan
$rows | Select-Object round,mode,paint_samples,paint_p95_ms,paint_p99_ms,
 input_to_paint_p95_ms,viewport_bbox_p95_fraction,
 local_requests,full_fallbacks | Format-Table -AutoSize
Write-Host ('P95 pintura mediana OFF/ON: {0:N4} / {1:N4} ms ({2:+0.00;-0.00;0.00}% ON vs OFF)' -f $off95,$on95,$reg95)
Write-Host ('P99 pintura mediana OFF/ON: {0:N4} / {1:N4} ms ({2:+0.00;-0.00;0.00}% ON vs OFF)' -f $off99,$on99,$reg99)
Write-Host ('Bbox P95 (proporcao viewport) OFF/ON: {0:N3} / {1:N3}' -f $bboxOff,$bboxOn)
Write-Host ('Requisicoes local ON: {0}; fallbacks completos ON: {1}' -f
 (($b | Measure-Object -Property local_requests -Sum).Sum),
 (($b | Measure-Object -Property full_fallbacks -Sum).Sum))
Write-Host ('Balanceamento de amostras: {0:N4}' -f $balance)
Write-Host ('Gate de desempenho: {0}' -f $gate) -ForegroundColor Yellow
Write-Warning 'O gate e uma regra heuristica de seguranca, nao um teste de significancia estatistica nem homologacao visual.'
Write-Host 'SOMENTE LEITURA. Nenhum comando de maquina, arquivo alterado ou Git push.' -ForegroundColor Green
