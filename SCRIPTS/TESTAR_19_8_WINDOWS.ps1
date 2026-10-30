# Alencar Laser 19.8 — build de desenvolvimento Windows
# Executar na raiz do repositorio:
# powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -Abrir
param([switch]$Abrir, [switch]$SomenteMarca, [switch]$SomenteTesoura, [switch]$SomenteImportacao, [switch]$SomenteFerramentas, [switch]$SomenteEdicao, [switch]$SomenteInteracao, [switch]$SomenteCamadas, [switch]$SomenteCasoReal, [switch]$SomenteRefino, [switch]$SomenteRaster, [switch]$SomenteDesenho, [switch]$SomenteSimulacao, [string]$ArquivoLbrn2 = "")
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

# Captura ambos os fluxos do processo nativo. Start-Transcript sozinho pode
# nao registrar o stderr emitido pelo g++/ninja no Windows PowerShell 5.1.
function Compilar-QtComDiagnostico {
    $saidaPadrao = Join-Path $relatorios 'BUILD_QT_19_8_STDOUT.txt'
    $saidaErros = Join-Path $relatorios 'BUILD_QT_19_8_STDERR.txt'
    Write-Host 'Compilando Qt em modo diagnostico (1 processo); saida em arquivos separados...' -ForegroundColor Cyan
    $processo = Start-Process -FilePath $cmake -ArgumentList @(
        '--build', $build, '--parallel', '1', '--target', 'alencar_test7_qt6'
    ) -WorkingDirectory $raiz -PassThru -Wait -NoNewWindow `
      -RedirectStandardOutput $saidaPadrao -RedirectStandardError $saidaErros
    if ($processo.ExitCode -ne 0) {
        Write-Host "Compilacao Qt falhou (codigo $($processo.ExitCode)). PRIMEIROS ERROS REAIS:" -ForegroundColor Red
        $ocorrencias = @(Select-String -Path @($saidaPadrao, $saidaErros) `
            -Pattern 'error:', 'fatal error:', 'FAILED:', 'undefined reference', 'collect2:', 'killed', 'terminated' `
            -Context 3,4 | Select-Object -First 12)
        if ($ocorrencias.Count -gt 0) {
            $ocorrencias | Out-String -Width 240 | Write-Host
        } else {
            Write-Host 'Nenhum marcador de erro encontrado; ultimas 70 linhas de cada fluxo:' -ForegroundColor Yellow
            Get-Content -LiteralPath $saidaPadrao -Tail 70 | Out-Host
            Get-Content -LiteralPath $saidaErros -Tail 70 | Out-Host
        }
        Write-Host "LOG STDOUT: $saidaPadrao" -ForegroundColor Yellow
        Write-Host "LOG STDERR: $saidaErros" -ForegroundColor Yellow
        throw "Compilacao Qt de refinamento falhou (codigo $($processo.ExitCode)). Erros exibidos acima."
    }
    Write-Host 'Compilacao Qt concluida. Iniciando testes de refino.' -ForegroundColor Green
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
    if ($SomenteRaster) {
        # Fast isolated check for raster fixtures: skips CMake and Qt6.
        Write-Host 'TESTE RAPIDO: C++ raster offline, sem configuracao CMake e sem Qt6.' -ForegroundColor Cyan
        New-Item -ItemType Directory -Path $build -Force | Out-Null
        $rasterSource = Join-Path $fontes 'tests\preview_raster_19_8_standalone.cpp'
        $rasterStandalone = Join-Path $build 'preview_raster_19_8_standalone.exe'
        & $gpp -std=c++20 -O2 -Wall -Wextra $rasterSource -o $rasterStandalone
        Checar-Saida 'Compilacao isolada do raster 19.8'
        & $rasterStandalone
        Checar-Saida 'Teste isolado do raster 19.8'
        Write-Host 'RASTER C++ PASS. Ainda faltam os testes Qt6 e da suite integrada.' -ForegroundColor Green
        return
    }
    & $cmake -S $fontes -B $build -G Ninja `
        '-DCMAKE_BUILD_TYPE=Release' `
        '-DALENCAR_REQUIRE_QT=ON' `
        '-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64' `
        '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe'
    Checar-Saida 'Configuracao CMake'

    if ($SomenteMarca) {
        Write-Host 'IDENTIDADE PEGASO_V1: compila Qt e testa o carregamento opcional, sem hardware.' -ForegroundColor Cyan
        Compilar-QtComDiagnostico
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\\AUDITORIAS\\GITHUB_19_8\\marca_pegaso'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --branding-19-8-test $fixture $destino
        Checar-ResultadoQt 'Identidade Pegaso 19.8' (Join-Path $destino 'RESULTADO_IDENTIDADE_PEGASO_19_8.json')
        Write-Host 'Identidade Pégaso: PASS sintético. Verificar cabeçalho, ícone e splash manualmente depois.' -ForegroundColor Green
        return
    }
    if ($SomenteDesenho) {
        Write-Host 'DESENHO 19.8: snap/Auto-Join, cliques e arraste Bézier sem hardware.' -ForegroundColor Cyan
        $testSource = Join-Path $fontes 'tests\preview_draw_join_19_8_standalone.cpp'
        $testExe = Join-Path $build 'preview_draw_join_19_8_standalone.exe'
        & $gpp -std=c++20 -O2 -Wall -Wextra $testSource -o $testExe
        Checar-Saida 'Compilacao standalone de snap, Auto-Join e Bezier'
        & $testExe
        Checar-Saida 'Teste standalone de desenho'
        Compilar-QtComDiagnostico
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\desenho_uniao'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --draw-join-19-8-test $fixture $destino
        Checar-ResultadoQt 'Desenhar Bezier e unir extremidades' (Join-Path $destino 'RESULTADO_DESENHO_UNIAO_19_8.json')
        & $app --text-format-19-8-test $fixture $destino
        Checar-ResultadoQt 'Painel de fontes e formatacao de texto' (Join-Path $destino 'RESULTADO_FORMATACAO_TEXTO_19_8.json')
        & $app --text-fill-align-19-8-test $fixture $destino
        Checar-ResultadoQt 'Texto reeditavel, Fill vazado, alinhamento e zoom' (Join-Path $destino 'RESULTADO_TEXTO_FILL_ALINHAMENTO_19_8.json')
        Write-Host 'DESENHO C++ + Qt PASS sintetico; teste manual do mouse ainda necessario.' -ForegroundColor Green
        return
    }
    if ($SomenteSimulacao) {
        Write-Host 'PREVIA OFFLINE: teste C++20 independente + Qt com relatorio JSON.' -ForegroundColor Cyan
        $source = Join-Path $fontes 'tests\preview_motion_19_8_standalone.cpp'
        $standalone = Join-Path $build 'preview_motion_19_8_standalone.exe'
        & $gpp -std=c++20 -Wall -Wextra -Werror -O2 $source -o $standalone
        Checar-Saida 'Compilacao do simulador C++ independente'
        & $standalone
        Checar-Saida 'Teste C++ de movimento'
        $rasterSource = Join-Path $fontes 'tests\preview_raster_19_8_standalone.cpp'
        $rasterStandalone = Join-Path $build 'preview_raster_19_8_standalone.exe'
        & $gpp -std=c++20 -O2 -Wall $rasterSource -o $rasterStandalone
        Checar-Saida 'Compilacao C++ do raster virtual'
        & $rasterStandalone
        Checar-Saida 'Teste C++ de Fill e bitmap'
        $routeSource = Join-Path $fontes 'tests\preview_route_19_8_standalone.cpp'
        $routeStandalone = Join-Path $build 'preview_route_19_8_standalone.exe'
        & $gpp -std=c++20 -O2 -Wall $routeSource -o $routeStandalone
        Checar-Saida 'Compilacao C++ da rota virtual'
        & $routeStandalone
        Checar-Saida 'Teste C++ de precedencia/rota virtual'
        Compilar-QtComDiagnostico
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\simulacao'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --simulation-motion-19-8-test $fixture $destino
        Checar-ResultadoQt 'Previa virtual cinemática' (Join-Path $destino 'RESULTADO_PREVIA_MOVIMENTO_19_8.json')
        & $app --simulation-raster-19-8-test $fixture $destino
        Checar-ResultadoQt 'Previa virtual Fill e imagem P&B' (Join-Path $destino 'RESULTADO_PREVIA_RASTER_19_8.json')
        & $app --layer-route-19-8-test $fixture $destino
        Checar-ResultadoQt 'Camadas compactas, parametros e rota heuristica' (Join-Path $destino 'RESULTADO_CAMADA_ROTA_19_8.json')
        & $app --route-nesting-19-8-test $fixture $destino
        Checar-ResultadoQt 'Roteiro de contornos internos primeiro' (Join-Path $destino 'RESULTADO_CONTORNOS_INTERNOS_19_8.json')
        Write-Host 'SIMULACAO (MOVIMENTO + RASTER + CAMADAS/ROTA) PASS sintetico; controle fisico bloqueado.' -ForegroundColor Green
        return
    }
    if ($SomenteRefino) {
        Write-Host 'REFINO: curvas de texto R/O, raster P&B e diagnosticos da Tesoura.' -ForegroundColor Cyan
        Compilar-QtComDiagnostico
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\refino'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --refine-curves-trace-19-8-test $fixture $destino
        Checar-ResultadoQt 'Texto R/O, imagem P&B e Tesoura' (Join-Path $destino 'RESULTADO_REFINO_19_8.json')
        Write-Host 'Refino PASS nos testes sinteticos. Mouse real e imagem do cliente ainda necessarios.' -ForegroundColor Green
        return
    }
    if ($SomenteCasoReal) {
        Write-Host 'TESTE REAL: fecho duplicado Tesoura, alca central, paleta e LBRN2 arredondado.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt caso real'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\caso_real'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --scissor-closures-19-8-test $fixture $destino
        Checar-ResultadoQt 'Caso real: Tesoura/Alca/Paleta/LBRN2' (Join-Path $destino 'RESULTADO_CASO_REAL_19_8.json')
        Write-Host 'Caso real: teste sintetico PASS. Abrir arquivos do usuario e testar mouse ainda obrigatorios.' -ForegroundColor Green
        return
    }
    if ($SomenteCamadas) {
        Write-Host 'FOCO: Ctrl+U desagrupar, camadas e modo visual Linha/Fill.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt de camadas'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\camadas'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --layers-ungroup-19-8-test $fixture $destino
        Checar-ResultadoQt 'Camadas e Desagrupar' (Join-Path $destino 'RESULTADO_CAMADAS_19_8.json')
        Write-Host 'Camadas e Desagrupar PASS automatizado; teste manual ainda necessario.' -ForegroundColor Green
        return
    }
    if ($SomenteInteracao) {
        Write-Host 'FOCO: texto direto na mesa, escala e rotacao por alcas.' -ForegroundColor Cyan
        $textSource = Join-Path $fontes 'tests\text_alignment_19_8_standalone.cpp'
        $textExecutable = Join-Path $build 'text_alignment_19_8_standalone.exe'
        $boostJsonSource = Join-Path $fontes 'src\boost_json_impl.cpp'
        & $gpp -std=c++20 -O2 -Wall -Wextra $textSource $boostJsonSource -o $textExecutable
        Checar-Saida 'Compilacao C++ do texto editavel e alinhamento'
        & $textExecutable
        Checar-Saida 'Teste C++ de texto e alinhamento'
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt de interacao'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\interacao'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --raster-object-19-8-test $fixture $destino
        Checar-ResultadoQt 'Imagem editavel salva no .alaser v5' (Join-Path $destino 'RESULTADO_IMAGEM_OBJETO_19_8.json')
        & $app --inline-transforms-19-8-test $fixture $destino
        Checar-ResultadoQt 'Texto e transformacoes' (Join-Path $destino 'RESULTADO_TEXTO_TRANSFORMACOES_19_8.json')
        & $app --text-format-19-8-test $fixture $destino
        Checar-ResultadoQt 'Painel de formatacao de texto' (Join-Path $destino 'RESULTADO_FORMATACAO_TEXTO_19_8.json')
        & $app --text-fill-align-19-8-test $fixture $destino
        Checar-ResultadoQt 'Texto reeditavel, Fill vazado, alinhamento e zoom' (Join-Path $destino 'RESULTADO_TEXTO_FILL_ALINHAMENTO_19_8.json')
        Write-Host 'Texto/alcas PASS automatico; mouse fisico ainda obrigatorio.' -ForegroundColor Green
        return
    }
    if ($SomenteEdicao) {
        Write-Host 'TESTE FOCADO: linha continua, selecao por area e atalhos de edicao.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6
        Checar-Saida 'Compilacao Qt de edicao'
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\editor_multiferramentas'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --editor-multitool-19-8-test $fixture $destino
        Checar-ResultadoQt 'Editor multiferramentas' (Join-Path $destino 'RESULTADO_EDITOR_MULTIFERRAMENTAS_19_8.json')
        Write-Host 'Editor: teste focado PASS. Validacao integral e mouse real ainda obrigatorios.' -ForegroundColor Green
        return
    }
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
        if (-not [string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
            if (-not (Test-Path -LiteralPath $ArquivoLbrn2 -PathType Leaf)) {
                throw "Arquivo LBRN2 informado nao encontrado: $ArquivoLbrn2"
            }
            & $app --lbrn2-file-19-8-test $ArquivoLbrn2 $destino
            Checar-ResultadoQt 'LBRN2 real enviado' (Join-Path $destino 'RESULTADO_LBRN2_REAL_19_8.json')
        }
        Write-Host 'Teste de importacao passou. Teste completo ainda necessario.' -ForegroundColor Green
        return
    }
    if ($SomenteTesoura) {
        # Este gate precisa incluir tanto a geometria C++ dos recortes sucessivos
        # quanto o hover real Qt; um PASS grafico isolado nao homologa a Tesoura.
        Write-Host 'TESOURA: compilando nucleo 19.7/19.8 e regressao GUI Qt, offline.' -ForegroundColor Cyan
        & $cmake --build $build --parallel 2 --target alencar_test7_qt6 alencar_test19_7_scissors_layers alencar_test19_8_scissors_span
        Checar-Saida 'Compilacao Qt e nucleos Tesoura'
        foreach ($nome in @('alencar_test19_7_scissors_layers', 'alencar_test19_8_scissors_span')) {
            $executavel = Join-Path $build ($nome + '.exe')
            if (-not (Test-Path -LiteralPath $executavel -PathType Leaf)) { throw "Teste da Tesoura ausente: $executavel" }
            Write-Host "TESTANDO: $nome" -ForegroundColor Cyan
            & $executavel
            Checar-Saida "Regressao C++ $nome"
        }
        $app = Join-Path $build 'alencar_test7_qt6.exe'
        $fixture = Join-Path $fontes 'fixtures\web_nodes_normalized.csv'
        $destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\tesoura_foco'
        New-Item -ItemType Directory -Path $destino -Force | Out-Null
        & $app --edit-tools-19-7-test $fixture $destino
        Checar-ResultadoQt 'Tesoura - hover GUI' (Join-Path $destino 'RESULTADO_TESTE_19_7_GUI.json')
        Write-Host 'Tesoura: C++ 19.7 + C++ 19.8 + GUI Qt PASS focado. Validacao manual e suite completa ainda obrigatorias.' -ForegroundColor Green
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
    if (-not [string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
        if (-not (Test-Path -LiteralPath $ArquivoLbrn2 -PathType Leaf)) {
            throw "Arquivo LBRN2 informado nao encontrado: $ArquivoLbrn2"
        }
        & $app --lbrn2-file-19-8-test $ArquivoLbrn2 $dirImportacao
        Checar-ResultadoQt 'LBRN2 real enviado' (Join-Path $dirImportacao 'RESULTADO_LBRN2_REAL_19_8.json')
    }
    $dirDesenho = Join-Path $destino 'desenho'
    New-Item -ItemType Directory -Path $dirDesenho -Force | Out-Null
    & $app --drawing-tools-19-8-test $fixture $dirDesenho
    Checar-ResultadoQt 'Texto e Lapis Qt' (Join-Path $dirDesenho 'RESULTADO_TEXTO_LAPIS_19_8.json')
    $dirEditor = Join-Path $destino 'editor_multiferramentas'
    New-Item -ItemType Directory -Path $dirEditor -Force | Out-Null
    & $app --editor-multitool-19-8-test $fixture $dirEditor
    Checar-ResultadoQt 'Editor: linha continua e selecao por area' (Join-Path $dirEditor 'RESULTADO_EDITOR_MULTIFERRAMENTAS_19_8.json')

    $dirSim = Join-Path $destino 'simulacao'
    New-Item -ItemType Directory -Path $dirSim -Force | Out-Null
    $dirDrawing = Join-Path $destino 'desenho_uniao'
    New-Item -ItemType Directory -Path $dirDrawing -Force | Out-Null
    & $app --draw-join-19-8-test $fixture $dirDrawing
    Checar-ResultadoQt 'Desenho por arraste e Auto-Join 19.8' (Join-Path $dirDrawing 'RESULTADO_DESENHO_UNIAO_19_8.json')
    & $app --text-format-19-8-test $fixture $dirDrawing
    Checar-ResultadoQt 'Formatacao de texto Windows 19.8' (Join-Path $dirDrawing 'RESULTADO_FORMATACAO_TEXTO_19_8.json')
    & $app --text-fill-align-19-8-test $fixture $dirDrawing
    Checar-ResultadoQt 'Texto reeditavel, Fill vazado, alinhamento e zoom' (Join-Path $dirDrawing 'RESULTADO_TEXTO_FILL_ALINHAMENTO_19_8.json')
    & $app --simulation-motion-19-8-test $fixture $dirSim
    Checar-ResultadoQt 'Simulacao cinemática 19.8' (Join-Path $dirSim 'RESULTADO_PREVIA_MOVIMENTO_19_8.json')
    & $app --simulation-raster-19-8-test $fixture $dirSim
    Checar-ResultadoQt 'Simulacao Fill e bitmap P&B 19.8' (Join-Path $dirSim 'RESULTADO_PREVIA_RASTER_19_8.json')
    & $app --layer-route-19-8-test $fixture $dirSim
    Checar-ResultadoQt 'Configuracao por camada e rota 19.8' (Join-Path $dirSim 'RESULTADO_CAMADA_ROTA_19_8.json')
    & $app --route-nesting-19-8-test $fixture $dirSim
    Checar-ResultadoQt 'Contornos internos primeiro 19.8' (Join-Path $dirSim 'RESULTADO_CONTORNOS_INTERNOS_19_8.json')
    $dirInteracao = Join-Path $destino 'interacao'
    New-Item -ItemType Directory -Path $dirInteracao -Force | Out-Null
    & $app --inline-transforms-19-8-test $fixture $dirInteracao
    Checar-ResultadoQt 'Texto direto e alcas' (Join-Path $dirInteracao 'RESULTADO_TEXTO_TRANSFORMACOES_19_8.json')
    $dirCamadas = Join-Path $destino 'camadas'
    New-Item -ItemType Directory -Path $dirCamadas -Force | Out-Null
    & $app --layers-ungroup-19-8-test $fixture $dirCamadas
    Checar-ResultadoQt 'Camadas e Desagrupar 19.8' (Join-Path $dirCamadas 'RESULTADO_CAMADAS_19_8.json')
    $dirCasoReal = Join-Path $destino 'caso_real'
    New-Item -ItemType Directory -Path $dirCasoReal -Force | Out-Null
    & $app --scissor-closures-19-8-test $fixture $dirCasoReal
    Checar-ResultadoQt 'Regressao caso real Tesoura/Alca/Paleta/LBRN2' (Join-Path $dirCasoReal 'RESULTADO_CASO_REAL_19_8.json')
    $refino = Join-Path $destino 'refino'
    New-Item -ItemType Directory -Path $refino -Force | Out-Null
    & $app --refine-curves-trace-19-8-test $fixture $refino
    Checar-ResultadoQt 'Curvas de texto, P&B, coleta Tesoura' (Join-Path $refino 'RESULTADO_REFINO_19_8.json')
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
