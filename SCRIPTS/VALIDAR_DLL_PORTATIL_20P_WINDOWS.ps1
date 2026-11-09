#requires -Version 5.1
<#
Alencar Laser 20p - inspect native DLL imports, resolve MSYS2 UCRT64 dependencies
recursively, then run an isolated Qt smoke test WITHOUT the build toolchain PATH.
A failed test blocks creation of a distributable ZIP.
#>
[CmdletBinding()]
param(
 [Parameter(Mandatory=$true)][string]$Pasta,
 [Parameter(Mandatory=$true)][string]$Executavel
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$stage=[IO.Path]::GetFullPath($Pasta)
$exe=[IO.Path]::GetFullPath($Executavel)
$qtBin='C:\msys64\ucrt64\bin'
$objdumpCandidates=@((Join-Path $qtBin 'objdump.exe'),'C:\msys64\usr\bin\objdump.exe')
$objdump=@($objdumpCandidates|Where-Object {Test-Path -LiteralPath $_ -PathType Leaf}|Select-Object -First 1)
if($objdump.Count -ne 1){throw 'objdump nao encontrado em MSYS2. Instale binutils UCRT64 antes de empacotar.'}
if(-not (Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Executavel nao encontrado'}
if(-not $exe.StartsWith($stage+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Executavel fora da pasta do pacote'}
$system32=Join-Path $env:SystemRoot 'System32'
$systemWow=Join-Path $env:SystemRoot 'SysWOW64'
$copied=New-Object System.Collections.ArrayList
$scanned=New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
function ImportedDlls([string]$file){
 $lines=@(& $objdump[0] '-p' $file 2>&1)
 if($LASTEXITCODE -ne 0){throw "objdump falhou em $file"}
 $names=New-Object System.Collections.ArrayList
 foreach($line in $lines){
  if(([string]$line) -match '^\s*DLL Name:\s*([^\s]+)'){
   [void]$names.Add($Matches[1])
  }
 }
 return @($names|Select-Object -Unique)
}
# DLLs installed in Qt plugin subfolders must be scanned too, because
# their imported modules may differ from the executable's imports.
$queue=New-Object 'System.Collections.Generic.Queue[string]'
foreach($f in @(Get-ChildItem -LiteralPath $stage -Recurse -File | Where-Object {
 $_.Extension -ieq '.dll' -or $_.Extension -ieq '.exe'
})){$queue.Enqueue($f.FullName)}
$counter=0
while($queue.Count -gt 0){
 $file=$queue.Dequeue()
 if(-not $scanned.Add($file)){continue}
 $counter++
 if($counter -gt 400){throw 'Limite de DLLs analisadas excedido; interrompendo para revisao'}
 foreach($name in @(ImportedDlls $file)){
  $sameDir=Join-Path (Split-Path -Parent $file) $name
  $appDir=Join-Path $stage $name
  $systemDll=Join-Path $system32 $name
  $wowDll=Join-Path $systemWow $name
  if((Test-Path -LiteralPath $sameDir -PathType Leaf) -or
     (Test-Path -LiteralPath $appDir -PathType Leaf) -or
     (Test-Path -LiteralPath $systemDll -PathType Leaf) -or
     (Test-Path -LiteralPath $wowDll -PathType Leaf) -or
     $name -match '^(api-ms-win-|ext-ms-)'){continue}
  $origin=Join-Path $qtBin $name
  if(-not (Test-Path -LiteralPath $origin -PathType Leaf)){
   throw ("Dependencia nativa nao encontrada para {0}: {1}" -f $file,$name)
  }
  # Copy only runtime DLLs from the same Qt/UCRT installation, never
  # from unknown third-party directories.
  Copy-Item -LiteralPath $origin -Destination $appDir -ErrorAction Stop
  [void]$copied.Add($name)
  $queue.Enqueue($appDir)
 }
}
# Some platform plugins load Qt libraries via dynamic LoadLibrary and do
# not show in the static import table. Include offscreen plugin for smoke.
$offscreen=Join-Path $stage 'platforms\qoffscreen.dll'
$qtOffscreen='C:\msys64\ucrt64\share\qt6\plugins\platforms\qoffscreen.dll'
$altOffscreen='C:\msys64\ucrt64\lib\qt6\plugins\platforms\qoffscreen.dll'
foreach($candidate in @($qtOffscreen,$altOffscreen)){
 if(-not (Test-Path -LiteralPath $offscreen -PathType Leaf) -and
    (Test-Path -LiteralPath $candidate -PathType Leaf)){
  Copy-Item -LiteralPath $candidate -Destination $offscreen
 }
}
$reports=Join-Path $stage '_SMOKE_20P'
[void](New-Item -ItemType Directory -Path $reports -Force)
$oldPath=$env:PATH
$names=@('QT_QPA_PLATFORM','QT_QPA_PLATFORM_PLUGIN_PATH','ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','ALENCAR_EVENT_PAINT_TRACE_20J')
$old=@{}
foreach($key in $names){$old[$key]=[Environment]::GetEnvironmentVariable($key,'Process')}
$exitCode=-999
try{
 # Deliberately exclude C:\msys64 from PATH to expose missing dependencies
 # that were previously masked by the developer environment.
 $env:PATH=$stage+';'+$system32+';'+$env:SystemRoot
 $env:QT_QPA_PLATFORM_PLUGIN_PATH=Join-Path $stage 'platforms'
 $env:QT_QPA_PLATFORM=$(if(Test-Path -LiteralPath $offscreen -PathType Leaf){'offscreen'}else{'windows'})
 $env:ALENCAR_MINIMAL_VIEWPORT_20L='1'
 foreach($key in @('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','ALENCAR_EVENT_PAINT_TRACE_20J')){
  [Environment]::SetEnvironmentVariable($key,$null,'Process')
 }
 Write-Host 'SMOKE 20p: iniciando executavel SEM C:\msys64 no PATH.' -ForegroundColor Cyan
 & $exe '--grouped-center-transform-19-9-test' '-' $reports
 $exitCode=$LASTEXITCODE
}finally{
 $env:PATH=$oldPath
 foreach($key in $names){[Environment]::SetEnvironmentVariable($key,$old[$key],'Process')}
}
$testResult=Join-Path $reports 'RESULTADO_TRANSFORMACAO_CENTRAL_19_9.json'
if($exitCode -ne 0){
 $hex=('{0:X8}' -f ([uint32]([int64]$exitCode -band 0xffffffffL)))
 throw ("Smoke test falhou. Exit {0}, NTSTATUS 0x{1}. Falta DLL/plugin ou outro erro de inicializacao." -f $exitCode,$hex)
}
if(-not (Test-Path -LiteralPath $testResult -PathType Leaf)){
 throw 'Smoke test nao gerou o relatorio Qt esperado'
}
$r=Get-Content -LiteralPath $testResult -Raw -Encoding UTF8|ConvertFrom-Json
if($r.status -ne 'PASS' -or $r.undo_redo_preserved -ne $true -or
   $r.hardware_commands_sent -ne $false){
 throw 'Smoke test Qt falhou ou integridade Undo/Redo nao confirmada'
}
# Remove smoke artifacts from distribution; preserve a small validation
# record outside the package in the parent directory.
$record=[ordered]@{
 schema='alencar-laser-portable-smoke-20p-1'
 status='PASS'
 executable_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
 qt_test='grouped-center-transform-19-9'
 no_msys2_path=$true
 dlls_examined=$counter
 copied_native_dlls=@($copied)
 hardware_commands_sent=$false
}
$recordFile=Join-Path (Split-Path -Parent $stage) ((Split-Path -Leaf $stage)+'_SMOKE_RESULTADO.json')
[IO.File]::WriteAllText($recordFile,($record|ConvertTo-Json -Depth 6),[Text.UTF8Encoding]::new($false))
Remove-Item -LiteralPath $reports -Recurse -Force
Write-Host ("PASS: Qt portatil iniciou sem MSYS2 no PATH; dependencias examinadas: "+$counter) -ForegroundColor Green
Write-Host ("Resultado: "+$recordFile) -ForegroundColor Cyan
