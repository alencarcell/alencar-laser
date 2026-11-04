# Alencar Laser 19.9 — Perfilamento Qt e teste A/B Bézier real
Data: 04/11/2026. Branch: fix/curvas-tesoura-preflight-19-8-20261030. PR #13 draft. Issue #18.

## Motivação / baseline real

A sessão do usuário com ampulheta.alaser (03/11/2026) registrou 2.134 contornos Bézier, paint_p95_ms=496.693, input_to_paint_p95_ms=452.9751, hit_test_p95_ms=0.0061, rebuild_p95_ms=4.1371 e RSS de 155.820.032 bytes. Isso aponta para pintura, mas a captura agregava diferentes gestos. NÃO declarar ganho usando um único P95 ou comparando a sessão interativa com um benchmark automatizado de frames completos.

## Implementação, sem simplificar a geometria

- qt6_test6.cpp: operações de seleção nativa e troca de camada usam updateNativeSelection199(). Por padrão, apenas o caminho selecionado é reconstruído, com estilo aplicado aos itens anterior/novo. Construção global reservada para criação integral da cena e operações estruturais.
- A Tesoura chamava createNativeItems() seguido de updateNativePaths(true); a segunda construção total era redundante e foi eliminada.
- Teste sintético GUI verifica que seleção não dispara reconstrução total, não reconstrói mais de uma curva selecionada e não altera caminhos não selecionados.
- Diagnóstico: native_full_path_builds, native_selective_path_builds, native_full_refresh_p95_ms, native_selective_refresh_p95_ms, background_p95_ms, foreground_p95_ms, contagem de amostras e native_legacy_selection_rebuild.
- Perfil de foreground usa RAII para registrar mesmo quando drawForeground() retorna antes. Nenhum dado de geometria é exportado.
- Chave A/B somente para operações de seleção: ALENCAR_NATIVE_LEGACY_SELECTION_REBUILD_19_9=1 ativa o comportamento antigo nessa operação. Por padrão usa o modo seletivo; nenhuma mudança no documento canônico.

## Benchmark automatizado e offline com arquivo real

- Opção executável Qt: --native-paint-bench-19-9-test [copia.lbrn2] [diretorio_relatorio].
- Runner Windows: SCRIPTS/BENCHMARK_BEZIER_REAL_19_9.ps1. Seleciona o arquivo ou recebe -ArquivoLbrn2; cria cópia temporária, verifica SHA256 antes/depois, apaga cópia e deixa apenas JSON de métricas.
- Qt importa a cópia experimental em memória, verifica contornos, enquadra mesa 400 × 400 mm, aquece a viewport e coleta 24 repaints síncronos. Em seguida alterna seleção/desseleção do primeiro contorno por 12 ciclos.
- JSON RESULTADO_PERFORMANCE_BEZIER_REAL_19_9.json: contornos, paint P50/P95/P99, foreground/background P95, tempo de 12 seleções, construções globais vs seletivas, dimensões viewport/DPR, versão Qt e flags de integridade/SHA256.
- -ModoLegado ativa o modo antigo de seleção; -SemCompilar evita recompilação depois do primeiro PASS.
- Em ambos os modos: machine_commands_sent=false, fabrication_enabled=false, source_sha256_unchanged=true e geometry_unchanged=true são obrigatórios.

## Procedimento de validação no Windows

Fechar a candidata e manter o laser desconectado. Antes de atualizar, conferir git status --short e preservar todas as edições locais. Não executar pull se houver alterações locais desconhecidas.

Comandos, em sequência, no PowerShell:

    cd C:\ProjetoAlencarLaser
    git status --short
    git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
    git rev-parse --short HEAD
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteBezierNativoUI

Depois do PASS, com o mesmo arquivo LightBurn original, sem mudar DPI/resolução/aplicativos abertos:

    powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\BENCHMARK_BEZIER_REAL_19_9.ps1" -SemCompilar
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\BENCHMARK_BEZIER_REAL_19_9.ps1" -ModoLegado -SemCompilar

Pode usar -ArquivoLbrn2 com caminho nas duas chamadas para evitar seleção manual. Repetir três pares alternados para reduzir efeitos de cache; comparar apenas medições com mesmo arquivo, viewport e ambiente. Os relatórios são gravados em RELATORIOS/AUDITORIAS/GITHUB_19_8/BEZIER_PERFORMANCE_19_9/ com diretórios datados por modo.

Gates: PASS do teste sintético Qt na nova revisão, e na ampulheta real 2.134 contornos, geometria e SHA256 preservados, sem comandos físicos. No modo otimizado espera-se native_full_path_builds_delta=0 na sequência de seleção; no modo legado esse valor deve ser positivo. O baseline anterior de paint P95=496,7 ms NÃO é comparável diretamente ao benchmark sincronizado sem um roteiro equivalente.

## Riscos, medição e reversão

O código do benchmark ainda não foi recompilado/testado no Windows. Esta sessão não dispõe de Qt 6 Widgets. GitHub Actions mostra falhas sem logs recuperáveis, portanto sem PASS remoto. Não declarar fluidez homologada. Se uma regressão ocorrer, manter a branch em rascunho e preservar o executável aprovado; o opt-out de seleção antigo é temporário por variável ambiente de sessão, sem reconstrução do projeto canônico.

O arquivo LightBurn real, os relatórios privados e a geometria da ampulheta NÃO foram adicionados ao repositório.
