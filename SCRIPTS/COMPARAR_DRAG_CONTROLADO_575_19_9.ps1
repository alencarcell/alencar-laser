#requires -Version 5.1
<#
ALENCAR LASER 19.9 - A/B controlado com fixture REAL de 575 contornos.
Seis processos Qt separados na ordem OFF,ON,ON,OFF,OFF,ON.
Cada um repete 144 eventos Qt de mouse, zoom 1.2, mesma viewport.
Sem comandos de laser, sem projeto de cliente, sem git push.
PASS do teste sintetico nao e homologacao visual ou de producao.
#>
[CmdletBinding()]
param([switch]$SemCompilar)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$src = Join-Path $root 'DESENVOLVIMENTO\FONTE'
$fixture = Join-Path $src 'fixtures\web_nodes_normalized.csv'
$exe = Join-Path $src 'build_WINDOWS_DRAG_AB_19_9\alencar_test7_qt6.exe'
$resultsDir = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$groupDir = Join-Path $resultsDir ("CONTROLADO_575_" + $stamp + "_" + ([guid]::NewGuid().ToString('N').Substring(0,8)))
$oldPath = $env:PATH
$oldOverlay = [Environment]::GetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9','Process')
$oldGrouped = [Environment]::GetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG','Process')
function Mediana([object[]]$nums) {
 $sorted = @($nums | ForEach-Object { [double]$_ } | Sort-Object)
 if ($sorted.Count -ne 3) { throw 'Mediana exige tres rodadas por modo.' }
 return [math]::Round($sorted[1],4)
}
function Reducao([double]$baseline,[double]$candidate) {
 if ($baseline -le 0) { return $null }
 return [math]::Round(100*($baseline-$candidate)/$baseline,2)
}
function ValidateNumerical([object]$obj,[string]$key) {
 $p = $obj.PSObject.Properties[$key]
 if ($null -eq $p -or $null -eq $p.Value -or $p.Value -isnot [ValueType]) {
  throw "Metrica ausente: $key"
 }
 $n = [double]$p.Value
 if ([double]::IsNaN($n) -or [double]::IsInfinity($n) -or $n -lt 0) {
  throw "Metrica invalida: $key"
 }
 return $n
}
try {
 if (-not (Test-Path -LiteralPath $fixture -PathType Leaf)) {
  throw 'Fixture local normalizada ausente; nenhum projeto real sera usado como substituto.'
 }
 if (-not (Test-Path -LiteralPath 'C:\msys64\ucrt64\bin\libminizip-1.dll')) {
  throw 'Bibliotecas UCRT64 ausentes.'
 }
 if (-not $SemCompilar) {
  Write-Host 'Preflight: recompilando o binario Qt isolado e repetindo o teste de 40 objetos.' -ForegroundColor Cyan
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'COMPARAR_DRAG_OVERLAY_19_9.ps1')
  if ($LASTEXITCODE -ne 0) { throw 'Falha no gate Qt de 40 objetos ou compilacao. Teste 575 interrompido.' }
 }
 if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
  throw 'Executavel isolado ausente. Execute sem -SemCompilar.'
 }
 if ($SemCompilar -and
     (Get-Item -LiteralPath $exe).LastWriteTimeUtc -lt
     (Get-Item -LiteralPath (Join-Path $src 'qt\qt6_test6.cpp')).LastWriteTimeUtc) {
  throw 'Executavel mais antigo que a fonte Qt. Retire -SemCompilar.'
 }
 $run = @(Get-Process -Name 'alencar_test7_qt6' -ErrorAction SilentlyContinue |
     Where-Object { $_.Path -and
       [string]::Equals($_.Path,$exe,[StringComparison]::OrdinalIgnoreCase) })
 if ($run.Count -gt 0) { throw 'Feche a sessao Qt A/B em andamento antes de medir.' }
 $fixtureSha = (Get-FileHash -LiteralPath $fixture -Algorithm SHA256).Hash
 $binarySha  = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
 $env:PATH = 'C:\msys64\ucrt64\bin;' + $oldPath
 [Environment]::SetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG',$null,'Process')
 [void](New-Item -ItemType Directory -Path $groupDir -Force)
 $order = @('OFF','ON','ON','OFF','OFF','ON')
 $data = @()
 $reference = $null
 for ($i=0;$i -lt $order.Count;++$i) {
  $mode = $order[$i]
  $enabled = ($mode -eq 'ON')
  if ($enabled) { $env:ALENCAR_LOCAL_DRAG_OVERLAY_19_9='1' }
  else { [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$null,'Process') }
  $dest = Join-Path $groupDir ('RODADA_' + ('{0:D2}' -f ($i+1)) + '_' + $mode)
  [void](New-Item -ItemType Directory -Path $dest -Force)
  Write-Host ("Rodada {0}/6 - {1} (575 contornos, 144 movimentos)" -f ($i+1),$mode) -ForegroundColor Cyan
  & $exe --controlled-drag-575-19-9-test $fixture $dest
  $code = $LASTEXITCODE
  $file = Join-Path $dest 'RESULTADO_ARRASTE_CONTROLADO_575_19_9.json'
  if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
   throw "Qt nao gerou JSON na rodada $($i+1) - $mode"
  }
  $json = Get-Content -LiteralPath $file -Raw | ConvertFrom-Json
  if ($code -ne 0 -or $json.status -ne 'PASS_SYNTHETIC_MANUAL_PENDING') {
   $err = if ($null -ne $json.error) { $json.error } else { 'Falha nao especificada' }
   throw "Falha na rodada $($i+1) - $mode : $err"
  }
  if ($json.overlay_local_enabled -ne $enabled -or
      $json.selected_contours -ne 575 -or $json.fixture_nodes -ne 13415 -or
      $json.geometry_preserved -ne $true -or $json.undo_redo_preserved -ne $true -or
      $json.machine_commands_sent -ne $false -or
      $json.production_homologated -ne $false -or
      $json.measured.frames -ne 144 -or
      [double]$json.zoom_factor -ne 1.2) {
   throw "Gate de integridade divergente: rodada $($i+1)"
  }
  if ($enabled -and [double]$json.overlay_local_requests -le 0) {
   throw 'ON nao iniciou a repintura localizada.'
  }
  if (-not $enabled -and [double]$json.overlay_local_requests -ne 0) {
   throw 'OFF usou a politica ON inesperadamente.'
  }
  $environment = @($json.qt,$json.dpr,$json.viewport_width,$json.viewport_height) -join '|'
  if ($null -eq $reference) { $reference=$environment }
  elseif ($environment -ne $reference) { throw "Ambiente/viewport variou na rodada $($i+1)" }
  $paintSamples=ValidateNumerical $json.measured 'paint_samples'
  $inputSamples=ValidateNumerical $json.measured 'input_to_paint_samples'
  if ($paintSamples -lt 30 -or $inputSamples -lt 20) {
   throw "Amostragem insuficiente na rodada $($i+1)"
  }
  $entry = [pscustomobject]@{
   mode = $mode
   round = $i+1
   paint_samples = [int]$paintSamples
   paint_p50_ms = ValidateNumerical $json.measured 'paint_p50_ms'
   paint_p95_ms = ValidateNumerical $json.measured 'paint_p95_ms'
   paint_p99_ms = ValidateNumerical $json.measured 'paint_p99_ms'
   input_samples = [int]$inputSamples
   input_to_paint_p95_ms = ValidateNumerical $json.measured 'input_to_paint_p95_ms'
   viewport_bbox_p95_fraction = ValidateNumerical $json.measured 'viewport_bbox_p95_fraction'
   local_requests = [int]$json.overlay_local_requests
   full_fallbacks = [int]$json.overlay_full_fallbacks
  }
  $data += $entry
 }
 $a=@($data | Where-Object mode -eq 'OFF')
 $b=@($data | Where-Object mode -eq 'ON')
 if ($a.Count -ne 3 -or $b.Count -ne 3) { throw 'Quantidade de rodadas incorreta.' }
 $samples = @($data | ForEach-Object { $_.paint_samples })
 $minSamples = [double](($samples | Measure-Object -Minimum).Minimum)
 $maxSamples = [double](($samples | Measure-Object -Maximum).Maximum)
 if ($maxSamples -le 0) { throw 'Nenhuma amostra de pintura valida.' }
 $balance = $minSamples / $maxSamples
 $off95=Mediana @($a | ForEach-Object { $_.paint_p95_ms })
 $on95=Mediana @($b | ForEach-Object { $_.paint_p95_ms })
 $off99=Mediana @($a | ForEach-Object { $_.paint_p99_ms })
 $on99=Mediana @($b | ForEach-Object { $_.paint_p99_ms })
 $offInput=Mediana @($a | ForEach-Object { $_.input_to_paint_p95_ms })
 $onInput=Mediana @($b | ForEach-Object { $_.input_to_paint_p95_ms })
 # Heuristic gate is not a significance test: >=10% worse in BOTH
 # paint P95 and P99 blocks promoting this experiment. No auto-homologation.
 $paintP95Regression = if ($off95 -gt 0) { 100*($on95-$off95)/$off95 } else { 0 }
 $paintP99Regression = if ($off99 -gt 0) { 100*($on99-$off99)/$off99 } else { 0 }
 $performanceGate = if ($balance -lt 0.70) { 'INCONCLUSIVE_SAMPLE_IMBALANCE' }
                    elseif ($paintP95Regression -ge 10 -and $paintP99Regression -ge 10) {
                      'REGRESSION_OBSERVED_DO_NOT_PROMOTE'
                    } else { 'SYNTHETIC_RESULTS_MANUAL_REVIEW_REQUIRED' }
 $gate = if ($balance -lt 0.70) { 'INCONCLUSIVE_SAMPLE_IMBALANCE' }
         else { 'CONTROLLED_SYNTHETIC_MEASURED_MANUAL_PENDING' }
 $report=[ordered]@{
  schema='alencar-laser-controlled-drag-575-ab-19.9'
  status=$gate
  performance_gate=$performanceGate
  regression_threshold_percent=10
  automatic_promotion_allowed=$false
  fixture_sha256=$fixtureSha
  binary_sha256=$binarySha
  environment=$reference
  protocol='OFF-ON-ON-OFF-OFF-ON; 144 mouse moves; 575 contours, 13415 nodes; zoom=1.2'
  paint_sample_balance_ratio=[math]::Round($balance,4)
  medians=[ordered]@{
    viewport_bbox_p95_fraction_off=(Mediana @($a | ForEach-Object { $_.viewport_bbox_p95_fraction }))
    viewport_bbox_p95_fraction_on=(Mediana @($b | ForEach-Object { $_.viewport_bbox_p95_fraction }))
    local_requests_off=($a | Measure-Object -Property local_requests -Sum).Sum
    local_requests_on=($b | Measure-Object -Property local_requests -Sum).Sum
    full_fallbacks_off=($a | Measure-Object -Property full_fallbacks -Sum).Sum
    full_fallbacks_on=($b | Measure-Object -Property full_fallbacks -Sum).Sum
    paint_p95_off_ms=$off95; paint_p95_on_ms=$on95
    paint_p99_off_ms=$off99; paint_p99_on_ms=$on99
    input_to_paint_p95_off_ms=$offInput; input_to_paint_p95_on_ms=$onInput
    paint_p95_reduction_percent=(Reducao $off95 $on95)
    paint_p99_reduction_percent=(Reducao $off99 $on99)
    input_to_paint_p95_reduction_percent=(Reducao $offInput $onInput)
  }
  rounds=$data
  geometry_and_undo_redo_preserved=$true
  manual_visual_validation_pending=$true
  production_homologated=$false
  hardware_commands_sent=$false
  notes=@('Synthetic Qt mouse events, not human gestures.',
          'P95/P99 describe paint samples, not whole interaction throughput.',
          'Viewport bbox metric is paint event bounding rectangle, not exact QRegion pixels.',
          'No automated visual pixel equivalence proof; inspect for stale selection handles.')
 }
 $output=Join-Path $groupDir 'RESUMO_AB_CONTROLADO_575_19_9.json'
 [IO.File]::WriteAllText($output,(($report|ConvertTo-Json -Depth 8)+[Environment]::NewLine),
   [Text.UTF8Encoding]::new($false))
 Write-Host ('Gate sintetico: {0}, razao amostras: {1:N3}' -f $gate,$balance) -ForegroundColor Yellow
 Write-Host ('Gate de desempenho: {0}' -f $performanceGate) -ForegroundColor Yellow
 Write-Host ('P95 pintura OFF/ON: {0} / {1} ms, delta {2}%' -f $off95,$on95,(Reducao $off95 $on95))
 Write-Host ('P99 pintura OFF/ON: {0} / {1} ms, delta {2}%' -f $off99,$on99,(Reducao $off99 $on99))
 Write-Host ('P95 entrada-pintura OFF/ON: {0} / {1} ms, delta {2}%' -f $offInput,$onInput,(Reducao $offInput $onInput))
 Write-Host ('Repintura bbox P95 fracao OFF/ON: {0:N3} / {1:N3}' -f (Mediana @($a | ForEach-Object { $_.viewport_bbox_p95_fraction })),(Mediana @($b | ForEach-Object { $_.viewport_bbox_p95_fraction })))
 Write-Host ('Requisicoes local ON: {0}, fallbacks full ON: {1}' -f (($b | Measure-Object -Property local_requests -Sum).Sum),(($b | Measure-Object -Property full_fallbacks -Sum).Sum))
 if ($performanceGate -eq 'REGRESSION_OBSERVED_DO_NOT_PROMOTE') {
  Write-Warning 'REGRESSAO CONTROLADA: modo ON tem P95 e P99 >=10% piores. NAO PROMOVER.'
 }
 Write-Host "Resumo JSON local: $output" -ForegroundColor Green
 Write-Host 'Sem Git push e sem hardware. Homologacao visual/produçao pendente.'
}
finally {
 $env:PATH = $oldPath
 [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$oldOverlay,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG',$oldGrouped,'Process')
}
