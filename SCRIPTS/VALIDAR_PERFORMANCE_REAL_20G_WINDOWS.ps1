#requires -Version 5.1
<#
Fase20g: A/B reprodutivel da GUI Qt + leitura de um diagnostico REAL.
Somente relatorios locais, build de desenvolvimento isolado; sem comandos laser.
NENHUMA otimização e ativada neste script.
#>
[CmdletBinding()]
param(
  [string]$Diagnostico = '',
  [ValidateRange(1,3)][int]$Repeticoes = 2,
  [switch]$SomenteDiagnostico
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$dir=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\FASE20G_GUI_REAL'
$session=Join-Path $dir ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Path $session -Force)
$reportFile=Join-Path $session 'RESULTADO_FASE20G.json'
$real=$null
$comparisons=@()
$steps=@()
$originalEnv=[Environment]::GetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9','Process')
function Save([string]$status,[string]$message){
 $report=[ordered]@{
  schema='alencar-laser-phase20g-gui-real-1'
  status=$status
  message=$message
  generated_utc=(Get-Date).ToUniversalTime().ToString('o')
  test_branch='test/phase20g-real-gui-perf-20261106'
  diagnostic=$real
  comparisons=@($comparisons)
  steps=@($steps)
  actual_mouse_and_visual_homologated=$false
  production_homologated=$false
  hardware_commands_sent_by_runner=$false
  no_speedup_claim=$true
 }
 [IO.File]::WriteAllText($reportFile,($report|ConvertTo-Json -Depth 12),[Text.UTF8Encoding]::new($false))
}
function Capture([string]$id,[string]$scriptPath,[string[]]$args){
 $stdout=Join-Path $session ($id+'_OUT.txt')
 $stderr=Join-Path $session ($id+'_ERR.txt')
 Write-Host "TESTANDO: $id" -ForegroundColor Cyan
 $command=@('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$scriptPath+'"'))
 foreach($part in $args){
  if($part.Contains('"')){throw 'Aspas em argumento nao permitido'}
  $command+= $(if($part -match '\s') { '"'+$part+'"' }else{$part})
 }
 $p=Start-Process -FilePath 'powershell.exe' -ArgumentList $command -WorkingDirectory $root -NoNewWindow -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
 $steps+= [ordered]@{id=$id;exit_code=$p.ExitCode;stdout=$stdout;stderr=$stderr}
 if($p.ExitCode -ne 0){
  Write-Host "FALHA $id - veja o primeiro erro real:" -ForegroundColor Red
  if(Test-Path $stderr){Get-Content $stderr -Tail 25 | Out-Host}
  if(Test-Path $stdout){Get-Content $stdout -Tail 40 | Out-Host}
  throw "Etapa $id falhou (codigo $($p.ExitCode))"
 }
 Write-Host "PASS $id" -ForegroundColor Green
}
try {
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'test/phase20g-real-gui-perf-20261106'){
  throw "Branch incorreta ($branch); rode no branch de diagnostico 20g."
 }
 if([string]::IsNullOrWhiteSpace($Diagnostico)){
  $dirs=@(
   (Join-Path $root 'RELATORIOS'),
   (Join-Path $env:USERPROFILE 'Downloads')
  )
  $files=@()
  foreach($d in $dirs){
   if(Test-Path -LiteralPath $d -PathType Container){
    $files+=@(Get-ChildItem -LiteralPath $d -File -Recurse -Filter 'Alencar_Laser_Diagnostico_TESTE_19_7*.json' -ErrorAction SilentlyContinue)
   }
  }
  if($files.Count -gt 0){
   $Diagnostico=($files | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1).FullName
  }
 }
 if([string]::IsNullOrWhiteSpace($Diagnostico) -or -not (Test-Path -LiteralPath $Diagnostico -PathType Leaf)){
  throw 'JSON de diagnostico real ausente: informe -Diagnostico caminho_completo.json.'
 }
 $source=Get-Content -LiteralPath $Diagnostico -Encoding UTF8 -Raw | ConvertFrom-Json
 if($source.schema -ne 'alencar-laser-diagnostics-4' -or
    $source.environment.machine_connected -ne $false -or
    $source.environment.machine_transport -ne 'none' -or
    $source.machine_diagnostics.diagnostic_probes_sent -ne 0){
  throw 'Diagnostico invalido, de esquema desconhecido ou com hardware conectado.'
 }
 $perf=$source.performance
 $p50=[double]$perf.paint_p50_ms
 $p95=[double]$perf.paint_p95_ms
 $p99=[double]$perf.paint_p99_ms
 $max=[double]$perf.paint_max_ms
 $input95=[double]$perf.input_to_paint_p95_ms
 $preview95=[double]$perf.transform_preview_p95_ms
 $bbox=[double]$perf.viewport_paint_bbox_fraction_p95
 $samples=[int]$perf.paint_samples
 if($samples -lt 50 -or $p50 -le 0 -or $p95 -lt $p50 -or $p99 -lt $p95 -or
    $max -lt $p99 -or $input95 -lt 0 -or $preview95 -lt 0 -or
    $bbox -lt 0 -or $bbox -gt 1){
  throw 'Tempos/contadores incoerentes no diagnostico.'
 }
 $real=[ordered]@{
  filename=(Split-Path -Leaf $Diagnostico)
  schema=$source.schema
  session_ms=[double]$source.elapsed_ms
  polyline_contours=[int]$source.project_summary.polyline_contours
  bezier_contours=[int]$source.project_summary.bezier_contours
  selected_move_operations=[int]$source.counts.shapes_moved
  marquee_operations=[int]$source.counts.marquee_selected
  paint_samples=$samples
  paint_p50_ms=$p50;paint_p95_ms=$p95;paint_p99_ms=$p99
  paint_max_ms=$max
  input_to_paint_p95_ms=$input95
  grouped_transform_updates=[int]$perf.grouped_transform_frame_updates
  transform_preview_p95_ms=$preview95
  viewport_bbox_fraction_p95=$bbox
  high_paint_latency_observed=($p95 -gt 33.3)
  full_viewport_p95_observed=($bbox -ge 0.99)
 }
 Write-Host ('JSON real: paint P95 {0:N1} ms; P99 {1:N1} ms; max {2:N1} ms' -f $p95,$p99,$max)
 Write-Host ('Transform preview P95 {0:N2} ms; viewport bbox P95 {1:P0}' -f $preview95,$bbox)
 Save 'REAL_DIAGNOSTIC_VALIDATED' 'Dados reais lidos; nenhum mecanismo causal inferido'
 if(-not $SomenteDiagnostico){
  Write-Warning 'Desconecte fisicamente o laser. Este ensaio compila somente uma copia isolada Qt.'
  $script40=Join-Path $PSScriptRoot 'COMPARAR_DRAG_OVERLAY_19_9.ps1'
  $script575=Join-Path $PSScriptRoot 'COMPARAR_DRAG_CONTROLADO_575_19_9.ps1'
  foreach($required in @($script40,$script575)){
   if(-not (Test-Path -LiteralPath $required -PathType Leaf)){throw "Script ausente: $required"}
  }
  [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$null,'Process')
  Capture 'G01_REGRESSAO_40' $script40 @()
  for($n=1;$n -le $Repeticoes;$n++){
   $before=(Get-Date).ToUniversalTime()
   Capture ("G02_CONTROLADO_575_"+$n) $script575 @('-ModoOtimizado','3')
   $base=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
   $matching=@(Get-ChildItem -LiteralPath $base -Directory -Filter 'CONTROLADO_575_*' |
     Where-Object {$_.LastWriteTimeUtc -ge $before.AddSeconds(-5)} |
     Sort-Object LastWriteTimeUtc -Descending)
   $found=$null
   foreach($folder in $matching){
    $candidate=Join-Path $folder.FullName 'RESUMO_AB_CONTROLADO_575_19_9.json'
    if(Test-Path -LiteralPath $candidate -PathType Leaf){$found=$candidate;break}
   }
   if($null -eq $found){throw 'Nao localizei JSON da rodada controlada atual'}
   $data=Get-Content -LiteralPath $found -Encoding UTF8 -Raw | ConvertFrom-Json
   if($data.status -ne 'CONTROLLED_SYNTHETIC_MEASURED_MANUAL_PENDING' -or
      $data.performance_gate -ne 'EQUIVALENT_REPAINT_POLICY_NO_SPEEDUP_CLAIM' -or
      $data.speedup_claim_allowed -ne $false -or
      $data.hardware_commands_sent -ne $false -or
      @($data.rounds).Count -ne 6){
    throw 'Resultado A/B invalido ou flag indevida'
   }
   $comparisons+= [ordered]@{
    round=$n
    source_report=$found
    sample_balance=[double]$data.paint_sample_balance_ratio
    off_p95_ms=[double]$data.medians.paint_p95_off_ms
    on_p95_ms=[double]$data.medians.paint_p95_on_ms
    repaint_equivalent=$true
    causal_speedup_established=$false
   }
   Save 'IN_PROGRESS' ("Rodada A/B $n passou")
  }
 }
 Save 'AUTOMATED_PASS_MANUAL_PENDING' 'Testes sinteticos passaram; lentidao real e QA manual pendentes'
 Write-Host 'FASE20g: PASS tecnico, desempenho humano NAO homologado.' -ForegroundColor Green
 Write-Host "Resumo: $reportFile"
}catch{
 Save 'BLOCKED' $_.Exception.Message
 Write-Host "FASE20g BLOQUEADA: $($_.Exception.Message)" -ForegroundColor Red
 Write-Host "Resumo: $reportFile"
 throw
}finally{
 [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$originalEnv,'Process')
}
