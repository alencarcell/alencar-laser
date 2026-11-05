#requires -Version 5.1
<#
ALENCAR LASER 19.9 - A/B offline da repintura local no arraste agrupado.
Configura/compila apenas o Qt de desenvolvimento. Mesmo teste sintetico
de 40 objetos com flag OFF e ON; sem portas, Git push ou projetos reais.
#>
[CmdletBinding()]
param([switch]$SemCompilar)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$source = Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build = Join-Path $source 'build_WINDOWS_GIT_19_8'
$exe = Join-Path $build 'alencar_test7_qt6.exe'
$cmake = 'C:\msys64\ucrt64\bin\cmake.exe'
$oldPath = $env:PATH
$oldFlag = [Environment]::GetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9','Process')
$dir = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\DRAG_OVERLAY_19_9'
$off = Join-Path $dir 'OFF'
$on = Join-Path $dir 'ON'
[void](New-Item -ItemType Directory -Path $off,$on -Force)
try {
 if (-not (Test-Path -LiteralPath 'C:\msys64\ucrt64\bin\libminizip-1.dll')) {
  throw 'Runtime UCRT64 incompleto: libminizip-1.dll ausente.'
 }
 $env:PATH = 'C:\msys64\ucrt64\bin;' + $oldPath
 if (-not $SemCompilar) {
  if (-not (Test-Path -LiteralPath $cmake)) { throw 'CMake UCRT64 ausente.' }
  $configureArgs = @('-S',$source,'-B',$build,'-G','Ninja',
    '-DCMAKE_BUILD_TYPE=Release','-DALENCAR_REQUIRE_QT=ON',
    '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64',
    '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe')
  & $cmake @configureArgs
  if ($LASTEXITCODE -ne 0) { throw 'Configuracao Qt falhou.' }
  & $cmake --build $build --parallel 1 --target alencar_test7_qt6
  if ($LASTEXITCODE -ne 0) { throw 'Compilacao Qt falhou; veja os erros acima.' }
 }
 if (-not (Test-Path -LiteralPath $exe)) {
  throw 'Executavel experimental ausente. Execute sem -SemCompilar.'
 }
 function Testar([string]$dest,[bool]$usarFlag) {
  if ($usarFlag) {
   [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9','1','Process')
  } else {
   [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$null,'Process')
  }
  $saida = Join-Path $dest 'RESULTADO_ARRASTE_AGRUPADO_19_9.json'
  if (Test-Path -LiteralPath $saida) { Remove-Item -LiteralPath $saida -Force }
  & $exe --grouped-drag-19-9-test '-' $dest
  $exitCode = $LASTEXITCODE
  if (-not (Test-Path -LiteralPath $saida)) { throw "JSON ausente em $dest" }
  $result = Get-Content -LiteralPath $saida -Raw | ConvertFrom-Json
  if ($exitCode -ne 0 -or $result.status -ne 'PASS') {
   throw "Teste arraste Qt falhou: $dest, codigo $exitCode, resultado $($result | ConvertTo-Json -Compress)"
  }
  if ($result.selected_objects -ne 40 -or
      $result.grouped_starts -ne 1 -or
      $result.grouped_frames -lt 1 -or
      $result.undo_redo_preserved -ne $true -or
      $result.hardware_commands_sent -ne $false -or
      $result.drag_overlay_local_experiment -ne $usarFlag) {
   throw "Verificacoes de integridade do arraste falharam: $dest"
  }
  if ($usarFlag) {
   if ($result.drag_overlay_local_requests -lt 1 -or
       $result.drag_overlay_full_fallbacks -ne 0) {
    throw 'Modo local nao comprovado no teste.'
   }
  } elseif ($result.drag_overlay_local_requests -ne 0) {
   throw 'Modo de referencia alterou a politica de repintura.'
  }
  return $result
 }
 $baseline=Testar $off $false
 $optin=Testar $on $true
 $report=[ordered]@{
  schema='alencar-laser-drag-overlay-local-ab-19.9'
  status='PASS_SYNTHETIC_GUI_MANUAL_PENDING'
  baseline=[ordered]@{
   selected_objects=[int]$baseline.selected_objects
   local_requests=[int]$baseline.drag_overlay_local_requests
  }
  optin=[ordered]@{
   selected_objects=[int]$optin.selected_objects
   local_requests=[int]$optin.drag_overlay_local_requests
   full_fallbacks=[int]$optin.drag_overlay_full_fallbacks
  }
  geometry_and_undo_redo_preserved=$true
  machine_commands_sent=$false
  production_homologated=$false
 }
 $out=Join-Path $dir 'COMPARACAO_DRAG_OVERLAY_19_9.json'
 [IO.File]::WriteAllText($out,(($report | ConvertTo-Json -Depth 5)+[Environment]::NewLine),
     [Text.UTF8Encoding]::new($false))
 Write-Host 'PASS: Qt baseline e opt-in, 40 objetos, geometria, Undo/Redo, sem hardware.' -ForegroundColor Green
 Write-Host 'PENDENTE: zoom/pan real, rastro da moldura, P95 do mouse e CPU.' -ForegroundColor Yellow
 Write-Host "Relatorio local: $out"
}
finally {
 [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$oldFlag,'Process')
 $env:PATH = $oldPath
}
