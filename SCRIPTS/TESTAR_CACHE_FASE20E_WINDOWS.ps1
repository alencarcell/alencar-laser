#requires -Version 5.1
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$src=Join-Path $root 'DESENVOLVIMENTO\FONTE\experiments\phase20d'
$build=Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_PHASE20D_CACHE'
$dest=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\PHASE20E_CACHE'
$oldPath=$env:PATH
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
try {
 $bin='C:\msys64\ucrt64\bin'
 if(-not (Test-Path (Join-Path $bin 'cmake.exe'))) {throw 'CMake UCRT64 ausente'}
 if(-not (Test-Path (Join-Path $bin 'ninja.exe'))) {throw 'Ninja UCRT64 ausente'}
 $env:PATH=$bin+';'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 & (Join-Path $bin 'cmake.exe') -S $src -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
 if($LASTEXITCODE -ne 0){throw 'Falha ao configurar Fase20e'}
 & (Join-Path $bin 'cmake.exe') --build $build --target alencar_phase20e_dynamic -j 2
 if($LASTEXITCODE -ne 0){throw 'Falha na compilacao Fase20e'}
 $exe=Join-Path $build 'alencar_phase20e_dynamic.exe'
 if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Executavel ausente'}
 [void](New-Item -ItemType Directory -Path $dest -Force)
 $out=Join-Path $dest ('CACHE_DINAMICO_'+(Get-Date -Format 'yyyyMMdd_HHmmss')+'.json')
 & $exe --out $out
 if($LASTEXITCODE -ne 0){throw 'Execucao Fase20e falhou'}
 $report=Get-Content -LiteralPath $out -Raw -Encoding UTF8 | ConvertFrom-Json
 if($report.schema -ne 'alencar-laser-phase20e-dynamic-cache-v1' -or
    $report.status -ne 'SYNTHETIC_MEASURED_MANUAL_PENDING' -or
    $report.no_production_change -ne $true -or
    $report.hardware_commands_sent -ne $false -or
    $report.speedup_claim_allowed -ne $false -or
    $report.automatic_promotion_allowed -ne $false -or
    @($report.cases).Count -ne 18){throw 'Gate de integridade invalido'}
 foreach($n in @(40,128,575)){
  $group=@($report.cases | Where-Object { $_.contours -eq $n })
  if($group.Count -ne 6){throw "Rodadas incompletas: $n"}
  $off=@($group | ForEach-Object {[double]$_.baseline_median_frame_ms}|Sort-Object)
  $on=@($group | ForEach-Object {[double]$_.cached_median_frame_ms}|Sort-Object)
  $offMed=($off[2]+$off[3])/2
  $onMed=($on[2]+$on[3])/2
  $maxDelta=($group | Measure-Object -Property max_channel_difference -Maximum).Maximum
  $changes=($group | Measure-Object -Property changed_pixels_all_steps -Sum).Sum
  $peak=($group | Measure-Object -Property peak_process_working_set_bytes -Maximum).Maximum
  Write-Host ("{0} contornos: medianas OFF={1:N3}ms CACHE={2:N3}ms; pixels alterados={3}; max canal={4}; RSS pico={5:N0} bytes" -f $n,$offMed,$onMed,$changes,$maxDelta,$peak)
 }
 Write-Host 'FASE20e: MEDIDO, NAO HOMOLOGADO. Pixel divergente exige revisao; nenhuma promocao automatica.' -ForegroundColor Yellow
 Write-Host "Relatorio local: $out"
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
}
