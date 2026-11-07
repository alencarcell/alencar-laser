#requires -Version 5.1
<#
Alencar Laser OFFLINE Candidate 20n -- Windows integrated release gate.
Builds and tests an isolated development executable; NEVER installs,
changes main/APLICATIVO, writes to user projects, or contacts a laser.
A passing gate is NOT authorization for physical laser use.
#>
[CmdletBinding()]
param([switch]$Abrir)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$source=Join-Path $root 'DESENVOLVIMENTO\FONTE'
$build=Join-Path $source 'build_WINDOWS_CANDIDATE_20N'
$exe=Join-Path $build 'alencar_test7_qt6.exe'
$core=Join-Path $build 'alencar_test_bulk_duplicate_20m.exe'
$fixture=Join-Path $source 'fixtures\web_nodes_normalized.csv'
$cmake='C:\msys64\ucrt64\bin\cmake.exe'
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\CANDIDATE_20N'
$session=Join-Path $reports ((Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8))
[void](New-Item -ItemType Directory -Path $session -Force)
$reportPath=Join-Path $session 'RESULTADO_CANDIDATE_20N.json'
$steps=New-Object System.Collections.ArrayList
$oldPath=$env:PATH
$envNames=@('QT_QPA_PLATFORM','ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','ALENCAR_EVENT_PAINT_TRACE_20J')
$previous=@{}
foreach($name in $envNames){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$revision='unknown'
function Save([string]$status,[string]$message) {
 $hash=$null
 if(Test-Path -LiteralPath $exe -PathType Leaf){$hash=(Get-FileHash -Algorithm SHA256 -LiteralPath $exe).Hash}
 $r=[ordered]@{
  schema='alencar-laser-candidate-20n-1'
  status=$status;message=$message
  source_branch='candidate/offline-20n-20261107'
  source_revision=$script:revision
  executable_path=$exe
  executable_sha256=$hash
  viewport_mode='minimal'
  ctrl_d_bulk_fix=$true
  run_directory=$session
  steps=@($steps.ToArray())
  automatic_tests_complete=($status -eq 'AUTOMATED_PASS_MANUAL_PENDING')
  manual_homologation_complete=$false
  installed_application_modified=$false
  hardware_commands_sent_by_runner=$false
  production_homologated=$false
 }
 [IO.File]::WriteAllText($reportPath,($r|ConvertTo-Json -Depth 9),
   [Text.UTF8Encoding]::new($false))
}
function Gate([string]$id,[string]$description,[string]$program,[string[]]$arguments) {
 $started=Get-Date
 $dir=Join-Path $session $id
 [void](New-Item -ItemType Directory -Path $dir -Force)
 Write-Host ("INICIANDO: {0} - {1}" -f $id,$description) -ForegroundColor Cyan
 $code=-999;$errorMessage=''
 try {
  & $program @arguments
  $code=$LASTEXITCODE
 }catch{
  $errorMessage=$_.Exception.Message
 }
 $entry=[ordered]@{id=$id;description=$description;exit_code=$code;
  duration_seconds=[math]::Round(((Get-Date)-$started).TotalSeconds,1);
  status=$(if($code -eq 0 -and !$errorMessage){'PASS'}else{'FAIL'});
  error=$errorMessage;output_dir=$dir}
 [void]$steps.Add($entry)
 Save 'IN_PROGRESS' ("Last gate: "+$id)
 if($entry.status -ne 'PASS'){
  Save 'BLOCKED' ("Gate failed: "+$id)
  throw ("Gate {0} failed, exit {1}, error {2}. Details: {3}" -f $id,$code,$errorMessage,$reportPath)
 }
 Write-Host ("PASS {0}" -f $id) -ForegroundColor Green
}
try {
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'candidate/offline-20n-20261107'){
  throw ("Branch incorreta: "+$branch+". Nao use reset --hard ou git clean.")
 }
 $revision=(& git -C $root rev-parse HEAD)
 if($LASTEXITCODE -ne 0){throw 'Git HEAD indisponivel'}
 foreach($required in @($cmake,$fixture)){
  if(-not (Test-Path -LiteralPath $required -PathType Leaf)){
   throw ("Dependencia ausente: "+$required)
  }
 }
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 $env:QT_QPA_PLATFORM='offscreen'
 $env:ALENCAR_MINIMAL_VIEWPORT_20L='1'
 foreach($name in @('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H','ALENCAR_EVENT_PAINT_TRACE_20J')){
  [Environment]::SetEnvironmentVariable($name,$null,'Process')
 }
 Write-Warning 'Desconecte FISICAMENTE a maquina laser. Sem serial, Wi-Fi, G-code ou movimentos.'
 Write-Host ("Build isolado: "+$build) -ForegroundColor Yellow
 Save 'IN_PROGRESS' 'Preflight passed'
 & $cmake -S $source -B $build -G Ninja '-DCMAKE_BUILD_TYPE=Release' '-DALENCAR_REQUIRE_QT=ON' '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
 if($LASTEXITCODE -ne 0){throw 'Configuracao CMake falhou'}
 & $cmake --build $build --parallel 1 --target alencar_test_bulk_duplicate_20m alencar_test7_qt6
 if($LASTEXITCODE -ne 0){throw 'Compilacao do executavel candidato falhou'}
 foreach($file in @($exe,$core)){if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw ("Binario nao encontrado: "+$file)}}
 Gate 'G01_CORE_575' 'C++ duplicacao de 575 objetos e protecao de grupos' $core @($fixture)
 # The GUI modes write their own JSON under the unique gate directory.
 $gui=@(
  @('G02_CTRL_D_575','Ctrl+D com 575 objetos e Undo/Redo','--bulk-duplicate-shortcut-575-20m-test','-'),
  @('G03_TRANSFORMACAO','Transformacao e historico','--grouped-center-transform-19-9-test','-'),
  @('G04_ARRASTE','Arraste agrupado','--grouped-drag-19-9-test','-'),
  @('G05_TESOURA','Tesoura e topologia','--scissor-closures-19-8-test',$fixture),
  @('G06_RECUPERACAO','Recuperacao de sessao','--recovery-19-8-test',$fixture),
  @('G07_IMPORTACAO','Importacao LightBurn','--lbrn2-import-test',$fixture),
  @('G08_CAMADAS','Grupos e camadas','--layers-ungroup-19-8-test',$fixture),
  @('G09_MULTIFERRAMENTA','Ferramentas do editor','--editor-multitool-19-8-test',$fixture),
  @('G10_DESENHO','Desenho vetorial','--drawing-tools-19-8-test',$fixture),
  @('G11_TEXTO','Texto e estilos','--text-format-19-8-test',$fixture),
  @('G12_TEXT_FILL','Texto preenchimento/alinhamento','--text-fill-align-19-8-test',$fixture),
  @('G13_SIMULACAO','Simulacao de percurso, sem maquina','--simulation-motion-19-8-test',$fixture),
  @('G14_IMPORT_NATIVE','Bezier e importacao nativa','--native-import-gui-19-9-test','-')
 )
 foreach($gate in $gui){
  $dest=Join-Path $session ([string]$gate[0])
  Gate ([string]$gate[0]) ([string]$gate[1]) $exe @([string]$gate[2],[string]$gate[3],$dest)
 }
 Save 'AUTOMATED_PASS_MANUAL_PENDING' 'All offline regression gates passed, manual QA and installation pending.'
 Write-Host 'CANDIDATE 20n AUTOMATED PASS - NAO HOMOLOGADO PARA PRODUCAO.' -ForegroundColor Green
 Write-Host ("Executavel candidato: "+$exe) -ForegroundColor Cyan
 Write-Host ("Relatorio: "+$reportPath) -ForegroundColor Cyan
 if($Abrir){
  [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
  $manual=Join-Path $session 'SESSAO_MANUAL'
  [void](New-Item -ItemType Directory -Force -Path $manual)
  Write-Host 'Abrindo editor candidato. Teste Ctrl+D, Tesoura, selecao, zoom, Undo/Redo e exportacao de diagnostico. Feche a janela ao terminar.' -ForegroundColor Yellow
  & $exe --interactive $fixture $manual
  if($LASTEXITCODE -ne 0){throw ('Sessao manual fechou com erro: '+$LASTEXITCODE)}
 }
}catch {
 if(-not (Test-Path -LiteralPath $reportPath -PathType Leaf)){
  Save 'BLOCKED' $_.Exception.Message
 }else{
  $existing=Get-Content -LiteralPath $reportPath -Raw|ConvertFrom-Json
  if($existing.status -ne 'BLOCKED' -and $existing.status -ne 'AUTOMATED_PASS_MANUAL_PENDING'){
   Save 'BLOCKED' $_.Exception.Message
  }
 }
 Write-Host ("CANDIDATE 20n bloqueada: "+$_.Exception.Message) -ForegroundColor Red
 Write-Host ("Relatorio local: "+$reportPath) -ForegroundColor Yellow
 throw
}finally {
 $env:PATH=$oldPath
 foreach($name in $envNames){
  [Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')
 }
}
