#requires -Version 5.1
<#
ALENCAR LASER 20p - portable OFFLINE homologation pack.
Requires a PASS from candidate 20o on the SAME 20o source revision.
Copies only a verified development executable, Qt runtime and fixture into a
versioned folder. Never overwrites app/production or user projects.
#>
[CmdletBinding()]
param([string]$Destino='C:\ProjetoAlencarLaser\HOMOLOGACAO_PORTATIL')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$repo=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$reports=Join-Path $repo 'RELATORIOS\AUDITORIAS\GITHUB_19_8\CANDIDATE_20O'
$fixture=Join-Path $repo 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$qtBin='C:\msys64\ucrt64\bin'
$deployerCandidates=@((Join-Path $qtBin 'windeployqt6.exe'),(Join-Path $qtBin 'windeployqt.exe'))
function Ensure([bool]$condition,[string]$message){if(-not $condition){throw $message}}
function IsInside([string]$child,[string]$parent){
 $p=[IO.Path]::GetFullPath($parent).TrimEnd('\')+'\'
 $c=[IO.Path]::GetFullPath($child).TrimEnd('\')+'\'
 return $c.StartsWith($p,[StringComparison]::OrdinalIgnoreCase)
}
Ensure ((& git -C $repo branch --show-current) -eq 'release/20p-portable-homologation-20261109') 'Branch incorreta; troque para release/20p-portable-homologation-20261109'
Ensure ($LASTEXITCODE -eq 0) 'Falha Git'
$baseRev=(& git -C $repo rev-parse candidate/offline-20o-20261107).Trim()
Ensure ($LASTEXITCODE -eq 0 -and $baseRev -match '^[a-f0-9]{40}$') 'Nao encontrei a branch candidata 20o'
Ensure (Test-Path -LiteralPath $fixture -PathType Leaf) 'Fixture ausente'
Ensure (Test-Path -LiteralPath $reports -PathType Container) 'Execute os 14 gates 20o antes do empacotamento'
$deployer=@($deployerCandidates|Where-Object {Test-Path -LiteralPath $_ -PathType Leaf}|Select-Object -First 1)
Ensure ($deployer.Count -eq 1) 'Qt windeployqt nao encontrado. Instale ferramenta de deployment Qt UCRT64; nao copie bibliotecas manualmente de fontes desconhecidas.'
$approved=$null
foreach($file in @(Get-ChildItem -LiteralPath $reports -Recurse -File -Filter 'RESULTADO_CANDIDATE_20O.json' | Sort-Object LastWriteTimeUtc -Descending)){
 try{
  $r=Get-Content -LiteralPath $file.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
  if($r.status -ne 'AUTOMATED_PASS_MANUAL_PENDING' -or $r.source_revision -ne $baseRev -or
     $r.viewport_mode -ne 'minimal' -or $r.ctrl_d_bulk_fix -ne $true -or
     @($r.steps).Count -ne 14 -or @($r.steps|Where-Object {$_.status -ne 'PASS'}).Count -ne 0 -or
     -not (Test-Path -LiteralPath $r.executable_path -PathType Leaf)){continue}
  $hash=(Get-FileHash -LiteralPath $r.executable_path -Algorithm SHA256).Hash
  if($hash -ne $r.executable_sha256){continue}
  $approved=$r;break
 }catch{continue}
}
Ensure ($null -ne $approved) 'Sem 20o com 14 gates PASS da revisao exata, ou SHA-256 do exe mudou. Reexecute homologacao 20o; nao empacote a versao nao verificada.'
$destinationFull=[IO.Path]::GetFullPath($Destino)
$appRoot=[IO.Path]::GetFullPath((Join-Path $repo 'APLICATIVO'))
Ensure ($destinationFull -ne $appRoot -and -not (IsInside $destinationFull $appRoot)) 'Destino aponta para APLICATIVO de producao'
$sourceRoot=[IO.Path]::GetFullPath((Join-Path $repo 'DESENVOLVIMENTO\FONTE'))
Ensure ($destinationFull -ne $sourceRoot -and -not (IsInside $destinationFull $sourceRoot)) 'Destino dentro do codigo-fonte bloqueado'
Ensure (-not ($destinationFull -eq [IO.Path]::GetFullPath($repo))) 'Destino nao pode ser raiz do repositorio'
[void](New-Item -ItemType Directory -Path $destinationFull -Force)
$tag=(Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N').Substring(0,8)
$stage=Join-Path $destinationFull ('ALENCAR_LASER_20P_HOMOLOGACAO_'+$tag)
Ensure (-not (Test-Path -LiteralPath $stage)) 'Destino de homologacao ja existe'
[void](New-Item -ItemType Directory -Path $stage)
try{
 $program=Join-Path $stage 'Alencar_Laser_20P_Teste.exe'
 Copy-Item -LiteralPath $approved.executable_path -Destination $program -ErrorAction Stop
 $copiedHash=(Get-FileHash -LiteralPath $program -Algorithm SHA256).Hash
 Ensure ($copiedHash -eq $approved.executable_sha256) 'Falha de integridade ao copiar executavel'
 [void](New-Item -ItemType Directory -Path (Join-Path $stage 'fixtures') -Force)
 Copy-Item -LiteralPath $fixture -Destination (Join-Path $stage 'fixtures\web_nodes_normalized.csv')
 $oldPath=$env:PATH
 try{
  $env:PATH=$qtBin+';'+$oldPath
  & $deployer[0] '--release' '--no-translations' '--dir' $stage $program
  Ensure ($LASTEXITCODE -eq 0) 'windeployqt falhou'
 }finally{$env:PATH=$oldPath}
 # Explicit UCRT toolchain runtime DLLs; these ship from the same known toolchain.
 foreach($dll in @('libstdc++-6.dll','libgcc_s_seh-1.dll','libwinpthread-1.dll')){
  $origin=Join-Path $qtBin $dll
  if(Test-Path -LiteralPath $origin -PathType Leaf){
   Copy-Item -LiteralPath $origin -Destination (Join-Path $stage $dll)
  }
 }
 Ensure (Test-Path -LiteralPath (Join-Path $stage 'platforms\qwindows.dll') -PathType Leaf) 'Qt platforms/qwindows.dll ausente; dependencia incompleta'
 # Copy the launcher supplied with the release branch.
 $launcher=Join-Path $PSScriptRoot 'EXECUTAR_PORTATIL_20P_MODELO.ps1'
 Ensure (Test-Path -LiteralPath $launcher -PathType Leaf) 'Modelo do launcher nao encontrado'
 Copy-Item -LiteralPath $launcher -Destination (Join-Path $stage 'EXECUTAR_ALENCAR_LASER_20P.ps1')
 # Generate manifest LAST, excluding the manifest itself.
 $entries=@(Get-ChildItem -LiteralPath $stage -Recurse -File | ForEach-Object {
  $relative=$_.FullName.Substring($stage.Length+1).Replace('\','/')
  [ordered]@{path=$relative;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash;bytes=[long]$_.Length}
 }|Sort-Object {$_.path})
 $manifest=[ordered]@{
  schema='alencar-laser-portable-homologation-20p-1'
  source_branch='candidate/offline-20o-20261107'
  source_revision=$baseRev
  status='PORTABLE_CREATED_MANUAL_PENDING'
  created_utc=(Get-Date).ToUniversalTime().ToString('o')
  laser_hardware_homologated=$false
  production_installed=$false
  offline_fixture_only=$true
  minimal_viewport_opt_in=$true
  files=$entries
 }
 $manifestPath=Join-Path $stage 'MANIFESTO_SHA256_20P.json'
 [IO.File]::WriteAllText($manifestPath,($manifest|ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
 # Verify manifest content before archive.
 foreach($entry in $entries){
  $file=Join-Path $stage ($entry.path.Replace('/','\'))
  Ensure ((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -eq $entry.sha256) ("Hash incorreto "+$entry.path)
 }
 $zip=$stage+'.zip'
 Ensure (-not (Test-Path -LiteralPath $zip)) 'ZIP ja existe'
 Compress-Archive -LiteralPath $stage -DestinationPath $zip -CompressionLevel Optimal -ErrorAction Stop
 Ensure (Test-Path -LiteralPath $zip -PathType Leaf) 'ZIP nao criado'
 $zipHash=(Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash
 Write-Host 'PACOTE PORTATIL 20p CRIADO. HOMOLOGACAO MANUAL AINDA NECESSARIA.' -ForegroundColor Green
 Write-Host ('Pasta: '+$stage)
 Write-Host ('ZIP: '+$zip)
 Write-Host ('ZIP SHA256: '+$zipHash)
 Write-Host 'Para abrir, extraia o ZIP, desconecte o laser e execute EXECUTAR_ALENCAR_LASER_20P.ps1.' -ForegroundColor Yellow
 Write-Host 'Nao substitua o programa instalado nem use a maquina fisica.' -ForegroundColor Yellow
}catch{
 Write-Host ('EMPACOTAMENTO BLOQUEADO: '+$_.Exception.Message) -ForegroundColor Red
 Write-Host ('Pasta de tentativa preservada para diagnostico: '+$stage) -ForegroundColor Yellow
 throw
}
