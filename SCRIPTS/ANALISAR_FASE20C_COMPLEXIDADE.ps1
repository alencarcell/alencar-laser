#requires -Version 5.1
<#
Fase 20c: leitura estatistica de relatorio local JA coletado.
Sem Qt, sem recompilacao, sem comandos laser, sem alteracao de projetos.
Nao interpretar variacao OFF/ON3 como ganho: repaint equivalente no fixture denso.
#>
[CmdletBinding()]
param([string]$ResumoJson)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$base = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
function Fail([string]$message) { throw "FASE20C: $message" }
function Number($obj,[string]$key) {
  $property = $obj.PSObject.Properties[$key]
  if ($null -eq $property -or $null -eq $property.Value) { Fail "Campo ausente: $key" }
  $n = [double]$property.Value
  if ([double]::IsNaN($n) -or [double]::IsInfinity($n) -or $n -lt 0) { Fail "Metrica invalida: $key" }
  return $n
}
if ([string]::IsNullOrWhiteSpace($ResumoJson)) {
  if (-not (Test-Path -LiteralPath $base -PathType Container)) { Fail 'Sem diretorio de coleta local.' }
  $found = @(Get-ChildItem -LiteralPath $base -Directory -Filter 'CONTROLADO_575_FASE20_*' |
    Sort-Object LastWriteTimeUtc -Descending |
    ForEach-Object { Join-Path $_.FullName 'RESUMO_AB_CONTROLADO_575_19_9.json' } |
    Where-Object { Test-Path -LiteralPath $_ -PathType Leaf })
  if ($found.Count -eq 0) { Fail 'Nenhum resumo Fase20 encontrado. Execute a coleta existente primeiro.' }
  $ResumoJson = $found[0]
}
if (-not (Test-Path -LiteralPath $ResumoJson -PathType Leaf)) { Fail "Arquivo ausente: $ResumoJson" }
$report = Get-Content -LiteralPath $ResumoJson -Raw -Encoding UTF8 | ConvertFrom-Json
if ($report.schema -ne 'alencar-laser-controlled-drag-575-ab-19.9' -or
    $report.status -ne 'CONTROLLED_SYNTHETIC_MEASURED_MANUAL_PENDING' -or
    $report.vector_paint_probe_enabled -ne $true -or
    $report.performance_gate -ne 'EQUIVALENT_REPAINT_POLICY_NO_SPEEDUP_CLAIM' -or
    $report.speedup_claim_allowed -ne $false -or
    $report.automatic_promotion_allowed -ne $false -or
    $report.production_homologated -ne $false -or
    $report.hardware_commands_sent -ne $false) {
  Fail 'Gate de seguranca do relatorio nao atendido.'
}
$rounds = @($report.rounds)
if ($rounds.Count -ne 6) { Fail 'Esperadas seis rodadas.' }
$expected = @('OFF','ON','ON','OFF','OFF','ON')
$rows = @()
for ($i=0; $i -lt 6; $i++) {
  $round = $rounds[$i]
  if ($round.mode -ne $expected[$i] -or [int]$round.round -ne ($i+1) -or
      [int]$round.paint_samples -ne 144) { Fail "Sequencia/amostragem inesperada na rodada $($i+1)." }
  $bins = @($round.vector_complexity_buckets)
  if ($bins.Count -ne 4) { Fail "Quatro faixas esperadas na rodada $($i+1)." }
  $contourSum = 0
  $callSum = 0.0
  $timeSum = 0.0
  foreach ($bin in $bins) {
    $count = Number $bin 'contours'
    $calls = Number $bin 'paint_calls'
    $ms = Number $bin 'total_ms'
    $contourSum += [int]$count
    $callSum += $calls
    $timeSum += $ms
    $rows += [pscustomobject]@{
      Rodada = $i + 1
      Modo = $round.mode
      Faixa = [string]$bin.node_band
      Contornos = [int]$count
      Chamadas = [int]$calls
      TotalMs = [math]::Round($ms,3)
      MediaPorChamadaMs = $(if ($calls -gt 0) { [math]::Round($ms/$calls,6) } else { $null })
      ParticipacaoCustoPct = 0
    }
  }
  if ($contourSum -ne 575 -or $callSum -lt 575 -or $timeSum -le 0) {
    Fail "Integridade de contornos/tempos invalida na rodada $($i+1)."
  }
  foreach ($row in @($rows | Where-Object { $_.Rodada -eq ($i+1) })) {
    $row.ParticipacaoCustoPct = [math]::Round(100*$row.TotalMs/$timeSum,2)
  }
}
Write-Host 'ALENCAR LASER - FASE 20c / PERFIL POR COMPLEXIDADE' -ForegroundColor Cyan
Write-Host "Fonte local (somente leitura): $ResumoJson"
$rows | Format-Table Rodada,Modo,Faixa,Contornos,Chamadas,TotalMs,MediaPorChamadaMs,ParticipacaoCustoPct -AutoSize
Write-Host 'GATE: PERFIL_VALIDADO_APENAS_SINTETICO' -ForegroundColor Yellow
Write-Host 'Atenção: medias agregadas nao medem cache nem ganho causal. ON3 e OFF repintam viewport inteira para 575 contornos.'
Write-Host 'Proxima decisao: avaliar se as faixas complexas concentram custo consistente; somente depois desenhar experimento de cache opt-in.'
Write-Host 'Nao altera codigo Qt, vetores, arquivos .alaser, main, instalador ou controle da maquina.'
