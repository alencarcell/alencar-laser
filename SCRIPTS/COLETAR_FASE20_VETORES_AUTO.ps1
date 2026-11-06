#requires -Version 5.1
<#
ALENCAR LASER — FASE 20 / Coleta automatica do paint de contornos.
- Recompila apenas o binario Qt experimental, roda seis casos sinteticos.
- Nao envia comandos de maquina e nao publica arquivos no GitHub.
- Usa fixture publica de 575 contornos e valida geometria/Undo/Redo.
- Qualquer FAIL interrompe o gate; nenhum resultado e homologado.
#>
[CmdletBinding()]
param([switch]$SemCompilar)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$collect=Join-Path $PSScriptRoot 'COMPARAR_DRAG_CONTROLADO_575_19_9.ps1'
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
if (-not (Test-Path -LiteralPath $collect -PathType Leaf)) {
 throw 'Script de coleta controlada ausente no projeto.'
}
Write-Host 'FASE 20 — coleta offline de contornos vetoriais (Qt experimental).' -ForegroundColor Cyan
Write-Warning 'Mantenha a maquina laser desconectada. Feche o Alencar Laser antes de executar.'
$start=(Get-Date).ToUniversalTime()
$argsCollector=@('-NoProfile','-ExecutionPolicy','Bypass','-File',$collect,
                 '-ModoOtimizado','3','-ColetarVetores')
if ($SemCompilar) { $argsCollector+='-SemCompilar' }
& powershell.exe @argsCollector
$exit=$LASTEXITCODE
if ($exit -ne 0) { throw "Coleta Qt falhou, codigo $exit. Nenhum PASS registrado." }
if (-not (Test-Path -LiteralPath $reports -PathType Container)) {
 throw 'Coleta saiu sem diretorio de diagnosticos.'
}
$folder=@(Get-ChildItem -LiteralPath $reports -Directory -Filter 'CONTROLADO_575_FASE20_*' |
          Where-Object { $_.CreationTimeUtc -ge $start.AddSeconds(-3) } |
          Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1)
if ($folder.Count -ne 1) { throw 'Nao foi possivel identificar a coleta atual.' }
$file=Join-Path $folder[0].FullName 'RESUMO_AB_CONTROLADO_575_19_9.json'
if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
 throw 'Resumo final nao foi produzido; nao homologar.'
}
$r=Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
if ($r.vector_paint_probe_enabled -ne $true -or
    $r.optin_mode_code -ne '3' -or
    $r.status -ne 'CONTROLLED_SYNTHETIC_MEASURED_MANUAL_PENDING' -or
    $r.performance_gate -ne 'EQUIVALENT_REPAINT_POLICY_NO_SPEEDUP_CLAIM' -or
    $r.automatic_promotion_allowed -ne $false -or
    $r.speedup_claim_allowed -ne $false -or
    $r.production_homologated -ne $false -or
    $r.hardware_commands_sent -ne $false -or
    @($r.rounds).Count -ne 6) {
 throw 'Uma verificacao de seguranca ou integridade do resumo falhou.'
}
foreach($round in @($r.rounds)){
 if ($null -eq $round.vector_paint_p95_ms -or
     $null -eq $round.vector_paint_p99_ms -or
     $null -eq $round.noncontour_inclusive_p95_ms -or
     $null -eq $round.vector_paint_item_calls_p95 -or
     [double]$round.vector_paint_item_calls_p95 -lt 1) {
  throw "Metricas de pintura vetorial incompletas na rodada $($round.round)."
 }
}
# Resume os hotspots da propria coleta, sem novos testes e sem arquivo extra.
$analyzer=Join-Path $PSScriptRoot 'ANALISAR_FASE20_VETORES_AUTO.ps1'
if (-not (Test-Path -LiteralPath $analyzer -PathType Leaf)) {
 throw 'Analisador Fase20b ausente: resumo tecnico nao verificado.'
}
Write-Host 'FASE 20b: analisando automaticamente grupos de nos e dez maiores custos...' -ForegroundColor Cyan
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $analyzer -Resumo $file
if ($LASTEXITCODE -ne 0) {
 throw 'Analisador dos hotspots falhou; revisar relatorio antes de registrar conclusoes.'
}
Write-Host 'PASS TECNICO: 6 rodadas, 575 contornos, diagnosticos por frame.' -ForegroundColor Green
Write-Host 'A coleta e instrumentada e NAO comprova ganhos de desempenho na GUI comum.' -ForegroundColor Yellow
Write-Host "Resumo: $file" -ForegroundColor Cyan
Write-Host 'Sem Git push, sem hardware e sem homologacao de producao.' -ForegroundColor Green
