#requires -Version 5.1
<#
Phase 20i: 575-object transform OFF/ON stress, isolated Qt executable.
No laser IO, no installed executable changes, no promotion.
#>
[CmdletBinding()]
param([ValidateRange(1,5)][int]$Pares=3)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$src=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $src 'build_WINDOWS_DRAG_AB_19_9'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$dir=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\TRANSFORM_STRESS_20I'
$session=Join-Path $dir ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Path $session -Force)
$out=Join-Path $session 'RESULTADO_TRANSFORM_STRESS_20I.json'
$oldPath=$env:PATH
$oldFlag=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
$rows=@()
function Save([string]$status,[string]$note) {
 $data=[ordered]@{
  schema='alencar-laser-transform-stress-20i-1'
  status=$status;note=$note
  pairs=$Pares;executed=@($rows)
  software_only=$true;geometry_and_history_required=$true
  installed_application_modified=$false
  production_homologated=$false;hardware_commands_sent=$false
  speedup_claim_allowed=$false
 }
 [IO.File]::WriteAllText($out,($data|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
}
function Run([int]$pair,[string]$mode) {
 $sub=Join-Path $session ('PAR_'+$pair+'_'+$mode)
 [void](New-Item -ItemType Directory -Path $sub -Force)
 if($mode -eq 'ON'){$env:ALENCAR_TRANSFORM_LOCAL_REPAINT_20H='1'}
 else{[Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')}
 & $exe --grouped-center-transform-575-20i-test '-' $sub
 if($LASTEXITCODE -ne 0){throw "Teste $pair $mode falhou com exit $LASTEXITCODE"}
 $file=Join-Path $sub 'RESULTADO_TRANSFORMACAO_CENTRAL_575_20I.json'
 if(-not (Test-Path $file -PathType Leaf)){throw "JSON ausente: $file"}
 $r=Get-Content -LiteralPath $file -Raw -Encoding UTF8 | ConvertFrom-Json
 if($r.status -ne 'PASS' -or [int]$r.selected_objects -ne 575 -or
    $r.undo_redo_preserved -ne $true -or
    $r.document_unchanged_during_preview -ne $true -or
    $r.hardware_commands_sent -ne $false -or
    $r.transform_local_repaint_optin -ne ($mode -eq 'ON') -or
    [string]::IsNullOrWhiteSpace([string]$r.preview_sha256) -or
    [double]$r.paint_p95_ms -le 0 -or [double]$r.paint_p99_ms -le 0 -or
    [int]$r.paint_samples -lt 6 -or
    [double]$r.viewport_bbox_p95_fraction -gt 1 -or
    [double]$r.viewport_bbox_p95_fraction -lt 0){
    throw "Regressao de dados ou estado $pair $mode"
 }
 if($mode -eq 'ON' -and ([double]$r.transform_local_update_requests -lt 1 -or
    [double]$r.transform_full_fallbacks -gt 0)){
    throw "ON falhou na invalidacao localizada $pair"
 }
 if($mode -eq 'OFF' -and [double]$r.transform_local_update_requests -ne 0){
    throw "OFF ativou o experimento inesperadamente"
 }
 return $r
}
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'fix/phase20h-transform-repaint-20261106'){
  throw "Branch diferente da experimental: $branch"
 }
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 if(-not (Test-Path $cmake -PathType Leaf)){throw 'CMake UCRT64 ausente'}
 Write-Warning 'Mantenha o laser fisicamente desconectado.'
 & $cmake -S $src -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Falha na configuracao CMake'}
 & $cmake --build $build --parallel 1 --target alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Falha na compilacao Qt'}
 if(-not (Test-Path $exe -PathType Leaf)){throw 'Binario Qt isolado ausente'}
 for($n=1;$n -le $Pares;$n++){
  # Alternate ordering across pairs to mitigate fixed ordering bias.
  $order=$(if($n%2 -eq 1){@('OFF','ON')}else{@('ON','OFF')})
  $outputs=@{}
  foreach($mode in $order){
   Write-Host ("Par {0}/{1}: {2}" -f $n,$Pares,$mode) -ForegroundColor Cyan
   $outputs[$mode]=Run $n $mode
  }
  $a=$outputs['OFF'];$b=$outputs['ON']
  if($a.preview_sha256 -ne $b.preview_sha256){
   throw "BLOQUEIO VISUAL: hashes de pixels diferentes no par $n"
  }
  $rows+= [pscustomobject]@{
   pair=$n
   off_p95_ms=[double]$a.paint_p95_ms
   on_p95_ms=[double]$b.paint_p95_ms
   off_p99_ms=[double]$a.paint_p99_ms
   on_p99_ms=[double]$b.paint_p99_ms
   off_bbox_p95=[double]$a.viewport_bbox_p95_fraction
   on_bbox_p95=[double]$b.viewport_bbox_p95_fraction
   off_samples=[int]$a.paint_samples
   on_samples=[int]$b.paint_samples
   equal_pixels=$true
   geometry_undo_redo_preserved=$true
  }
  Save 'IN_PROGRESS' ("Par $n validado")
 }
 Save 'SYNTHETIC_PASS_MANUAL_PENDING' 'Cenarios 575 equivalentes; ganho causal ainda nao demonstrado'
 $rows|Format-Table pair,off_p95_ms,on_p95_ms,off_bbox_p95,on_bbox_p95,equal_pixels -AutoSize
 Write-Host 'PASS 20i: equivalencia sintetica. Nao homologado; medir DPI, mouse e projetos reais.' -ForegroundColor Green
 Write-Host "Resumo: $out" -ForegroundColor Cyan
}catch{
 Save 'BLOCKED' $_.Exception.Message
 Write-Host "FALHA 20i: $($_.Exception.Message)" -ForegroundColor Red
 Write-Host "Resumo: $out" -ForegroundColor Yellow
 throw
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldFlag,'Process')
}
