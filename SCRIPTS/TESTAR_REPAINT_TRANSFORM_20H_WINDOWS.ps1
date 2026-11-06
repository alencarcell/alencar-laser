#requires -Version 5.1
<#
Alencar Laser 20h - regression of localized transform overlay repaint.
Compiles QT executable into isolated A/B directory, runs OFF and opt-in ON.
Does not enable experiment in installed application or send hardware commands.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$src=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $src 'build_WINDOWS_DRAG_AB_19_9'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$results=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\TRANSFORM_REPAINT_20H'
$run=Join-Path $results ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$oldPath=$env:PATH
$oldFlag=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldQt=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
$summary=Join-Path $run 'RESULTADO_TRANSFORM_REPAINT_20H.json'
[void](New-Item -ItemType Directory -Force -Path $run)
$off=$null;$on=$null
try {
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'fix/phase20h-transform-repaint-20261106'){
   throw "Branch inesperada: $branch"
 }
 if(-not (Test-Path $cmake -PathType Leaf)){throw 'CMake MSYS2 ausente'}
 if(-not (Test-Path 'C:\msys64\ucrt64\bin\libminizip-1.dll')){throw 'Qt runtime incompleto'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 # A software-rendered Qt viewport reduces differences between launches;
 # real monitor/DPI manual checks still remain mandatory.
 $env:QT_QPA_PLATFORM='offscreen'
 & $cmake -S $src -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'CMake configure falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao Qt falhou; capture o primeiro erro C++'}
 if(-not (Test-Path $exe -PathType Leaf)){throw 'Executavel Qt isolado ausente'}
 function Run([string]$label,[bool]$enabled){
  $dir=Join-Path $run $label
  [void](New-Item -ItemType Directory -Force -Path $dir)
  if($enabled){$env:ALENCAR_TRANSFORM_LOCAL_REPAINT_20H='1'}
  else{[Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')}
  & $exe --grouped-center-transform-19-9-test '-' $dir
  if($LASTEXITCODE -ne 0){throw "Teste Qt $label falhou (exit $LASTEXITCODE)"}
  $p=Join-Path $dir 'RESULTADO_TRANSFORMACAO_CENTRAL_19_9.json'
  if(-not (Test-Path $p -PathType Leaf)){throw "Resultado ausente: $label"}
  $o=Get-Content -LiteralPath $p -Raw -Encoding UTF8|ConvertFrom-Json
  if($o.status -ne 'PASS' -or $o.undo_redo_preserved -ne $true -or
     $o.document_unchanged_during_preview -ne $true -or
     $o.hardware_commands_sent -ne $false -or
     $o.transform_local_repaint_optin -ne $enabled -or
     [string]::IsNullOrWhiteSpace([string]$o.preview_sha256) -or
     [int]$o.selected_objects -ne 40){throw "Regressao de integridade: $label"}
  return $o
 }
 $off=Run 'OFF' $false
 $on=Run 'ON' $true
 if($off.preview_sha256 -ne $on.preview_sha256){throw 'REGRESSAO VISUAL: pixels OFF e ON diferentes'}
 if([double]$off.transform_local_update_requests -ne 0 -or
    [double]$on.transform_local_update_requests -lt 1 -or
    [double]$on.transform_full_fallbacks -ne 0){
    throw 'Política de repintura experimental nao foi ativada corretamente'
 }
 $result=[ordered]@{
  schema='alencar-laser-phase20h-transform-repaint-1'
  status='PASS_SYNTHETIC_MANUAL_PENDING'
  off_preview_sha256=$off.preview_sha256
  on_preview_sha256=$on.preview_sha256
  equal_preview_pixels=$true
  geometry_and_undo_redo_preserved=$true
  local_requests_on=[double]$on.transform_local_update_requests
  full_fallbacks_on=[double]$on.transform_full_fallbacks
  machine_commands_sent=$false
  installed_application_modified=$false
  production_homologated=$false
  speedup_claim_allowed=$false
 }
 [IO.File]::WriteAllText($summary,($result|ConvertTo-Json -Depth 7),[Text.UTF8Encoding]::new($false))
 Write-Host 'PASS 20h: pixels, geometria e Undo/Redo equivalentes no teste Qt sintetico.' -ForegroundColor Green
 Write-Host 'Sem ganho de desempenho comprovado; teste com mouse fisico e 575 objetos pendente.' -ForegroundColor Yellow
 Write-Host "Resumo: $summary" -ForegroundColor Cyan
}catch{
 $r=[ordered]@{schema='alencar-laser-phase20h-transform-repaint-1';status='BLOCKED';error=$_.Exception.Message;production_homologated=$false}
 [IO.File]::WriteAllText($summary,($r|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
 Write-Host "FASE20h FALHOU: $($_.Exception.Message)" -ForegroundColor Red
 Write-Host "Resumo: $summary" -ForegroundColor Yellow
 throw
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldQt,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldFlag,'Process')
}
