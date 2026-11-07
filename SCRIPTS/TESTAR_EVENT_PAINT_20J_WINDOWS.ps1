#requires -Version 5.1
<#
Alencar Laser Fase 20j - coleta por categoria de evento.
Build Qt isolado + teste de regressao. -AbrirSessao opcional para mouse real.
NUNCA conecta ao laser, instala, salva projeto ou faz push.
#>
[CmdletBinding()]
param([switch]$AbrirSessao)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_EVENT_TRACE_20J'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$dir=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\EVENT_PAINT_TRACE_20J'
$run=Join-Path $dir ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Force -Path $run)
$oldPath=$env:PATH
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$oldTransform=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'test/phase20j-event-paint-trace-20261107'){throw "Branch incorreta: $branch"}
 foreach($needed in @($cmake,'C:\msys64\ucrt64\bin\ninja.exe')){
  if(-not (Test-Path -LiteralPath $needed -PathType Leaf)){throw "Ferramenta ausente: $needed"}
 }
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:ALENCAR_EVENT_PAINT_TRACE_20J='1'
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 $env:QT_QPA_PLATFORM='offscreen'
 Write-Warning 'Deixe o laser fisicamente desconectado.'
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao Qt falhou'}
 if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Binario Qt nao encontrado'}
 $smoke=Join-Path $run 'SMOKE'
 [void](New-Item -ItemType Directory -Force -Path $smoke)
 & $exe --grouped-center-transform-19-9-test '-' $smoke
 $exit=$LASTEXITCODE
 $result=Join-Path $smoke 'RESULTADO_TRANSFORMACAO_CENTRAL_19_9.json'
 if(-not (Test-Path -LiteralPath $result -PathType Leaf)){throw 'Qt nao gerou JSON do smoke test'}
 $r=Get-Content -LiteralPath $result -Raw -Encoding UTF8|ConvertFrom-Json
 if($exit -ne 0 -or $r.status -ne 'PASS' -or
    $r.undo_redo_preserved -ne $true -or
    $r.hardware_commands_sent -ne $false -or
    $r.event_paint_trace_20j.enabled -ne $true -or
    @($r.event_paint_trace_20j.groups).Count -lt 1){
  Write-Host ($r|ConvertTo-Json -Depth 8) -ForegroundColor Yellow
  throw 'Smoke test da instrumentacao reprovado'
 }
 Write-Host 'PASS: instrumentacao de pintura por evento + Undo/Redo.' -ForegroundColor Green
 Write-Host "Relatorio de teste: $result"
 if($AbrirSessao){
  # Restore Windows platform before opening the actual interactive view.
  [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
  $fixture=Join-Path $source 'fixtures\web_nodes_normalized.csv'
  if(-not (Test-Path -LiteralPath $fixture -PathType Leaf)){throw 'Fixture de trabalho ausente'}
  $session=Join-Path $run 'SESSAO_REAL'
  [void](New-Item -ItemType Directory -Force -Path $session)
  Write-Host 'Editor de laboratorio: teste selecionar, arrastar, zoom e pan. Exporte o JSON pelo menu de diagnosticos.' -ForegroundColor Cyan
  & $exe --interactive $fixture $session
  if($LASTEXITCODE -ne 0){throw "Sessao GUI fechou com erro: $LASTEXITCODE"}
  Write-Host 'O JSON exportado manualmente conterá performance.event_paint_trace_20j.' -ForegroundColor Yellow
 }
 Write-Host 'Fase20j: teste sintetico aprovado; desempenho real NAO homologado.' -ForegroundColor Yellow
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldTransform,'Process')
}
