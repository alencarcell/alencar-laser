#requires -Version 5.1
<#
FASE 20 - Analisa offline os seis relatorios da coleta vetorial ja existente.
Nao abre Qt, nao recompila, nao altera arquivos, nao usa GitHub, nao envia
comandos ao laser. Sem -Resumo: usa a ultima pasta CONTROLADO_575_FASE20_*.
Uma media dos P95 (ou P95/P95) NAO mede parcela real por frame.
#>
[CmdletBinding()]
param([string]$Resumo='')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$base=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
if ([string]::IsNullOrWhiteSpace($Resumo)) {
 if (-not (Test-Path -LiteralPath $base -PathType Container)) {
  throw 'Pasta local de auditoria nao encontrada.'
 }
 $groups=@(Get-ChildItem -LiteralPath $base -Directory -Filter 'CONTROLADO_575_FASE20_*' |
   Sort-Object LastWriteTimeUtc -Descending)
 $file=$null
 foreach ($g in $groups) {
  $candidate=Join-Path $g.FullName 'RESUMO_AB_CONTROLADO_575_19_9.json'
  if (Test-Path -LiteralPath $candidate -PathType Leaf) {
   $file=$candidate;break
  }
 }
 if (-not $file) {throw 'Nenhum resumo da Fase 20 encontrado.'}
} else {
 $file=(Resolve-Path -LiteralPath $Resumo -ErrorAction Stop).Path
}
$info=Get-Item -LiteralPath $file -ErrorAction Stop
if ($info.Extension -ine '.json' -or $info.Length -le 0 -or $info.Length -gt 8MB) {
 throw 'Resumo invalido ou excessivamente grande.'
}
$r=Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
if ($r.schema -ne 'alencar-laser-controlled-drag-575-ab-19.9' -or
 $r.vector_paint_probe_enabled -ne $true -or
 [string]$r.optin_mode_code -ne '3' -or
 $r.status -ne 'CONTROLLED_SYNTHETIC_MEASURED_MANUAL_PENDING' -or
 $r.performance_gate -ne 'EQUIVALENT_REPAINT_POLICY_NO_SPEEDUP_CLAIM' -or
 $r.production_homologated -ne $false -or
 $r.automatic_promotion_allowed -ne $false -or
 $r.hardware_commands_sent -ne $false) {
 throw 'Relatorio nao corresponde a Fase 20 validada.'
}
$rows=@($r.rounds)
$off=@($rows | Where-Object { $_.mode -eq 'OFF' })
$on=@($rows | Where-Object { $_.mode -eq 'ON' })
if ($rows.Count -ne 6 -or $off.Count -ne 3 -or $on.Count -ne 3) {
 throw 'A coleta deve conter tres rodadas de cada modo.'
}
function Num([object]$row,[string]$key) {
 $prop=$row.PSObject.Properties[$key]
 if ($null -eq $prop -or $null -eq $prop.Value) {
  throw "Metrica $key ausente ou nula."
 }
 $n=[double]$prop.Value
 if ([double]::IsNaN($n) -or [double]::IsInfinity($n) -or $n -lt 0) {
  throw "Metrica $key fora do intervalo."
 }
 return $n
}
function Med([object[]]$items,[string]$field) {
 $ordered=@($items | ForEach-Object { Num $_ $field } | Sort-Object)
 if ($ordered.Count -ne 3) { throw "Mediana exige 3 itens para $field." }
 return [math]::Round($ordered[1],4)
}
$required=@('paint_samples','paint_p95_ms','paint_p99_ms','vector_paint_p95_ms',
 'vector_paint_p99_ms','vector_paint_item_calls_p95',
 'noncontour_inclusive_p95_ms','paired_other_inclusive_p95_ms',
 'input_to_paint_p95_ms','viewport_bbox_p95_fraction','local_requests')
foreach($row in $rows) {
 foreach($field in $required) { [void](Num $row $field) }
 if ((Num $row 'paint_samples') -lt 30 -or
     (Num $row 'vector_paint_item_calls_p95') -lt 1) {
  throw "Amostras insuficientes na rodada $($row.round)."
 }
 if ((Num $row 'viewport_bbox_p95_fraction') -lt 0.99 -or
     (Num $row 'local_requests') -ne 0) {
  throw 'Politica de repaint nao foi equivalente ao OFF.'
 }
}
foreach($row in $on) {
 if ((Num $row 'dense_legacy_updates') -lt 144) {
  throw 'A guarda densa ON3 nao foi confirmada.'
 }
}
$minS=($rows | Measure-Object -Property paint_samples -Minimum).Minimum
$maxS=($rows | Measure-Object -Property paint_samples -Maximum).Maximum
if ([double]$maxS -le 0 -or ([double]$minS/[double]$maxS) -lt .70) {
 throw 'Amostragem desequilibrada.'
}
Write-Host 'ALENCAR LASER / FASE 20: leitura do ultimo teste vetorial (SEM nova coleta)' -ForegroundColor Cyan
Write-Host "Fonte: $file"
$rows | Select-Object round,mode,paint_samples,paint_p95_ms,
 vector_paint_p95_ms,vector_paint_p99_ms,vector_paint_item_calls_p95,
 noncontour_inclusive_p95_ms | Format-Table -AutoSize
Write-Host 'MEDIANAS P95 em milissegundos (nao aditivas):' -ForegroundColor Cyan
[pscustomobject]@{
 Indicador='Pintura vetorial (subclasse instrumentada)'
 OFF=(Med $off 'vector_paint_p95_ms')
 ON3=(Med $on 'vector_paint_p95_ms')
} | Format-Table -AutoSize
[pscustomobject]@{
 Indicador='Chamadas de paint() dos contornos por frame, P95'
 OFF=(Med $off 'vector_paint_item_calls_p95')
 ON3=(Med $on 'vector_paint_item_calls_p95')
} | Format-Table -AutoSize
[pscustomobject]@{
 Indicador='Restante inclusivo nao-contorno'
 OFF=(Med $off 'noncontour_inclusive_p95_ms')
 ON3=(Med $on 'noncontour_inclusive_p95_ms')
} | Format-Table -AutoSize
Write-Warning 'ON3 e OFF tem a mesma repintura para 575 contornos; diferencias entre rodadas nao demonstram aceleracao.'
Write-Warning 'A subclasse de diagnostico e os cronometros perturbam o tempo. Quantis P95 de etapas NAO se somam ou representam porcentagens.'
Write-Host 'Hipotese seguinte: separar numero de chamadas de paint de custo de cada chamada, sem modificar a geometria.' -ForegroundColor Yellow
Write-Host 'Somente leitura: sem Qt, sem laser, sem alteracoes de arquivo ou Git push.' -ForegroundColor Green
