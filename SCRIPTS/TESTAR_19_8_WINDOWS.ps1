# Alencar Laser 19.8 — build de desenvolvimento Windows
# Executar na raiz do repositorio:
# powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -Abrir
param([switch]$Abrir)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$raiz = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$fontes = Join-Path $raiz 'DESENVOLVIMENTO\FONTE'
$build = Join-Path $fontes 'build_WINDOWS_GIT_19_8'
$relatorios = Join-Path $raiz 'RELATORIOS\COMPILACAO'
$cmake = 'C:\msys64\ucrt64\bin\cmake.exe'
$ninja = 'C:\msys64\ucrt64\bin\ninja.exe'
$gpp = 'C:\msys64\ucrt64\bin\g++.exe'

foreach ($arquivo in @(
    (Join-Path $fontes 'CMakeLists.txt'),
    $cmake, $ninja, $gpp
)) {
    if (-not (Test-Path -LiteralPath $arquivo -PathType Leaf)) {
        throw "Dependencia ausente: $arquivo"
    }
}
New-Item -ItemType Directory -Path $relatorios -Force | Out-Null
$relatorio = Join-Path $relatorios 'BUILD_GITHUB_19_8.txt'
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
$transcript = $false

function Checar-Saida([string]$etapa) {
    if ($LASTEXITCODE -ne 0) {
        throw "$etapa falhou (codigo $LASTEXITCODE). Veja $relatorio"
    }
}

try {
    Start-Transcript -Path $relatorio -Force | Out-Null
    $transcript = $true
    Write-Host 'ALENCAR LASER 19.8 - VALIDACAO DA BRANCH GITHUB' -ForegroundColor Cyan
    Write-Host "Fontes: $fontes"
    Write-Host "Build isolado: $build"
    & $cmake -S $fontes -B $build -G Ninja `
        '-DCMAKE_BUILD_TYPE=Release' `
        '-DALENCAR_REQUIRE_QT=ON' `
        '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' `
        '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
    Checar-Saida 'Configuracao CMake'

    $alvos = @(
        'alencar_test7_qt6',
        'alencar_test_delete_bezier_anchor',
        'alencar_test19_5_grbl_observer',
        'alencar_test19_6_rules',
        'alencar_test19_7_scissors_layers',
        'alencar_test19_8_recovery_policy',
        'alencar_test_v5_codec_regression'
    )
    & $cmake --build $build --parallel 2 --target $alvos
    Checar-Saida 'Compilacao'

    foreach ($teste in $alvos | Where-Object { $_ -ne 'alencar_test7_qt6' }) {
        $exe = Join-Path $build ($teste + '.exe')
        if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
            throw "Teste nao compilado: $exe"
        }
        Write-Host "TESTANDO: $teste" -ForegroundColor Cyan
        & $exe
        Checar-Saida "Teste $teste"
    }

    $app = Join-Path $build 'alencar_test7_qt6.exe'
    $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
    $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8'
    New-Item -ItemType Directory -Path $destino -Force | Out-Null
    if (-not (Test-Path -LiteralPath $fixture -PathType Leaf)) {
        throw "Fixture nao encontrado: $fixture"
    }

    # Ambos sao testes em ambiente local sem maquina fisica.
    & $app --edit-tools-19-7-test $fixture (Join-Path $destino 'ferramentas')
    Checar-Saida 'Regressao GUI das ferramentas'
    & $app --recovery-19-8-test $fixture (Join-Path $destino 'recuperacao')
    Checar-Saida 'Recuperacao .alaser v5'

    Write-Host 'BUILD E REGRESSOES AUTOMATIZADAS PASSARAM. AVALIACAO VISUAL AINDA NECESSARIA.' -ForegroundColor Green
    if ($Abrir) {
        Write-Host 'Abrindo editor em modo de desenvolvimento; mantenha o laser desconectado.' -ForegroundColor Yellow
        & $app --interactive $fixture (Join-Path $destino 'sessao')
        Checar-Saida 'Sessao do editor'
    }
} finally {
    if ($transcript) { Stop-Transcript | Out-Null }
}
Write-Host "Relatorio: $relatorio"
