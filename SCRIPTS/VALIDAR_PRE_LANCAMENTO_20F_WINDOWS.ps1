#requires -Version 5.1
<#
ALENCAR LASER — release readiness 20f. Read-only with respect to application,
user projects, installer and laser. Invokes existing offline development tests.
Runs in an isolated child PowerShell per gate; logs and JSON are local only.
This is NOT a production certification and never calls -Abrir.
#>
[CmdletBinding()]
param([switch]$Rapido,[switch]$IncluirPerformance,[string]$ArquivoLbrn2='')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\RELEASE_READINESS_20F'
$stamp=(Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8)
$session=Join-Path $reports $stamp
[void](New-Item -ItemType Directory -Path $session -Force)
$steps=New-Object System.Collections.ArrayList
$summary=Join-Path $session 'RESULTADO_RELEASE_READINESS_20F.json'
$baseline=Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
$drag=Join-Path $PSScriptRoot 'COMPARAR_DRAG_OVERLAY_19_9.ps1'
$dense=Join-Path $PSScriptRoot 'COMPARAR_DRAG_CONTROLADO_575_19_9.ps1'
function SaveSummary([string]$status,[string]$message){
 $data=[ordered]@{
  schema='alencar-laser-readiness-20f-1'
  status=$status
  message=$message
  generated_utc=(Get-Date).ToUniversalTime().ToString('o')
  experimental_only=$true
  application_installed_modified=$false
  hardware_commands_sent_by_runner=$false
  production_homologated=$false
  manual_visual_tests_pending=$true
  branch_expected='test/release-readiness-20f-20261106'
  source_revision=$script:revision
  run_directory=$session
  steps=@($steps.ToArray())
 }
 [IO.File]::WriteAllText($summary,($data|ConvertTo-Json -Depth 9),
   [Text.UTF8Encoding]::new($false))
}
function Gate([string]$id,[string]$description,[string]$file,[string[]]$arguments){
 $stdout=Join-Path $session ($id+'_STDOUT.txt')
 $stderr=Join-Path $session ($id+'_STDERR.txt')
 $started=Get-Date
 Write-Host ("INICIANDO {0}: {1}" -f $id,$description) -ForegroundColor Cyan
 $exitCode=-999
 $errorMessage=''
 try{
  $argv=@('-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $file + '"'))
  foreach($arg in $arguments) {
   # Quote only when necessary: explicit path arguments may contain spaces.
   if($arg.Contains('"')){throw 'Argumento com aspas nao permitido'}
   $argv+= $(if($arg -match '\s') { '"'+$arg+'"' } else { $arg })
  }
  $p=Start-Process -FilePath 'powershell.exe' -ArgumentList $argv -WorkingDirectory $root -NoNewWindow -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
  $exitCode=$p.ExitCode
 }catch{
  $errorMessage=$_.Exception.Message
 }
 $entry=[ordered]@{
  id=$id;description=$description
  status=$(if($exitCode -eq 0 -and !$errorMessage){'PASS'}else{'FAIL'})
  exit_code=$exitCode
  duration_seconds=[math]::Round(((Get-Date)-$started).TotalSeconds,1)
  stdout=$stdout;stderr=$stderr
  error=$errorMessage
 }
 [void]$steps.Add($entry)
 SaveSummary 'IN_PROGRESS' "Last gate: $id"
 if($entry.status -ne 'PASS'){
  Write-Host "FALHA EM $id (codigo $exitCode): $errorMessage" -ForegroundColor Red
  if(Test-Path -LiteralPath $stderr){Get-Content -LiteralPath $stderr -Tail 35 | Out-Host}
  if(Test-Path -LiteralPath $stdout){Get-Content -LiteralPath $stdout -Tail 50 | Out-Host}
  SaveSummary 'BLOCKED' ("Gate falhou: "+$id)
  throw ("Validacao interrompida no gate {0}. Relatorio: {1}" -f $id,$summary)
 }
 Write-Host "PASS: $id" -ForegroundColor Green
}
$revision='unknown'
try{
 $branchName=(& git -C $root branch --show-current 2>$null)
 $revision=(& git -C $root rev-parse --verify HEAD 2>$null)
 if($LASTEXITCODE -ne 0){throw 'Repositorio Git nao disponivel'}
 if($branchName -ne 'test/release-readiness-20f-20261106'){
  throw "Branch incorreta: $branchName. Nenhum teste iniciado."
 }
 foreach($required in @($baseline,$drag)){
  if(-not (Test-Path -LiteralPath $required -PathType Leaf)){throw "Script ausente: $required"}
 }
 if($IncluirPerformance -and -not (Test-Path -LiteralPath $dense -PathType Leaf)){
  throw "Benchmark denso ausente: $dense"
 }
 if(-not [string]::IsNullOrWhiteSpace($ArquivoLbrn2) -and
    -not (Test-Path -LiteralPath $ArquivoLbrn2 -PathType Leaf)){
   throw "LBRN2 real nao encontrado: $ArquivoLbrn2"
 }
 Write-Warning 'Maquina laser deve estar FISICAMENTE desconectada.'
 Write-Host 'NAO executa -Abrir, nem instalador, nem merge/push.' -ForegroundColor Yellow
 SaveSummary 'IN_PROGRESS' 'Preflight passed'
 # Main suite: core C++ and integrated Qt regression across geometry, save/recovery,
 # scissor, draw, layers, simulation, history, and safe import.
 $args=@()
 if(-not [string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
  $args=@('-ArquivoLbrn2',(Resolve-Path -LiteralPath $ArquivoLbrn2).Path)
 }
 Gate 'G01_INTEGRACAO' 'Suite integral C++ e Qt (sem abrir editor)' $baseline $args
 if(-not $Rapido){
  Gate 'G02_BEZIER_GUI' 'Codec/importacao Bezier GUI e salvamento v5' $baseline @('-SomenteBezierNativoUI')
  Gate 'G03_IMPORTACAO_SEGURA' 'Parser LightBurn rejeita conteudo invalido' $baseline @('-SomenteImportacaoSegura')
  Gate 'G04_TESOURA' 'Tesoura Qt + nucleos e Undo/Redo' $baseline @('-SomenteTesoura')
  Gate 'G05_FLUIDEZ' 'Historico e arraste agrupado Qt' $baseline @('-SomenteFluidez')
  Gate 'G06_AB_40' 'A/B do repaint de selecao com 40 contornos' $drag @()
 }
 if($IncluirPerformance){
  Gate 'G07_AB_575' 'A/B 575 contornos; sem alegacao de speedup ON3' $dense @('-ModoOtimizado','3')
 }
 SaveSummary 'AUTOMATED_PASS_MANUAL_PENDING' 'Gates executados; QA manual, instalacao e hardware pendentes'
 Write-Host 'AUTOMATED PASS. NAO HOMOLOGADO PARA PRODUCAO.' -ForegroundColor Green
 Write-Host "Resumo: $summary" -ForegroundColor Cyan
 Write-Host 'Proxima etapa: validar mouse real, DPI, arquivos reais, recovery e instalacao em copia de teste.'
}catch{
 if(-not (Test-Path -LiteralPath $summary -PathType Leaf)){
  SaveSummary 'BLOCKED' $_.Exception.Message
 }
 Write-Host ("PRE-LANCAMENTO BLOQUEADO: "+$_.Exception.Message) -ForegroundColor Red
 Write-Host ("Resumo local: "+$summary) -ForegroundColor Yellow
 throw
}
