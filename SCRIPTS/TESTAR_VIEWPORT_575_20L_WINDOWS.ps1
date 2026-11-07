#requires -Version 5.1
<#
Fase20l: A/B zoom OFF/ON, 575 contornos, 12 eventos de zoom.
Qt development build only, no app install, no physical laser IO.
#>
[CmdletBinding()]
param([ValidateRange(1,5)][int]$Pares=3)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_VIEWPORT_AB_20L'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$dest=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\VIEWPORT_AB_20L'
$run=Join-Path $dest ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Force -Path $run)
$summary=Join-Path $run 'RESULTADO_VIEWPORT_AB_20L.json'
$oldPath=$env:PATH
$oldOpt=[Environment]::GetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L','Process')
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$oldWheel=[Environment]::GetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','Process')
$oldTransform=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
$rows=@()
function Save([string]$status,[string]$message){
 $result=[ordered]@{
  schema='alencar-laser-viewport-ab-20l-1'
  status=$status;message=$message
  pairs_requested=$Pares
  pairs_completed=@($rows)
  change_enabled_in_production=$false
  hardware_commands_sent=$false
  production_homologated=$false
  speedup_claim_allowed=$false
 }
 [IO.File]::WriteAllText($summary,($result|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
}
function Run([int]$n,[string]$mode){
 $d=Join-Path $run ('PAR_'+$n+'_'+$mode)
 [void](New-Item -ItemType Directory -Force -Path $d)
 if($mode -eq 'ON'){$env:ALENCAR_MINIMAL_VIEWPORT_20L='1'}
 else{[Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$null,'Process')}
 & $exe --wheel-zoom-dense-575-20k-test '-' $d
 $exit=$LASTEXITCODE
 $json=Join-Path $d 'RESULTADO_ZOOM_575_20K.json'
 if(-not (Test-Path -LiteralPath $json -PathType Leaf)){throw "Teste $mode nao gerou JSON (exit $exit)"}
 $r=Get-Content -LiteralPath $json -Raw -Encoding UTF8|ConvertFrom-Json
 if($exit -ne 0 -or $r.status -ne 'PASS'){
  $reason=if($r.error){[string]$r.error}else{"exit $exit"}
  throw "Falha Qt no par $n modo $mode : $reason"
 }
 if([int]$r.objects -ne 575 -or [int]$r.wheel_events -ne 12 -or
    $r.minimal_viewport_20l -ne ($mode -eq 'ON') -or
    $r.geometry_unchanged -ne $true -or $r.undo_unchanged -ne $true -or
    $r.hardware_commands_sent -ne $false -or
    [int]$r.paint_samples -lt 6 -or
    [double]$r.paint_p95_ms -le 0 -or [double]$r.paint_p99_ms -le 0 -or
    [string]::IsNullOrWhiteSpace([string]$r.viewport_sha256)){
   throw "Dados Qt incompletos no par $n modo $mode"
 }
 return $r
}
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'exp/phase20l-minimal-viewport-20261107'){
  throw "Branch diferente da experimental: $branch"
 }
 if(-not (Test-Path -LiteralPath $cmake -PathType Leaf)){throw 'CMake UCRT64 ausente'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 Write-Warning 'Mantenha a maquina laser fisicamente desconectada.'
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao Qt falhou'}
 if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Qt binario ausente'}
 for($n=1;$n -le $Pares;$n++){
  $order=if($n%2 -eq 1){@('OFF','ON')}else{@('ON','OFF')}
  $values=@{}
  foreach($mode in $order){
   Write-Host ("VIEWPORT {0}/{1}: {2}" -f $n,$Pares,$mode) -ForegroundColor Cyan
   $values[$mode]=Run $n $mode
  }
  $a=$values['OFF'];$b=$values['ON']
  if($a.viewport_sha256 -ne $b.viewport_sha256){
   throw "REGRESSAO VISUAL no par $n : hashes de frames diferentes"
  }
  $rows+= [pscustomobject]@{
   par=$n
   off_p95_ms=[double]$a.paint_p95_ms
   on_p95_ms=[double]$b.paint_p95_ms
   off_p99_ms=[double]$a.paint_p99_ms
   on_p99_ms=[double]$b.paint_p99_ms
   off_bbox_p95=[double]$a.viewport_bbox_p95_fraction
   on_bbox_p95=[double]$b.viewport_bbox_p95_fraction
   visual_equal=$true
   geometry_undo_equal=$true
  }
  Save 'IN_PROGRESS' ("Par $n validado")
 }
 $regressions=@($rows|Where-Object {$_.on_p95_ms -gt $_.off_p95_ms})
 if($regressions.Count -eq $rows.Count){
  Save 'PERFORMANCE_REGRESSION' 'Todos os pares ON pioraram em P95, manter flag OFF.'
  Write-Warning 'REGRESSAO: opt-in ON piorou os P95 de todos os pares.'
 }else{
  Save 'VISUAL_PASS_PERFORMANCE_INCONCLUSIVE' 'Pixels equivalentes; ganho real ainda nao homologado.'
  Write-Host 'PASS visual. Nenhuma promocao automatica.' -ForegroundColor Green
 }
 $rows|Format-Table par,off_p95_ms,on_p95_ms,off_bbox_p95,on_bbox_p95,visual_equal -AutoSize
 Write-Host "Relatorio: $summary" -ForegroundColor Cyan
}catch{
 Save 'BLOCKED' $_.Exception.Message
 Write-Host ("FALHA FASE20l: "+$_.Exception.Message) -ForegroundColor Red
 Write-Host "Relatorio: $summary" -ForegroundColor Yellow
 throw
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$oldOpt,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$oldWheel,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldTransform,'Process')
}
