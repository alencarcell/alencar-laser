#requires -Version 5.1
<#
Fase 20l: recupera diagnostico OFF anterior e mede ON no editor real.
Exportacao permanece manual; arquivo detectado com validacao por horario.
Nao altera projetos, instalador, main ou comunicacao com laser.
#>
[CmdletBinding()]
param([string]$ArquivoOff='')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_VIEWPORT_AB_20L'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$fixture=Join-Path $source 'fixtures\web_nodes_normalized.csv'
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\VIEWPORT_REAL_20L'
$run=Join-Path $reports ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Force -Path $run)
$oldPath=$env:PATH
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$oldMinimal=[Environment]::GetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L','Process')
$oldWheel=[Environment]::GetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','Process')
$oldLocal=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
function ReadDiagnostic([string]$file,[string]$mode) {
 if(-not (Test-Path -LiteralPath $file -PathType Leaf)){
  throw "Diagnostico $mode nao encontrado: $file"
 }
 $d=Get-Content -LiteralPath $file -Raw -Encoding UTF8|ConvertFrom-Json
 if($d.schema -ne 'alencar-laser-diagnostics-4' -or
    $d.environment.machine_connected -ne $false -or
    $d.machine_diagnostics.diagnostic_probes_sent -ne 0 -or
    $d.performance.event_paint_trace_20j.enabled -ne $true -or
    [int]$d.performance.paint_samples -lt 50){
  throw "Diagnostico $mode invalido ou sem dados suficientes"
 }
 if($mode -eq 'ON' -and $d.performance.viewport_update_strategy -ne 'minimal'){
  throw 'Diagnostico ON nao confirma MinimalViewportUpdate. Atualize/recompile o Qt.'
 }
 if($mode -eq 'OFF' -and $d.performance.viewport_update_strategy -ne 'bounding_rect'){
  throw 'Diagnostico OFF nao confirma BoundingRectViewportUpdate.'
 }
 return $d
}
function NewestExport([datetime]$since){
 $locations=@((Join-Path $env:USERPROFILE 'Downloads'),$reports)
 $found=@()
 foreach($folder in $locations){
  if(Test-Path -LiteralPath $folder -PathType Container){
   $found+=@(Get-ChildItem -LiteralPath $folder -File -Filter 'Alencar_Laser_Diagnostico_TESTE_19_7*.json' -ErrorAction SilentlyContinue |
     Where-Object {$_.LastWriteTime -ge $since} )
  }
 }
 return @($found|Sort-Object LastWriteTimeUtc -Descending)
}
function LocateExport([string]$destination,[datetime]$since,[string]$mode){
 if(Test-Path -LiteralPath $destination -PathType Leaf){return $destination}
 $candidates=New-Object System.Collections.ArrayList
 foreach($f in @(NewestExport $since)){
  try{
   $d=ReadDiagnostic $f.FullName $mode
   [void]$candidates.Add($f.FullName)
  }catch{}
 }
 if($candidates.Count -ge 1){
  if($candidates.Count -gt 1){
   Write-Warning 'Mais de um JSON recente: usando o mais novo. Verifique se pertence a esta sessao.'
  }
  Copy-Item -LiteralPath ([string]$candidates[0]) -Destination $destination -ErrorAction Stop
  Write-Host ("Diagnostico $mode localizado automaticamente: "+[string]$candidates[0]) -ForegroundColor Green
  return $destination
 }
 throw ("Nao encontrei o diagnostico {0}. Exporte no painel Diagnosticos exatamente como {1}; se o arquivo ja existe, copie-o para esse caminho e execute de novo." -f $mode,$destination)
}
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'exp/phase20l-minimal-viewport-20261107'){
  throw "Branch incorreta: $branch"
 }
 if(-not (Test-Path -LiteralPath $fixture -PathType Leaf)){throw 'Fixture de teste ausente'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
 $env:ALENCAR_EVENT_PAINT_TRACE_20J='1'
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 Write-Warning 'Maquina laser deve permanecer FISICAMENTE desconectada.'
 # Build the actual ON-reporting binary, not a stale executable.
 $cmake='C:\msys64\ucrt64\bin\cmake.exe'
 if(-not (Test-Path -LiteralPath $cmake -PathType Leaf)){throw 'CMake UCRT64 ausente'}
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Falha ao configurar Qt'}
 & $cmake --build $build --parallel 1 --target alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Falha ao compilar Qt'}
 if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Executavel de laboratorio nao encontrado'}
 $offDir=Join-Path $run 'OFF'
 $onDir=Join-Path $run 'ON'
 [void](New-Item -ItemType Directory -Force -Path $offDir)
 [void](New-Item -ItemType Directory -Force -Path $onDir)
 $offFile=Join-Path $offDir 'diagnostico.json'
 if([string]::IsNullOrWhiteSpace($ArquivoOff)){
  $latest=@(NewestExport (Get-Date).AddDays(-2))
  foreach($candidate in $latest){
   try{$off=ReadDiagnostic $candidate.FullName 'OFF';$ArquivoOff=$candidate.FullName;break}catch{}
  }
  if([string]::IsNullOrWhiteSpace($ArquivoOff)){
   throw 'Nao achei diagnostico OFF recente em Downloads. Rode novamente com -ArquivoOff "C:\caminho\diagnostico.json"'
  }
  Write-Host ("OFF recuperado: "+$ArquivoOff) -ForegroundColor Green
 }
 $off=ReadDiagnostic $ArquivoOff 'OFF'
 Copy-Item -LiteralPath $ArquivoOff -Destination $offFile
 $env:ALENCAR_MINIMAL_VIEWPORT_20L='1'
 $onFile=Join-Path $onDir 'diagnostico.json'
 $started=(Get-Date).AddSeconds(-5)
 Write-Host 'ABRINDO ON: repita zoom, arraste, selecao e pan; exporte JSON antes de fechar.' -ForegroundColor Cyan
 Write-Host ("Destino de exportacao: "+$onFile) -ForegroundColor Yellow
 & $exe --interactive $fixture $onDir
 if($LASTEXITCODE -ne 0){throw ("Editor ON encerrou com codigo {0}" -f $LASTEXITCODE)}
 $onFile=LocateExport $onFile $started 'ON'
 $on=ReadDiagnostic $onFile 'ON'
 $rows=@()
 foreach($category in @('zoom','transform','marquee','mouse_move','idle')){
  $a=@($off.performance.event_paint_trace_20j.groups|Where-Object {$_.event -eq $category})
  $b=@($on.performance.event_paint_trace_20j.groups|Where-Object {$_.event -eq $category})
  if($a.Count -eq 1 -and $b.Count -eq 1){
   $rows+=[pscustomobject]@{
    evento=$category
    off_amostras=[int]$a[0].samples
    on_amostras=[int]$b[0].samples
    off_p95_ms=[double]$a[0].p95_ms
    on_p95_ms=[double]$b[0].p95_ms
   }
  }
 }
 $rows|Format-Table -AutoSize
 $offErrors=if($off.counts.command_error){[int]$off.counts.command_error}else{0}
 $onErrors=if($on.counts.command_error){[int]$on.counts.command_error}else{0}
 if($offErrors -gt 0 -or $onErrors -gt 0){
  Write-Warning ("Erros de comando registrados: OFF={0}, ON={1}. O JSON nao informa a causa individual." -f $offErrors,$onErrors)
 }
 $sameGeometry=([int]$off.project_summary.polyline_contours -eq [int]$on.project_summary.polyline_contours -and
                [int]$off.project_summary.polyline_nodes -eq [int]$on.project_summary.polyline_nodes)
 $summary=[ordered]@{
  schema='alencar-laser-viewport-real-ab-20l-2'
  status= $(if($sameGeometry){'MEASURED_MANUAL_REVIEW'}else{'NOT_COMPARABLE_DIFFERENT_GEOMETRY'})
  off_contours=[int]$off.project_summary.polyline_contours
  on_contours=[int]$on.project_summary.polyline_contours
  off_nodes=[int]$off.project_summary.polyline_nodes
  on_nodes=[int]$on.project_summary.polyline_nodes
  off_command_errors=$offErrors
  on_command_errors=$onErrors
  manual_functional_review_required=($offErrors -gt 0 -or $onErrors -gt 0)
  same_final_geometry_counts=$sameGeometry
  recorded_operations_not_identical=$true
  no_causal_speedup_claim=$true
  hardware_commands_sent_by_script=$false
  production_homologated=$false
  rows=$rows
 }
 $report=Join-Path $run 'COMPARACAO_RESUMO.json'
 [IO.File]::WriteAllText($report,($summary|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
 if(-not $sameGeometry){
  Write-Warning 'OFF/ON terminaram com quantidades diferentes de nos ou contornos. Tempos NAO sao comparacao justa!'
 }
 Write-Host ("Relatorio local: "+$report) -ForegroundColor Cyan
 Write-Host 'NENHUMA promocao automatica. Compare tambem as mesmas operacoes manualmente.' -ForegroundColor Yellow
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$oldMinimal,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$oldWheel,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldLocal,'Process')
}
