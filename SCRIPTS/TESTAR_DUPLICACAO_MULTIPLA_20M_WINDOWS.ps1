#requires -Version 5.1
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_CTRL_D_20M'
$dir=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\CTRL_D_20M'
$session=Join-Path $dir ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Force -Path $session)
$summary=Join-Path $session 'RESULTADO_CTRL_D_20M.json'
$oldPath=$env:PATH
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
$oldMinimal=[Environment]::GetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L','Process')
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$steps=@()
function Save([string]$status,[string]$message){
 $record=[ordered]@{
  schema='alencar-laser-ctrl-d-20m-1'
  status=$status;message=$message
  steps=@($steps)
  machine_commands_sent=$false
  installed_application_modified=$false
  production_homologated=$false
  sample_project_used='fixture_web_nodes_normalized'
 }
 [IO.File]::WriteAllText($summary,($record|ConvertTo-Json -Depth 6),[Text.UTF8Encoding]::new($false))
}
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'fix/phase20m-bulk-duplicate-20261107'){
  throw "Branch de teste incorreta: $branch"
 }
 $cmake='C:\msys64\ucrt64\bin\cmake.exe'
 if(-not (Test-Path -LiteralPath $cmake -PathType Leaf)){throw 'CMake UCRT64 ausente'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$null,'Process')
 Write-Warning 'Laser fisicamente desconectado. Teste de software somente.'
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test_bulk_duplicate_20m alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Falha na compilacao; capture o primeiro erro vermelho com arquivo e linha'}
 $fixture=Join-Path $source 'fixtures\web_nodes_normalized.csv'
 $core=Join-Path $build 'alencar_test_bulk_duplicate_20m.exe'
 $qt=Join-Path $build 'alencar_test7_qt6.exe'
 foreach($required in @($fixture,$core,$qt)){
  if(-not (Test-Path -LiteralPath $required -PathType Leaf)){throw "Ausente: $required"}
 }
 & $core $fixture
 $exit=$LASTEXITCODE
 $steps+= [pscustomobject]@{name='CORE_575_GROUPS_TEXT_LOCKS';exit_code=$exit}
 if($exit -ne 0){throw "Teste do nucleo falhou (exit $exit)"}
 $guiDir=Join-Path $session 'QT_GUI'
 [void](New-Item -ItemType Directory -Force -Path $guiDir)
 & $qt --bulk-duplicate-shortcut-575-20m-test '-' $guiDir
 $exit=$LASTEXITCODE
 $json=Join-Path $guiDir 'RESULTADO_CTRL_D_575_20M.json'
 $steps+= [pscustomobject]@{name='GUI_CTRL_D_575';exit_code=$exit;report=$json}
 if(-not (Test-Path -LiteralPath $json -PathType Leaf)){throw "Qt nao gerou resultado (exit $exit)"}
 $report=Get-Content -LiteralPath $json -Raw -Encoding UTF8|ConvertFrom-Json
 if($exit -ne 0 -or $report.status -ne 'PASS'){
  $reason=if($report.error){[string]$report.error}else{"exit $exit"}
  throw "Falha Qt: $reason"
 }
 if([int]$report.objects_duplicated -ne 575 -or
    $report.undo_redo_atomic -ne $true -or
    $report.node_count_preserved -ne $true -or
    $report.hardware_commands_sent -ne $false){throw 'Qt report nao passou gate de integridade'}
 Save 'AUTOMATED_PASS_MANUAL_PENDING' 'Ctrl+D 575 objetos, Undo/Redo, grupos, texto e bloqueio validados em laboratorio'
 Write-Host 'PASS CTRL+D 20m: duplicacao atomica de 575 objetos validada no nucleo e na GUI Qt.' -ForegroundColor Green
 Write-Host 'NENHUMA alteracao promovida ao instalador. Validacao real do mouse ainda pendente.' -ForegroundColor Yellow
 Write-Host "Resumo: $summary" -ForegroundColor Cyan
}catch{
 Save 'BLOCKED' $_.Exception.Message
 Write-Host ("FALHA CTRL+D 20m: "+$_.Exception.Message) -ForegroundColor Red
 Write-Host "Resumo: $summary" -ForegroundColor Yellow
 throw
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$oldMinimal,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
}
