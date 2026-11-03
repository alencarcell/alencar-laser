#requires -Version 5.1
<#
Diagnostico nao destrutivo do fechamento do editor Alencar Laser 19.9.
Captura stack C++ se MSYS2/UCRT64 gdb estiver disponivel.
Nao instala dependencias, altera QSettings ou envia comandos a laser.
A sessao pode abrir a janela: reproduza o problema e feche normalmente se funcionar.
#>
[CmdletBinding()]
param()
$ErrorActionPreference = "Stop"
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$ucrt = 'C:\msys64\ucrt64\bin'
$gdb = Join-Path $ucrt 'gdb.exe'
$program = Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_GIT_19_8\alencar_test7_qt6.exe'
$fixture = Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$session = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\diagnostico_inicio'
$logs = Join-Path $root 'RELATORIOS\DIAGNOSTICOS'
$log = Join-Path $logs ("FECHAMENTO_GDB_19_9_{0}.txt" -f (Get-Date -Format 'yyyyMMdd_HHmmss'))
if (!(Test-Path -LiteralPath $program -PathType Leaf)) { throw "Executavel ausente: $program" }
if (!(Test-Path -LiteralPath $fixture -PathType Leaf)) { throw "Fixture ausente: $fixture" }
if (!(Test-Path -LiteralPath $gdb -PathType Leaf)) {
 Write-Warning "Depurador ainda nao instalado: $gdb"
 Write-Host "Abra o terminal MSYS2 UCRT64 e instale com: pacman -S mingw-w64-ucrt-x86_64-gdb"
 Write-Host "O Alencar Laser e seus projetos NAO foram alterados."
 return
}
[void](New-Item -Path $logs -ItemType Directory -Force)
[void](New-Item -Path $session -ItemType Directory -Force)
$oldPath = $env:PATH
$oldDir = (Get-Location).Path
try {
 $env:PATH = "$ucrt;$oldPath"
 Set-Location -LiteralPath $root
 Write-Host "Reproduza o encerramento da janela Alencar Laser. Nenhuma emissao laser sera habilitada." -ForegroundColor Yellow
 Write-Host "Captura GDB: $log"
 & $gdb --quiet --batch --nx -ex "set pagination off" -ex "set confirm off" -ex "run" -ex "thread apply all bt 35" --args $program --interactive $fixture $session 2>&1 | Tee-Object -FilePath $log
 Write-Host "Relatorio: $log" -ForegroundColor Cyan
 Write-Host "Envie este TXT para identificar a funcao e a linha que originaram map::at."
}
finally {
 $env:PATH = $oldPath
 Set-Location -LiteralPath $oldDir
}
