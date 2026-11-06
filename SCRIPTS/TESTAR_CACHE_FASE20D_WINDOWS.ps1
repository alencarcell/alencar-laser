#requires -Version 5.1
<#
Phase20d: isolated Qt cache A/B benchmark, NOT the Alencar Laser application.
No project I/O, .alaser files, Git push or laser commands.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$src=Join-Path $root 'DESENVOLVIMENTO\FONTE\experiments\phase20d'
$build=Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_PHASE20D_CACHE'
$dest=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\PHASE20D_CACHE'
$oldPath=$env:PATH
$oldPlatform=[Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM','Process')
try {
 if(-not (Test-Path 'C:\msys64\ucrt64\bin\cmake.exe')) {
  throw 'CMake UCRT64 ausente: esperado C:\msys64\ucrt64\bin\cmake.exe'
 }
 if(-not (Test-Path 'C:\msys64\ucrt64\bin\ninja.exe')) {
  throw 'Ninja UCRT64 ausente.'
 }
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 & 'C:\msys64\ucrt64\bin\cmake.exe' -S $src -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
 if($LASTEXITCODE -ne 0){throw 'Falha de configuracao Qt Fase20d.'}
 & 'C:\msys64\ucrt64\bin\cmake.exe' --build $build --target alencar_phase20d_cache -j 2
 if($LASTEXITCODE -ne 0){throw 'Falha de compilacao Fase20d.'}
 $exe=Join-Path $build 'alencar_phase20d_cache.exe'
 if(-not (Test-Path $exe -PathType Leaf)){throw 'Executavel de laboratorio ausente.'}
 [void](New-Item -ItemType Directory -Force -Path $dest)
 $name='CACHE_AB_'+(Get-Date -Format 'yyyyMMdd_HHmmss')+'.json'
 $output=Join-Path $dest $name
 & $exe --out $output
 if($LASTEXITCODE -ne 0){throw "Falha na execucao Fase20d, codigo $LASTEXITCODE"}
 $json=Get-Content -LiteralPath $output -Raw -Encoding UTF8 | ConvertFrom-Json
 if($json.status -ne 'MEASURED_VISUAL_REVIEW_REQUIRED' -or
    $json.cache_in_production -ne $false -or
    $json.hardware_commands_sent -ne $false -or
    $json.automatic_promotion_allowed -ne $false -or
    @($json.cases).Count -ne 9) {
   throw 'Gate de integridade Fase20d falhou.'
 }
 foreach($row in $json.cases) {
  if([double]$row.baseline_median_ms -le 0 -or
     [double]$row.cached_median_ms -le 0 -or
     [double]$row.changed_pixels -lt 0) {
    throw 'Metrica Fase20d invalida.'
  }
 }
 Write-Host 'FASE20D: experimento isolado medido; NAO homologado.' -ForegroundColor Yellow
 Write-Host "Relatorio local: $output"
 Write-Host 'Compare os pixels divergentes e medianas; nao promover cache automaticamente.'
}finally{
 $env:PATH=$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$oldPlatform,'Process')
}
