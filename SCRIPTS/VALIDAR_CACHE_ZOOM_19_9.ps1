#requires -Version 5.1
<#
ALENCAR LASER 19.9 - read-only audit of the existing Qt cache A/B JSON.
Does not build, start Qt, operate hardware or modify the source project.
A PASS below is automatic evidence only, never production homologation.
#>
[CmdletBinding()]
param([string]$Relatorio = '')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$folder = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9'
if ([string]::IsNullOrWhiteSpace($Relatorio)) {
 $file = Get-ChildItem -LiteralPath $folder -Filter 'COMPARACAO_CACHE_19_9_*.json' -File |
   Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if ($null -eq $file) { throw 'Nenhum relatorio A/B foi encontrado.' }
 $Relatorio = $file.FullName
}
$path = (Resolve-Path -LiteralPath $Relatorio).Path
$r = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
$issues = New-Object 'System.Collections.Generic.List[string]'
function Assert-Check([bool]$good, [string]$reason) {
 if (-not $good) { $script:issues.Add($reason) }
}
Assert-Check ($r.status -eq 'PASS') 'Par A/B nao passou.'
Assert-Check ([bool]$r.matching_source -and [bool]$r.source_intact) 'Origem sem prova de integridade.'
Assert-Check (-not [bool]$r.machine_commands_sent) 'Relatorio indica comandos para maquina.'
$a = $r.no_cache
$b = $r.device_cache
Assert-Check ($null -ne $a -and $null -ne $b) 'Modos A/B ausentes.'
if ($null -ne $a -and $null -ne $b) {
 Assert-Check ($a.source_sha256 -eq $b.source_sha256 -and $a.source_sha256 -eq $r.source_sha256) 'SHA de origem divergente.'
 Assert-Check ([int]$a.contours -gt 0 -and [int]$a.contours -eq [int]$b.contours) 'Contornos divergentes.'
 Assert-Check ($a.qt -eq $b.qt -and [double]$a.dpr -eq [double]$b.dpr) 'Qt/DPR divergentes.'
 Assert-Check ([int]$a.viewport_width_px -eq [int]$b.viewport_width_px -and [int]$a.viewport_height_px -eq [int]$b.viewport_height_px) 'Viewport divergente.'
 Assert-Check ([double]$a.paint_p95_ms -gt 0 -and [double]$b.paint_p95_ms -gt 0 -and [double]$b.paint_p95_ms -lt [double]$a.paint_p95_ms) 'Cache sem ganho P95 no enquadramento.'
 Assert-Check ([double]$b.cache_visual_pixels_over8_percent -eq 0) 'Divergencia visual de pixels acima de 8.'
 Assert-Check ([int]$b.cache_visual_max_channel_difference -le 8) 'Diferenca maxima de canal superior a 8.'
 Assert-Check ($r.zoom_audit_status -eq 'MEASURED_VISUAL_REVIEW_PENDING') 'Zoom nao foi medido completamente.'
 $zooms = @($b.zoom_audit)
 Assert-Check ($zooms.Count -eq 3) 'Esperadas exatamente tres escalas de zoom.'
 if ($zooms.Count -eq 3) {
  $expected = @(1,2,4)
  for ($i=0; $i -lt 3; $i++) {
   $z = $zooms[$i]
   $scale = [double]$z.relative_zoom_factor
   Assert-Check ($scale -eq $expected[$i]) ("Escala inesperada: {0}" -f $scale)
   Assert-Check ([int]$z.visible_native_items -ge [math]::Ceiling([int]$a.contours * 0.90)) ("Zoom {0}: menos de 90% dos contornos visiveis" -f $scale)
   Assert-Check ([int]$z.cache_items -ge [math]::Ceiling([int]$z.visible_native_items * 0.90)) ("Zoom {0}: cobertura de cache abaixo de 90%" -f $scale)
   Assert-Check ([int]$z.native_ink_pixels -ge 32) ("Zoom {0}: arte nao visivel" -f $scale)
   Assert-Check ([double]$z.visual_pixels_over8_percent -eq 0) ("Zoom {0}: divergencia visual acima de 8" -f $scale)
   Assert-Check ([double]$z.no_cache_paint_p95_ms -gt 0 -and [double]$z.cached_paint_p95_ms -gt 0 -and [double]$z.cached_paint_p95_ms -lt [double]$z.no_cache_paint_p95_ms) ("Zoom {0}: cache nao acelerou P95" -f $scale)
  }
 }
}
$status = if ($issues.Count -eq 0) { 'PASS_AUTOMATED_MANUAL_PENDING' } else { 'FAIL_AUTOMATED' }
$result = [ordered]@{
 status = $status
 audit = 'alencar-laser-cache-zoom-review-19.9'
 input_json = $path
 source_sha256 = [string]$r.source_sha256
 checks_failed = @($issues.ToArray())
 manual_gates_pending = @(
   'Real mouse: repeated select/move and deselect with handles',
   'Continuous zoom and pan with no ghosting, blur or missing strokes',
   'Scissors: preview, trim, undo and redo over curved/overlapping paths',
   'At least three independent A/B pairs and RSS trend',
   'Second real file and full application regression'
 )
 hardware_permitted = $false
 production_homologated = $false
}
$out = Join-Path $folder ('AUDITORIA_CACHE_ZOOM_19_9_' + (Get-Date -Format 'yyyyMMdd_HHmmss') + '.json')
[IO.File]::WriteAllText($out,($result | ConvertTo-Json -Depth 6),[Text.UTF8Encoding]::new($false))
Write-Host ("Auditoria: {0}" -f $status) -ForegroundColor $(if ($issues.Count -eq 0) {'Green'} else {'Red'})
foreach ($issue in $issues) { Write-Warning $issue }
Write-Host 'Homologacao manual permanece pendente; comandos de laser continuam bloqueados.'
Write-Host "Relatorio: $out"
if ($issues.Count -gt 0) { exit 1 }
