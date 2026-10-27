# Alencar Laser 19.8 — build de desenvolvimento Windows
# Executar na raiz do repositorio:
# powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -Abrir
param([switch]$Abrir, [switch]$SomenteTesoura, [switch]$SomenteImportacao, [switch]$SomenteFerramentas)
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
# Os executaveis GUI escrevem a causa real em JSON, inclusive ao retornar codigo 1.
# Exibir o relatorio no proprio terminal evita outro ciclo de compilacao as cegas.
function Checar-ResultadoQt([string]$etapa, [string]$arquivo) {
    $codigo = $LASTEXITCODE
    if ($codigo -ne 0) {
        Write-Host "FALHA NA ETAPA: $etapa (codigo $codigo)" -ForegroundColor Red
        if (Test-Path -LiteralPath $arquivo -PathType Leaf) {
            Write-Host "Detalhes do teste (JSON):" -ForegroundColor Yellow
            Write-Host ([System.IO.File]::ReadAllText($arquivo))
        } else {
            Write-Host "O executavel nao produziu JSON: $arquivo" -ForegroundColor Yellow
        }
        throw "$etapa falhou (codigo $codigo). Consulte o JSON acima."
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

    if ($SomenteFerramentas) {
        Write-Host 'TESTE FOCADO: Texto vetorial e Lapis experimental.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt das ferramentas'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\desenho'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --drawing-tools-19-8-test $fixture $destino
        Checar-ResultadoQt 'Texto e Lapis' (Join-Path $destino 'RESULTADO_TEXTO_LAPIS_19_8.json')
        Write-Host 'Texto/Lapis passaram nos testes sinteticos. Teste manual ainda necessario.' -ForegroundColor Green
        return
    }
    if ($SomenteImportacao) {
        Write-Host 'VALIDACAO FOCADA: importacao segura LightBurn .lbrn2.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt de importacao'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\importacao'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --lbrn2-import-test $fixture $destino
        Checar-ResultadoQt 'Importacao LBRN2' (Join-Path $destino 'RESULTADO_IMPORTACAO_LBRN2.json')
        Write-Host 'Teste de importacao passou. Teste completo ainda necessario.' -ForegroundColor Green
        return
    }
    if ($SomenteTesoura) {
        Write-Host 'DIAGNOSTICO FOCADO: compila apenas Qt e executa teste da Tesoura.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt focada'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\tesoura_foco'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --edit-tools-19-7-test $fixture $destino
        Checar-ResultadoQt 'Tesoura - hover GUI' (Join-Path $destino 'RESULTADO_TESTE_19_7_GUI.json')
        Write-Host 'Teste focado passou; validacao completa ainda obrigatoria.' -ForegroundColor Green
        return
    }
    $alvos = @(
        'alencar_test7_qt6',
        'alencar_test_delete_bezier_anchor',
        'alencar_test19_5_grbl_observer',
        'alencar_test19_6_rules',
        'alencar_test19_7_scissors_layers',
        'alencar_test19_8_scissors_span',
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
    $dirFerramentas = Join-Path $destino 'ferramentas'
    $dirRecuperacao = Join-Path $destino 'recuperacao'
    & $app --edit-tools-19-7-test $fixture $dirFerramentas
    Checar-ResultadoQt 'Regressao GUI das ferramentas' (Join-Path $dirFerramentas 'RESULTADO_TESTE_19_7_GUI.json')
    & $app --recovery-19-8-test $fixture $dirRecuperacao
    Checar-ResultadoQt 'Recuperacao persistente GUI 19.8' (Join-Path $dirRecuperacao 'RESULTADO_RECUPERACAO_19_8.json')
    $dirImportacao = Join-Path $destino 'importacao'
    & $app --lbrn2-import-test $fixture $dirImportacao
    Checar-ResultadoQt 'Importacao geometrica LBRN2' (Join-Path $dirImportacao 'RESULTADO_IMPORTACAO_LBRN2.json')
    $dirDesenho = Join-Path $destino 'desenho'
    New-Item -ItemType Directory -Path $dirDesenho -Force | Out-Null
    & $app --drawing-tools-19-8-test $fixture $dirDesenho
    Checar-ResultadoQt 'Texto e Lapis Qt' (Join-Path $dirDesenho 'RESULTADO_TEXTO_LAPIS_19_8.json')

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
