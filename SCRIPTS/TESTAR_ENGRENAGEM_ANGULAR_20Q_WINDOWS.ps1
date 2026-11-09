#requires -Version 5.1
<#
Alencar Laser: Gerador de engrenagens, perfis ANGULAR e SERRILHADO 20q.
Build isolado, teste de nucleo + regressao antiga + Qt GUI.
Nao instala, nao modifica APLICATIVO, nao envia comandos ao laser.
#>
[CmdletBinding()]
param([switch]$Abrir)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_GEAR_20Q'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$core=Join-Path $build 'alencar_test_gear_angular_20q.exe'
$legacy=Join-Path $build 'alencar_test19_9_gear_offline.exe'
$dir=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\GEAR_20Q'
$session=Join-Path $dir ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Path $session -Force)
$summary=Join-Path $session 'RESULTADO_GEAR_20Q.json'
$steps=@()
$revision='unknown'
$oldPath=$env:PATH
$keys=@('QT_QPA_PLATFORM','ALENCAR_MINIMAL_VIEWPORT_20L',
        'ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H')
$old=@{}
foreach($key in $keys){$old[$key]=[Environment]::GetEnvironmentVariable($key,'Process')}
function Save([string]$status,[string]$message){
 $data=[ordered]@{
  schema='alencar-laser-gear20q-1'
  status=$status;message=$message
  branch='feat/20q-angular-gear-20261109'
  source_revision=$script:revision
  executable_path=$exe
  executable_sha256=$(if(Test-Path -LiteralPath $exe -PathType Leaf){(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash}else{$null})
  gates=@($steps)
  production_homologated=$false
  physical_laser_tested=$false
  hardware_commands_sent=$false
  original_installed_version_modified=$false
 }
 [IO.File]::WriteAllText($summary,($data|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
}
function Run([string]$name,[string]$binary,[string[]]$params,[string]$json=''){
 Write-Host ("INICIANDO "+$name) -ForegroundColor Cyan
 & $binary @params
 $exit=$LASTEXITCODE
 if($exit -ne 0){
  $errorInfo=''
  if($json -and (Test-Path -LiteralPath $json -PathType Leaf)){
   $errorInfo=(Get-Content -LiteralPath $json -Raw -Encoding UTF8)
  }
  throw ("Teste {0} falhou com exit {1}. {2}" -f $name,$exit,$errorInfo)
 }
 if($json){
  if(-not (Test-Path -LiteralPath $json -PathType Leaf)){throw "JSON de teste ausente: $json"}
  $r=Get-Content -LiteralPath $json -Raw -Encoding UTF8|ConvertFrom-Json
  if($r.status -ne 'PASS' -or $r.hardware_commands_sent -ne $false){
   throw ("Relatorio invalido: "+$json)
  }
 }
 $script:steps+= [pscustomobject]@{name=$name;result='PASS';json=$json}
 Save 'IN_PROGRESS' ("Gate "+$name+" concluido")
 Write-Host ("PASS "+$name) -ForegroundColor Green
}
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'feat/20q-angular-gear-20261109'){
  throw ("Branch incorreta: "+$branch+"; nao use force/reset/clean.")
 }
 $revision=(& git -C $root rev-parse HEAD)
 if($LASTEXITCODE -ne 0){throw 'Git HEAD indisponivel'}
 if(-not (Test-Path -LiteralPath $cmake -PathType Leaf)){throw 'CMake UCRT64 ausente'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 foreach($key in @('ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H')){
  [Environment]::SetEnvironmentVariable($key,$null,'Process')
 }
 Write-Warning 'Laser deve ficar fisicamente DESCONECTADO. Geracao apenas vetorial/offline.'
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test_gear_angular_20q alencar_test19_9_gear_offline alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao do motor Qt ou de testes falhou'}
 foreach($file in @($exe,$core,$legacy)){
  if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw ("Binario ausente: "+$file)}
 }
 Run 'G01_CORE_ANGULAR' $core @()
 Run 'G02_INVOLUTA_LEGADA' $legacy @()
 $newDir=Join-Path $session 'G03_GUI_ANGULAR'
 [void](New-Item -ItemType Directory -Path $newDir -Force)
 Run 'G03_GUI_ANGULAR' $exe @('--gear-angular-20q-test','-',$newDir) (Join-Path $newDir 'RESULTADO_ENGRENAGEM_ANGULAR_20Q.json')
 $oldDir=Join-Path $session 'G04_GUI_ENGRENAGEM_LEGADA'
 [void](New-Item -ItemType Directory -Path $oldDir -Force)
 Run 'G04_GUI_ENGRENAGEM_LEGADA' $exe @('--gear-generator-19-9-test','-',$oldDir) (Join-Path $oldDir 'RESULTADO_ENGRENAGENS_19_9.json')
 Save 'AUTOMATED_PASS_MANUAL_PENDING' '4 testes aprovados, perfis angulares e involuta, criacao GUI Undo/Redo.'
 Write-Host 'PASS 20q: 4 gates aprovados; pronto para teste visual manual em candidata isolada.' -ForegroundColor Green
 Write-Host ("Relatorio: "+$summary) -ForegroundColor Cyan
 if($Abrir){
  $launcher=Join-Path $PSScriptRoot 'ABRIR_GERADOR_ANGULAR_20Q_WINDOWS.ps1'
  if(-not (Test-Path -LiteralPath $launcher -PathType Leaf)){throw 'Launcher manual 20q ausente'}
  & powershell.exe '-NoProfile' '-ExecutionPolicy' 'Bypass' '-File' $launcher
  if($LASTEXITCODE -ne 0){throw 'Validacao visual manual retornou erro'}
 }
}catch{
 Save 'BLOCKED' $_.Exception.Message
 Write-Host ("FALHA 20q: "+$_.Exception.Message) -ForegroundColor Red
 Write-Host ("Relatorio: "+$summary) -ForegroundColor Yellow
 throw
}finally{
 $env:PATH=$oldPath
 foreach($key in $keys){[Environment]::SetEnvironmentVariable($key,$old[$key],'Process')}
}
