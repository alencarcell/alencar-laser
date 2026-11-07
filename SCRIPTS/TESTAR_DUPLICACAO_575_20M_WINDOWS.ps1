#requires -Version 5.1
<#
ALENCAR LASER - Correcao Ctrl+D em lote (20m), somente offline.
Compila e executa regressao C++ (575+grupos/texto) e Qt Ctrl+D 575.
Nao altera APLICATIVO, instalador, projetos ou laser.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$src=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $src 'build_WINDOWS_DUPLICAR_20M'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$bin='C:\msys64\ucrt64\bin'
$exeCore=Join-Path $build 'alencar_test20m_batch_duplicate.exe'
$exeGui=Join-Path $build 'alencar_test7_qt6.exe'
$reportDir=Join-Path $root ('RELATORIOS\AUDITORIAS\GITHUB_19_8\DUPLICACAO_20M\'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
$json=Join-Path $reportDir 'RESULTADO_DUPLICACAO_575_20M.json'
$log=Join-Path $reportDir 'RESULTADO_GATE_20M.json'
$oldPath=$env:PATH
$oldQt=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$oldViewport=[Environment]::GetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L','Process')
$oldWheel=[Environment]::GetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','Process')
$oldTransform=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
[void](New-Item -ItemType Directory -Path $reportDir -Force)
$status='BLOCKED'
$reason=''
try {
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'fix/duplicate-batch-20m-20261107'){
  throw "Branch inesperada: $branch"
 }
 if(-not (Test-Path -LiteralPath $cmake -PathType Leaf)){
  throw 'CMake UCRT64 ausente'
 }
 $env:PATH=$bin+';'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 foreach($name in @('ALENCAR_EVENT_PAINT_TRACE_20J','ALENCAR_MINIMAL_VIEWPORT_20L',
   'ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H')){
   [Environment]::SetEnvironmentVariable($name,$null,'Process')
 }
 Write-Warning 'Maquina laser deve permanecer FISICAMENTE desconectada.'
 & $cmake -S $src -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test20m_batch_duplicate alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao dos testes falhou'}
 if(-not (Test-Path -LiteralPath $exeCore -PathType Leaf) -or
    -not (Test-Path -LiteralPath $exeGui -PathType Leaf)){
  throw 'Executaveis de validacao nao encontrados'
 }
 Write-Host 'G01: nucleo - 575 objetos, grupos, textos, camadas e Undo/Redo...' -ForegroundColor Cyan
 & $exeCore
 if($LASTEXITCODE -ne 0){throw "Regressao C++ 20m falhou (exit $LASTEXITCODE)"}
 Write-Host 'G02: GUI Qt - Ctrl+D real com 575 selecionados...' -ForegroundColor Cyan
 & $exeGui --duplicate-batch-575-20m-test '-' $reportDir
 $exit=$LASTEXITCODE
 if(-not (Test-Path -LiteralPath $json -PathType Leaf)){
  throw "Teste Qt nao gerou JSON (exit $exit)"
 }
 $result=Get-Content -LiteralPath $json -Raw -Encoding UTF8|ConvertFrom-Json
 if($exit -ne 0 -or $result.status -ne 'PASS'){
  $details=if($result.error){[string]$result.error}else{"exit $exit"}
  throw "Ctrl+D Qt falhou: $details"
 }
 if([int]$result.original_objects -ne 575 -or
    [int]$result.duplicated_objects -ne 575 -or
    [int]$result.final_contours -ne 1150 -or
    $result.undo_atomic -ne $true -or
    $result.redo_exact -ne $true -or
    $result.pixels_geometry_match -ne $true -or
    $result.hardware_commands_sent -ne $false){
  throw 'Regressao Qt: invariantes do teste nao foram confirmados'
 }
 $status='AUTOMATED_PASS_MANUAL_PENDING'
 Write-Host 'PASS 20m: duplicacao Ctrl+D em lote, Undo/Redo e integridade C++/Qt.' -ForegroundColor Green
 Write-Host 'Ainda falta validar com desenhos reais; versao instalada nao alterada.' -ForegroundColor Yellow
}catch{
 $reason=$_.Exception.Message
 Write-Host "FALHA 20m: $reason" -ForegroundColor Red
 throw
}finally{
 $meta=[ordered]@{
  schema='alencar-laser-duplicate-batch-gate-20m-1'
  status=$status;error=$reason
  core_575_test_required=$true
  gui_575_test_required=$true
  installed_application_modified=$false
  production_homologated=$false
  hardware_commands_sent=$false
  result_json=$json
 }
 [IO.File]::WriteAllText($log,($meta|ConvertTo-Json -Depth 6),[Text.UTF8Encoding]::new($false))
 Write-Host "Resumo local: $log" -ForegroundColor Cyan
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldQt,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$oldViewport,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$oldWheel,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldTransform,'Process')
}
