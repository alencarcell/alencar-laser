#requires -Version 5.1
<#
Abrir somente a build de LABORATORIO validada por 6 gates da fase 20r.
Sem instalacao, sem alteracao de projetos originais, sem laser conectado.
#>
[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$reports=Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\GEAR_20R'
$fixture=Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$branch=(& git -C $root branch --show-current)
if($LASTEXITCODE -ne 0 -or $branch -ne 'feat/20r-measured-tooth-preview-20261109'){
 throw 'Branch 20r nao esta ativa; nao use force, reset nem clean.'
}
$revision=(& git -C $root rev-parse HEAD)
if($LASTEXITCODE -ne 0){throw 'Falha ao identificar commit'}
if(-not (Test-Path -LiteralPath $reports -PathType Container)){throw 'Execute primeiro TESTAR_ENGRENAGEM_ANGULO_ZOOM_20R_WINDOWS.ps1'}
if(-not (Test-Path -LiteralPath $fixture -PathType Leaf)){throw 'Fixture ausente'}
$valid=$null
foreach($file in @(Get-ChildItem -LiteralPath $reports -Filter 'RESULTADO_GEAR_20R.json' -File -Recurse |
                  Sort-Object LastWriteTimeUtc -Descending)){
 $r=Get-Content -LiteralPath $file.FullName -Raw -Encoding UTF8|ConvertFrom-Json
 if($r.status -ne 'AUTOMATED_PASS_MANUAL_PENDING' -or
    $r.source_revision -ne $revision -or
    @($r.gates).Count -ne 6 -or
    @($r.gates|Where-Object {$_.result -ne 'PASS'}).Count -ne 0 -or
    -not (Test-Path -LiteralPath $r.executable_path -PathType Leaf)){continue}
 if((Get-FileHash -LiteralPath $r.executable_path -Algorithm SHA256).Hash -ne $r.executable_sha256){continue}
 $valid=$r;break
}
if($null -eq $valid){throw 'Nao encontrei build atual com 6 gates PASS e SHA256 valido; reexecute os testes 20r.'}
$oldPath=$env:PATH
$envNames=@('QT_QPA_PLATFORM','ALENCAR_MINIMAL_VIEWPORT_20L','ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K','ALENCAR_TRANSFORM_LOCAL_REPAINT_20H')
$previous=@{}
foreach($name in $envNames){$previous[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
try{
 $env:PATH='C:\msys64\ucrt64\bin;'+$oldPath
 [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_MINIMAL_VIEWPORT_20L',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K',$null,'Process')
 [Environment]::SetEnvironmentVariable('ALENCAR_TRANSFORM_LOCAL_REPAINT_20H',$null,'Process')
 $session=Join-Path $reports ('TESTE_VISUAL_'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
 [void](New-Item -ItemType Directory -Path $session -Force)
 Write-Warning 'Mantenha fisicamente desconectada a maquina laser.'
 Write-Host 'Abra Gerar > Engrenagens > Engrenagem, escolha Angular/Serrilhado, Definicao: Por angulo interno; teste Zoom/Ajustar.' -ForegroundColor Cyan
 Write-Host 'Use somente a fixture e contornos de laboratorio; nao sobrescreva trabalhos originais.' -ForegroundColor Yellow
 & $valid.executable_path --interactive $fixture $session
 if($LASTEXITCODE -ne 0){throw ("Editor Qt encerrou com erro: "+$LASTEXITCODE)}
 Write-Host 'Sessao visual do gerador encerrada; instalacao principal preservada.' -ForegroundColor Green
}catch{
 Write-Host ("FALHA DO TESTE VISUAL 20r: "+$_.Exception.Message) -ForegroundColor Red
 throw
}finally{
 $env:PATH=$oldPath
 foreach($name in $envNames){[Environment]::SetEnvironmentVariable($name,$previous[$name],'Process')}
}
