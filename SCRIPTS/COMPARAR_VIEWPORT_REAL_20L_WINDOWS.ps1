#requires -Version 5.1
<#
ALENCAR LASER: Fase20l - comparacao manual de zoom/arraste OFF e ON.
Nao instala, nao modifica projetos, nao aciona laser.
Abra apenas com a maquina fisicamente desconectada.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$exe=Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_VIEWPORT_AB_20L\alencar_test7_qt6.exe'
$fixture=Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$sessionRoot=Join-Path $root ('RELATORIOS\AUDITORIAS\GITHUB_19_8\VIEWPORT_REAL_20L\'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
$oldPath=$env:PATH
$oldTrace=[Environment]::GetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J','Process')
$oldMinimal=[Environment]::GetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L','Process')
$oldWheel=[Environment]::GetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','Process')
$oldLocal=[Environment]::GetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','Process')
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'exp/phase20l-minimal-viewport-20261107'){throw "Branch diferente da esperada: $branch"}
 if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Editor de laboratorio nao compilado. Execute primeiro TESTAR_VIEWPORT_575_20L_WINDOWS.ps1.'}
 if(-not (Test-Path -LiteralPath $fixture -PathType Leaf)){throw 'Fixture ausente'}
 [void](New-Item -ItemType Directory -Force -Path $sessionRoot)
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
 $env:ALENCAR_EVENT_PAINT_TRACE_20J='1'
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 Write-Warning 'Deixe o laser FISICAMENTE desconectado, sem porta serial/Wi-Fi.'
 foreach($mode in @('OFF','ON')){
  $destination=Join-Path $sessionRoot $mode
  [void](New-Item -ItemType Directory -Force -Path $destination)
  if($mode -eq 'ON'){$env:ALENCAR_MINIMAL_VIEWPORT_20L='1'}
  else{[Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$null,'Process')}
  Write-Host ("ABRINDO EDITOR $mode : selecione tudo, mova, use zoom, pan e Tesoura (se seguro), depois exporte JSON e feche.") -ForegroundColor Cyan
  Write-Host ("Salve o diagnostico pelo painel como: "+(Join-Path $destination 'diagnostico.json')) -ForegroundColor Yellow
  & $exe --interactive $fixture $destination
  if($LASTEXITCODE -ne 0){throw ("Editor terminou com erro no modo {0}: {1}" -f $mode, $LASTEXITCODE)}
  $json=Join-Path $destination 'diagnostico.json'
  if(-not (Test-Path -LiteralPath $json -PathType Leaf)){
   throw "Diagnostico $mode ausente: $json. Exportacao e manual."
  }
  $r=Get-Content -LiteralPath $json -Raw -Encoding UTF8 | ConvertFrom-Json
  if($r.schema -ne 'alencar-laser-diagnostics-4' -or
     $r.environment.machine_connected -ne $false -or
     $r.performance.event_paint_trace_20j.enabled -ne $true -or
     [int]$r.performance.paint_samples -lt 50){
   throw "Diagnostico $mode invalido ou insuficiente"
  }
  Write-Host ("$mode : amostras="+$r.performance.paint_samples+" P95="+$r.performance.paint_p95_ms+" ms") -ForegroundColor Green
 }
 $off=Get-Content -LiteralPath (Join-Path $sessionRoot 'OFF\diagnostico.json') -Raw|ConvertFrom-Json
 $on=Get-Content -LiteralPath (Join-Path $sessionRoot 'ON\diagnostico.json') -Raw|ConvertFrom-Json
 $rows=@()
 foreach($category in @('zoom','transform','marquee','mouse_move','idle')){
  $a=@($off.performance.event_paint_trace_20j.groups|Where-Object {$_.event -eq $category})
  $b=@($on.performance.event_paint_trace_20j.groups|Where-Object {$_.event -eq $category})
  if($a.Count -eq 1 -and $b.Count -eq 1){
   $rows+=[pscustomobject]@{
    evento=$category
    amostras_off=[int]$a[0].samples
    amostras_on=[int]$b[0].samples
    p95_off_ms=[double]$a[0].p95_ms
    p95_on_ms=[double]$b[0].p95_ms
   }
  }
 }
 $rows|Format-Table -AutoSize
 $report=[ordered]@{
  schema='alencar-laser-viewport-real-ab-20l-1'
  status='MANUAL_MEASURED_NOT_HOMOLOGATED'
  project_data_exported_by_script=$false
  machine_commands_sent_by_script=$false
  production_homologated=$false
  source_reports_local=$true
  rows=$rows
 }
 [IO.File]::WriteAllText((Join-Path $sessionRoot 'COMPARACAO_RESUMO.json'),
  ($report|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
 Write-Host ("Comparacao concluida. Pasta: "+$sessionRoot) -ForegroundColor Cyan
 Write-Host 'Nenhuma alteracao automatica foi promovida. Confira visualmente a tela dos dois modos.' -ForegroundColor Yellow
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$oldTrace,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$oldMinimal,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$oldWheel,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$oldLocal,'Process')
}
