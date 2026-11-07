#requires -Version 5.1
<#
Open the same compiled candidate that passed integrated offline gates.
Does NOT install/replace Alencar Laser. Fixture only, laser disconnected.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\CANDIDATE_20O'
$fixture=Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$envNames=@('QT_QPA_PLATFORM','ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H')
$old=@{}
foreach($n in $envNames){$old[$n]=[Environment]::GetEnvironmentVariable($n,'Process')}
$oldPath=$env:PATH
try{
 $branch=(& git -C $root branch --show-current)
 if($LASTEXITCODE -ne 0 -or $branch -ne 'candidate/offline-20o-20261107'){
  throw 'Branch incorreta. Nao force troca de arquivos.'
 }
 $head=(& git -C $root rev-parse HEAD)
 if($LASTEXITCODE -ne 0){throw 'Git HEAD indisponivel'}
 if(-not (Test-Path -LiteralPath $reports -PathType Container)){throw 'Execute o gate de homologacao antes de abrir.'}
 $passing=@(Get-ChildItem -LiteralPath $reports -Filter 'RESULTADO_CANDIDATE_20O.json' -Recurse -File |
  Sort-Object LastWriteTimeUtc -Descending)
 $valid=$null
 foreach($p in $passing){
  $r=Get-Content -LiteralPath $p.FullName -Encoding UTF8 -Raw|ConvertFrom-Json
  if($r.status -eq 'AUTOMATED_PASS_MANUAL_PENDING' -and
     $r.source_revision -eq $head -and
     $r.viewport_mode -eq 'minimal' -and
     $r.ctrl_d_bulk_fix -eq $true -and
     (Test-Path -LiteralPath $r.executable_path -PathType Leaf) -and
     (Get-FileHash -LiteralPath $r.executable_path -Algorithm SHA256).Hash -eq $r.executable_sha256){
   $valid=$r;break
  }
 }
 if($null -eq $valid){throw 'Nao ha build aprovado da revisao Git atual com SHA256 valido. Execute HOMOLOGAR_CANDIDATA_OFFLINE_20O_WINDOWS.ps1.'}
 if(-not (Test-Path -LiteralPath $fixture -PathType Leaf)){throw 'Fixture de laboratorio ausente'}
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
 $env:ALENCAR_MINIMAL_VIEWPORT_20L='1'
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 $dir=Join-Path $valid.run_directory ('SESSAO_OPERADOR_'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
 [void](New-Item -ItemType Directory -Force -Path $dir)
 Write-Warning 'Laser deve estar fisicamente DESCONECTADO.'
 Write-Host 'Abrindo candidato de laboratorio aprovado pelos gates automatizados.' -ForegroundColor Green
 Write-Host ('Executavel: '+$valid.executable_path)
 & $valid.executable_path --interactive $fixture $dir
 if($LASTEXITCODE -ne 0){throw ('Editor fechou com erro '+$LASTEXITCODE)}
 Write-Host ('Sessao finalizada. Dados locais: '+$dir) -ForegroundColor Cyan
}catch{
 Write-Host ('ABERTURA BLOQUEADA: '+$_.Exception.Message) -ForegroundColor Red
 throw
}finally{
 $env:PATH=$oldPath
 foreach($n in $envNames){[Environment]::SetEnvironmentVariable($n,$old[$n],'Process')}
}
