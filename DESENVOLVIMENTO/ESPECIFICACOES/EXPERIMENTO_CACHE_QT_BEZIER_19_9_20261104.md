# Alencar Laser 19.9 - Experimento controlado de cache Qt para curvas Bezier
04/11/2026. PR #13 draft, issue #18. Nao altera main/APLICATIVO/instalador.

## Evidencia real em Windows

Par A/B validado com a MESMA origem .lbrn2, 2.134 contornos e SHA256 preservado:
- Otimizado: paint P95 = 563,2402 ms; 12 ciclos de selecao = 10.637,6105 ms; full path rebuilds = 0; selective = 12.
- Legado: paint P95 = 544,2971 ms; 12 ciclos de selecao = 12.369,3043 ms; full path rebuilds = 51.216; selective = 0.
- Fases background/foreground P95 (otimizado): 0,4332 / 0,0158 ms.
- Ocultando somente os itens graficos nativos na mesma cena: paint P95 = 0,9073 ms (otimizado) e 1,1423 ms (legado).
- CONCLUSAO: o desempenho do desenho/visibilidade dos 2.134 QGraphicsPathItem e gargalo forte. Esses percentis independentes nao permitem atribuir uma porcentagem exata a rasterizacao, scene indexing ou sobrecarga por item.
- Selecao com menos rebuilds melhorou 14% nesta rodada, mas o paint P95 otimizado continuou 3,5% pior. Repetir pares antes de considerar ganho consistente.

## Experimento adicionado (DESLIGADO POR PADRAO)

Em qt/qt6_test6.cpp: o teste --native-paint-bench-19-9-test reconhece a variavel de processo ALENCAR_NATIVE_DEVICE_CACHE_BENCH_19_9=1; por padrao a variavel fica ausente e os itens permanecem NoCache.

Ao ativar:
- QGraphicsItem::DeviceCoordinateCache apenas em QGraphicsPathItem nativos, estaticos, visiveis na viewport e nao selecionados.
- Protecao conservadora: tamanho maximo estimado 256 x 256 px por item e limite estimado agregado de 24 MiB.
- QPainterPath, CurveDocument, nos, handles, CutIndex, undo, hit-test e persistencia NAO mudam.
- Qt recria caches quando a transformacao muda; esse modo tem qualidade maxima na resolucao do dispositivo, diferentemente do ItemCoordinateCache que pode borrar em zoom. Ainda assim precisa medir diferencas de pixels e testar zoom no Windows.
- Fonte LightBurn e copiada para arquivo temporario, hasheada antes/depois, sem G-code, GRBL, movimento ou emissao.
- O benchmark mede paint P50/P95/P99, seleciona/desseleciona 12 vezes, mede Qt RSS e relata numero de itens cacheados e memoria cache estimada. Cria duas capturas de viewport apenas na memoria para comparar percentual de pixels com diferenca de canal >8; NAO salva imagens nem geometria.

## Novo gate unico do Windows

Script ASCII para PowerShell 5.1: SCRIPTS/COMPARAR_CACHE_BEZIER_19_9.ps1.
Seleciona UMA origem LightBurn e:
1. Compila e executa gate sintetico Qt (a menos que -SemCompilar).
2. Executa BENCHMARK_BEZIER_REAL_19_9.ps1 -SemCompilar SEM cache e COM cache com o mesmo executavel e mesmo caminho de origem.
3. Rejeita divergencia de SHA, contornos, Qt, DPR, dimensoes da viewport, amostras, integridade e modo de cache.
4. Salva JSON agregado em RELATORIOS/AUDITORIAS/GITHUB_19_8/BEZIER_PERFORMANCE_19_9/COMPARACAO_CACHE_19_9_<timestamp>.json.
5. Restaura o ambiente e nunca altera original nem codigo instalado.

Com editor fechado, maquina desconectada, e alteracoes locais preservadas:

    cd C:\ProjetoAlencarLaser
    git status --short
    git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_CACHE_BEZIER_19_9.ps1"

Nao executar git pull se houver conflito de trabalho local. Os arquivos do usuario permanecem no Windows.

## Critico: aceite e rollback

Este experimento foi somente escrito/revisado no repositorio. NAO foi compilado nem executado no Windows neste turno.

Para levar ao produto, exigir no minimo:
- PASS de build, teste sintetico, origens SHA e Bezier identicas.
- Pelo menos tres pares com a mesma origem e mesmas configuracoes de tela, sem artefatos, travamentos ou regressao de selecao.
- Menor paint P95 (ganho estatisticamente estavel); medir aumento de RSS. Se cache nao reduz paint ou utiliza memoria demais, descartar.
- Testar zoom 100, 200, 400%, pan, Tesoura, undo/redo, selecao, linhas sobrepostas e qualidade visual. Nao aceitar borrao, lacunas ou pixels fantasma.
- Manter modo desligado por padrao ate homologar. Nenhum hardware fisico deve ser ativado.

Referencia Qt 6: https://doc.qt.io/qt-6/qgraphicsitem.html (DeviceCoordinateCache versus ItemCoordinateCache), https://doc.qt.io/qt-6/qgraphicsview.html (viewport update modes).
