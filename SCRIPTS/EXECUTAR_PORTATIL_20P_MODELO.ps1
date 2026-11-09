#requires -Version 5.1
<#
Portable 20p verified launcher, runs only the bundled test fixture.
Keep laser physically disconnected. No production installation.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$base=(Resolve-Path -LiteralPath $PSScriptRoot).Path
$manifestFile=Join-Path $base 'MANIFESTO_SHA256_20P.json'
if(-not (Test-Path -LiteralPath $manifestFile -PathType Leaf)){throw 'Manifesto ausente; pacote incompleto'}
$manifest=Get-Content -LiteralPath $manifestFile -Encoding UTF8 -Raw|ConvertFrom-Json
if($manifest.schema -ne 'alencar-laser-portable-homologation-20p-1' -or
   $manifest.status -ne 'PORTABLE_CREATED_MANUAL_PENDING' -or
   $manifest.offline_fixture_only -ne $true -or
   $manifest.laser_hardware_homologated -ne $false){
 throw 'Manifesto invalido ou nao autorizado para execucao de laboratorio'
}
$files=@($manifest.files)
if($files.Count -lt 4){throw 'Pacote sem dependencias suficientes'}
foreach($entry in $files){
 $relative=[string]$entry.path
 if([IO.Path]::IsPathRooted($relative) -or $relative.Contains('..') -or $relative.StartsWith('/')){
  throw ('Entrada insegura no manifesto: '+$relative)
 }
 $full=[IO.Path]::GetFullPath((Join-Path $base $relative.Replace('/','\')))
 if(-not $full.StartsWith($base+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Manifesto aponta para fora da pasta'}
 if(-not (Test-Path -LiteralPath $full -PathType Leaf)){throw ('Arquivo faltando: '+$relative)}
 if((Get-FileHash -LiteralPath $full -Algorithm SHA256).Hash -ne [string]$entry.sha256){
  throw ('Arquivo adulterado ou corrompido: '+$relative)
 }
}
$exe=Join-Path $base 'Alencar_Laser_20P_Teste.exe'
$fixture=Join-Path $base 'fixtures\web_nodes_normalized.csv'
$platform=Join-Path $base 'platforms\qwindows.dll'
foreach($p in @($exe,$fixture,$platform)){
 if(-not (Test-Path -LiteralPath $p -PathType Leaf)){throw ('Dependencia ausente: '+$p)}
}
Write-Warning 'USO DE HOMOLOGACAO: desconecte fisicamente o laser. Nao enviar comandos, nao abrir projetos originais.'
$oldPath=$env:PATH
$envVars=@('QT_QPA_PLATFORM','QT_QPA_PLATFORM_PLUGIN_PATH','ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','ALENCAR_EVENT_PAINT_TRACE_20J')
$old=@{}
foreach($name in $envVars){$old[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
try{
 $env:PATH=$base+';'+$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
 $env:QT_QPA_PLATFORM_PLUGIN_PATH=Join-Path $base 'platforms'
 $env:ALENCAR_MINIMAL_VIEWPORT_20L='1'
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_EVENT_PAINT_TRACE_20J',$null,'Process')
 $data=Join-Path $env:LOCALAPPDATA 'AlencarLaser\Homologacao20P'
 $session=Join-Path $data ('Sessao_'+(Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
 [void](New-Item -ItemType Directory -Path $session -Force)
 Write-Host ('Sessao local de teste: '+$session) -ForegroundColor Cyan
 & $exe --interactive $fixture $session
 $code=$LASTEXITCODE
 if($code -ne 0){throw ('Editor de homologacao encerrou com codigo '+$code)}
 Write-Host 'Sessao encerrada. Nenhuma instalacao realizada.' -ForegroundColor Green
}finally{
 $env:PATH=$oldPath
 foreach($name in $envVars){
  [Environment]::SetEnvironmentVariable($name,$old[$name],'Process')
 }
}
