# Experimento de repintura localizada no arraste — Alencar Laser 19.9

**Data:** 05/11/2026. **Estado:** implementação experimental, sem homologação Qt Windows/mouse e sem promoção para produção.

## Problema observado

O diagnóstico real Qt 6.12.0 enviado pelo operador registrou 195 eventos retidos, movimentação de seleções com ~558–575 objetos, `paint_p95_ms=208.7439`, `paint_max_ms=663.3852`, `viewport_paint_bbox_fraction_p95=1`, `input_to_paint_p95_ms=58.2393`. Estes valores pertencem à sessão real TESTE_19.8; não são o A/B de cache. A causa do P95 alto ainda precisa ser confirmada.

## Alteração no Qt experimental

No método `QtEditorLab::mouseMoveEvent()`, quando `shapeDragging_` está ativo, o editor anteriormente chamava `viewport()->update()` incondicionalmente a cada movimento, solicitando repintura de toda a viewport até para o contorno e alças da seleção. Os `QGraphicsItem`s móveis já fazem invalidação da própria geometria pelo Qt.

A nova política, **somente quando** a variável de processo `ALENCAR_LOCAL_DRAG_OVERLAY_19_9=1`, calcula as caixas em pixels das alças/contorno anteriores e posteriores, com margem de segurança, e pede `viewport()->update(QRegion(antiga).united(QRegion(nova)))`. Se geometria for inválida, ocorre **fallback para repintura completa**. O comportamento anterior permanece o padrão quando a variável não existe.

Novas métricas locais em `captureDocument().performance`: `drag_overlay_local_experiment_19_9` (bool), `drag_overlay_local_requests_19_9`, `drag_overlay_full_fallbacks_19_9`. Não registram coordenadas, nomes, projetos ou dados da máquina. `shapePreviewDelta_`, `QGraphicsItemGroup`, serialização, histórico e emissão física não foram alterados.

## Gate A/B sintético — Windows, sem laser

Atualizar o código da branch experimental e executar, com o aparelho desconectado:

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_DRAG_OVERLAY_19_9.ps1"
```

O script compila `alencar_test7_qt6.exe` no build **isolado** `DESENVOLVIMENTO/FONTE/build_WINDOWS_DRAG_AB_19_9` (separado do editor em `build_WINDOWS_GIT_19_8`, que pode permanecer aberto), chama a mesma regressão Qt de arraste agrupado (40 objetos) primeiro com a flag OFF, depois ON, e gera `RELATORIOS/AUDITORIAS/GITHUB_19_8/DRAG_OVERLAY_19_9/COMPARACAO_DRAG_OVERLAY_19_9.json`. Exige `PASS` da regressão de geometria, grupo transitório, Undo/Redo, e valida que o modo ON realmente solicitou repintura local. **Ainda não prova melhora de P95 real nem fidelidade em seleções 575.**

Para rodar novamente com executável já compilado, opcionalmente acrescente `-SemCompilar` ao comando.

## Gate manual — duas execuções separadas

Com **cópia** do mesmo projeto, laser desconectado e sem alterações em produção:

```powershell
# Referência: política anterior
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -UsarBuildDragAB
# Feche a primeira sessão antes de iniciar a outra.
# Candidata: MESMO executável A/B, com flag temporária ligada
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -UsarBuildDragAB -TestarRepinturaLocal
```

ATENÇÃO: use `-UsarBuildDragAB` **nas duas sessões** para evitar comparar o executável antigo com o novo. Por segurança, o launcher recusa `-TestarRepinturaLocal` desacompanhado de `-UsarBuildDragAB`. O binário A/B precisa estar compilado e validado previamente.

Repetir seleção por área, arraste de centenas de formas, zoom e pan, Tesoura, Undo e Redo nas duas sessões. Registrar P95/P99 de pintura, latência entrada→pintura, porcentagem da viewport repintada, contadores locais, qualidade visual das alças, integridade/Undo e RSS. Comparar somente sessões com a mesma origem, zoom, duração aproximada e gestos. A GUI não automatiza esses gestos; resultados do mouse continuam pendentes.

## Critérios de decisão e rollback

- Se houver artefatos, rastro de moldura, seleção incorreta, erro de Undo, compilação/teste falho, **desligar imediatamente**: iniciar novamente sem `-TestarRepinturaLocal`. Nenhum arquivo precisa ser revertido para isso.
- Mesmo após PASS sintético, a política anterior continua como padrão. Não migrar para `main` ou habilitar máquina sem regressão visual e autorização separada.
- `RELATORIOS/` continua ignorada pelo Git. Resultados não são publicados automaticamente; GitHub recebe apenas comentário técnico e, futuramente, relatório filtrado após revisão.
- A chamada `viewport()->update()` de outras operações ainda existe; não afirmar que esta flag sozinha elimina todo repaint completo.

## Continuidade

Ver Issue #18 (desempenho) e Issue #20 (auditoria). Branch `fix/curvas-tesoura-preflight-19-8-20261030` permanece experimental. O teste Qt do arraste existente foi ampliado para validar contadores de invalidação local quando a flag está ligada.


## 05/11/2026 — Correção do bloqueio do linker no Windows

Uma captura do operador mostrou a falha `ld.exe: cannot open output file alencar_test7_qt6.exe: Permission denied` durante `[2/2] Linking CXX executable`, após warnings não fatais do compilador. O cenário é **compatível com o executável aberto/bloqueado**, mas a causa exata não foi confirmada por PID. O A/B não executou, e não há PASS novo.

O hotfix do script `COMPARAR_DRAG_OVERLAY_19_9.ps1` (commit `22b92d5`) evita compartilhar o alvo binário com o editor: agora compila e executa uma **cópia isolada** em `build_WINDOWS_DRAG_AB_19_9`, diretório ignorado pelo Git. Não força encerramento de processos e não altera a versão experimental aberta. Se a própria cópia A/B estiver em uso, o preflight recusa o teste e informa o processo. O modo `-SemCompilar` também recusa executável mais antigo que a fonte Qt.

Para retomar, após `git pull --ff-only`, repita o comando sem `-SemCompilar`. A primeira compilação isolada pode recompilar o alvo inteiro. Somente considerar o gate aprovado após aparecer `PASS` nos dois modos e existir o JSON de comparação; não tratar warnings como falha.



## 05/11/2026 — Telemetria OFF/ON em pastas separadas

A captura do operador mostra a execução do launcher ON com aviso de flag ativa, seguida do prompt PowerShell. A versão Qt interativa usa `app.exec()`; portanto, o retorno do prompt implica que o processo inicial terminou (mas não comprova por quanto tempo a janela ficou visível nem se os gestos foram testados).

Foi identificado um risco no launcher anterior: OFF e ON passavam a mesma pasta `RELATORIOS/AUDITORIAS/GITHUB_19_8/sessao` para `--interactive`, cujo JSON automático tem sempre o nome `SESSAO_INTERATIVA_TESTE_6.json`. Assim, uma sessão podia substituir o arquivo da outra.

O launcher experimental agora cria pastas únicas `DRAG_AB_19_9_OFF_<data>_<id>` e `DRAG_AB_19_9_ON_<data>_<id>`, informa duração e código de saída, e mostra o caminho do JSON automático. Cada pasta permanece local em `RELATORIOS` (ignorada pelo Git). Para uma comparação fiel, execute novamente OFF e ON com o launcher atualizado e o mesmo projeto/copias equivalentes; confirme os dois caminhos distintos. A saída automática não equivale a comprovação de melhora visual, e os diagnósticos adicionais podem exigir exportação manual.


## 05/11/2026 — Comparador observacional de sessões GUI

As imagens do operador comprovam duas sessões usando o mesmo executável experimental:
- OFF: 98,9 segundos, encerramento 0, telemetria automática salva em diretório `DRAG_AB_19_9_OFF_*`.
- ON: 69,1 segundos, encerramento 0, telemetria automática salva em diretório `DRAG_AB_19_9_ON_*`.
- Ambas com máquina desconectada. Há seleção de 575 contornos/13.415 nós em capturas, mas escala/posicionamento visuais não são idênticos; duração de uso NÃO é indicador de velocidade.

O código Qt da telemetria automática `SESSAO_INTERATIVA_TESTE_6.json` contém `paint_count`, `paint_p95_ms`, `input_to_paint_samples`, `input_to_paint_p95_ms`, `memory.rss_bytes`, `memory.peak_rss_bytes`, versão Qt, plataforma, DPR e sistema operacional. **Não** exporta P99, `viewport_paint_bbox_fraction_p95`, contadores da flag `drag_overlay_local_requests_19_9`, nem métricas de arraste agrupado. Esses campos fazem parte de outro formato de diagnóstico, e não podem ser inventados na comparação.

Criado `SCRIPTS/COMPARAR_SESSOES_GUI_DRAG_REAL_19_9.ps1` (commit `0f03767`; validação da contagem no commit `709f05d`). O comparador é **somente leitura** dos últimos diretórios OFF e ON e:
- Compara P95 de pintura, P95 de entrada-até-pintura, suas contagens de amostras, RSS final/pico e ambiente Qt/DPR/OS.
- Calcula delta percentual `((OFF-ON)/OFF)*100` e deixa claro que o valor é observacional e confunde se gestos, zoom e carga forem diferentes.
- Alerta se amostras escassas ou muito diferentes entre sessões, sem converter observações em homologação.
- Gera resumo JSON exclusivamente local em `RELATORIOS/AUDITORIAS/GITHUB_19_8/DRAG_OVERLAY_19_9/`, sem nomes/caminhos do usuário, sem cópias dos JSONs e sem `git push`.

Após `git pull --ff-only`, executar:

```powershell
cd C:\ProjetoAlencarLaser
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_SESSOES_GUI_DRAG_REAL_19_9.ps1"
```

Os dois editores podem permanecer fechados durante essa leitura. Não publicar com `-Publicar`: este formato não faz parte da allowlist atual da Central de Auditoria. Primeiro confirmar os números locais; depois registrar síntese técnica na Issue #18, mantendo P95 real e integridade de objetos como gates separados.


## 06/11/2026 — Protocolo A/B automatizado com 575 contornos

O A/B manual realizado em 05/11 foi inconclusivo: OFF paint P95 176,0124 ms (390 amostras), ON 233,938 ms (139 amostras); input-to-paint P95 OFF 38,0386 e ON 33,6162 ms. O desbalanceamento e os gestos/zoom diferentes invalidam conclusao causal.

### Implementacao do novo gate

O executavel Qt experimental passou a reconhecer `--controlled-drag-575-19-9-test <fixture_csv> <saida>`, usando **somente** a fixture fixa do projeto `DESENVOLVIMENTO/FONTE/fixtures/web_nodes_normalized.csv`, com 575 contornos e 13.415 nos. A cada execucao: janela 1180x680, zoom 1,2x, selecao dos 575, 144 eventos de mouse QTest em trajetoria deterministica, contagem de frames e verificacao do resultado canonico de cada no antes/depois, Undo e Redo. A medicao separa inicializacao/selecao dos eventos de arraste. Coleta P50/P95/P99 de paint, P95 input-to-paint, fração P95 do bounding rectangle da pintura, contadores do overlay e número de amostras. O modo ON somente e habilitado pela flag de processo `ALENCAR_LOCAL_DRAG_OVERLAY_19_9=1`; sem flag, OFF continua padrao.

O orquestrador `SCRIPTS/COMPARAR_DRAG_CONTROLADO_575_19_9.ps1` recompila o binario Qt no build A/B isolado (reaproveita e exige o PASS do teste sintetico de 40 objetos), e executa **seis processos separados** na ordem OFF–ON–ON–OFF–OFF–ON. Todas as rodadas usam **o mesmo executavel** e SHA-256 de fixture, com verificacao de Qt/DPR/viewport e nenhuma conexao fisica. Calcula medianas das tres rodadas por modo e delta porcentual de P95/P99. Recusa hardware/metricas invalidas; marca `INCONCLUSIVE_SAMPLE_IMBALANCE` se a razao minimo/maximo de amostras de pintura for inferior a 0,70.

### Comando (Windows PowerShell, laser desconectado)

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar' }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_DRAG_CONTROLADO_575_19_9.ps1"
```

Arquivos gerados somente localmente: `RELATORIOS/AUDITORIAS/GITHUB_19_8/DRAG_OVERLAY_19_9/CONTROLADO_575_*/RODADA_*` e `RESUMO_AB_CONTROLADO_575_19_9.json`. A pasta RELATORIOS e ignorada pelo Git. Nao usar `-Publicar` (esse esquema ainda nao faz parte da allowlist da Central de Auditoria). No PowerShell, a primeira compilacao pode exibir avisos nao fatais. Em caso de falha de compilacao/fixture, preservar o log; **nao declarar PASS**.

### Interpretacao

O novo A/B **foi implementado no GitHub, mas ainda nao executado no Windows**. Mesmo se seis rodadas passarem, o resultado comprova apenas o teste Qt controlado, nao a ausencia de artefatos visuais, compatibilidade de GPU, fluidez real do usuario ou exatidao de corte. O comportamento OFF permanece padrao; `main`, instalador e emissao do laser intocados. Comparacao de pixel e teste de zoom/pan/Tesoura com mouse real continuam gates de homologacao separados.


## 06/11/2026 — Modo 2: frame cacheado durante arraste

**Evidência Windows 06/11/2026**, teste controlado com 575 contornos, 13.415 nós, 144 movimentos por rodada, 3 pares OFF/ON. `ANALISAR_DRAG_CONTROLADO_575_19_9.ps1` confirmou: mediana pintura P95 OFF 29,784 ms, ON1 40,8599 ms (+37,19%); pintura P99 30,546 versus 44,891 ms (+46,96%); input→paint P95 35,1264 versus 47,1889 ms (+34,34%). Repintura bbox P95 **OFF 1,000 / ON1 0,299478...**, 432 atualizações locais no ON1, zero fallbacks, razão de amostras 1,000. O ON1 reduziu área repintada mas piorou os tempos; gate REGRESSION_OBSERVED_DO_NOT_PROMOTE registrado na Issue #18.

**Candidato técnico (não causa comprovada):** durante o drag ON1, `selectionQuadScene()` é chamado antes/depois do movimento e no foreground. Sem frame orientado válido, chama `selectedBounds()`, que percorre todos os contornos e nós selecionados. Essa varredura pode ocorrer repetidamente em cada atualização de mouse/pintura. A regressão também pode envolver overhead Qt `QRegion`/clipping; somente o teste irá separá-los.

**Implementação isolada (sem alterar default):** em `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp`, `ALENCAR_LOCAL_DRAG_OVERLAY_19_9` passa a aceitar 0/ausente = OFF legado, 1 = ON1 QRegion com frame recalculado, **2 = ON2 QRegion com frame de seleção cacheado**. ON2 captura o quad uma única vez em `beginShapePreview()` antes de `shapeDragging_` e aplica `shapePreviewDelta_` durante os movimentos, inclusive `drawForeground`/alças; limpa o cache em `clearShapePreview()`. Muda somente a prévia em memória; geometria canônica, Undo/Redo e laser não são alterados. Os novos relatórios Qt incluem `overlay_cached_bounds_enabled`/`drag_overlay_cached_experiment_19_9`.

O orquestrador `COMPARAR_DRAG_CONTROLADO_575_19_9.ps1` aceita `-ModoOtimizado 2`, mantendo modo 1 como default para reproduzir a versão original. Esse teste irá repetir as seis rodadas controladas e validar que OFF não habilitou o cache e ON2 habilitou. Os resultados são salvos em uma nova pasta `RELATORIOS/.../CONTROLADO_575_*`; nenhum dado bruto vai para o GitHub.

### Próximo teste de segurança e desempenho no Windows

```powershell
cd C:\ProjetoAlencarLaser
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
if ($LASTEXITCODE -ne 0) { throw 'Falha na atualização' }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_DRAG_CONTROLADO_575_19_9.ps1" -ModoOtimizado 2
```

**Ainda não compilado/executado:** não afirmar PASS nem melhoria. O novo teste deve validar seis rodadas, integridade de 575 contornos, Undo/Redo, cache ligado em ON2, P95/P99, equilíbrio de amostras e zero hardware. Mesmo que P95 melhore, homologação visual com ratros de moldura, zoom/Tesoura e GPU ainda fica pendente. Não utilizar `-Publicar` nem promover para `main`.


## 06/11/2026 — ON2 reprovado e guarda experimental ON3

Captura do operador apos compilar e executar `-ModoOtimizado 2`: seis rodadas automatizadas sobre 575 contornos, 13.415 nos, 144 movimentos, equilibrio de amostras 1,000, teste de integridade Qt 40 objetos PASS, hardware desconectado. **REGRESSION_OBSERVED_DO_NOT_PROMOTE**. Medianas: pintura P95 OFF **16,4751 ms** vs ON2 **30,4121 ms** (+84,59% ON); pintura P99 **17,8071** vs **32,2839 ms** (+81,30%); entrada→pintura P95 **19,8845** vs **32,9958 ms** (+65,94%). Bbox P95 OFF 1,000 / ON2 ~0,300; 432 requests localizadas ON2; 0 fallbacks. A comparacao OFF-ON2 e pareada e controlada; os valores OFF de outra execucao OFF-ON1 nao sao diretamente comparaveis por variacao temporal.

**Diagnostico:** caching dos limites de selecao reduziu custos de varredura potencialmente presentes no modo 1, mas **nao resolveu** a regressao; as medições não isolam se o custo remanescente vem de clipping QRegion, chamadas extras, desenho do foreground, plataforma raster ou outros fatores. NAO presumir causa comprovada apenas com a area repintada.

**Proxima protecao experimental (nao adotada em producao):** fonte Qt recebeu modo `ALENCAR_LOCAL_DRAG_OVERLAY_19_9=3` (commit `e95ca46`) com **guarda de densidade**. Em arraste com `shapePreviewItems_.size() >= 128`, usa a estrategia legada de repaint completo; com ate 127 items, usa QRegion e quadro cacheado como ON2. Adiciona diagnosticos `overlay_dense_guard_enabled` e `overlay_dense_legacy_updates`. Requisicoes ON3 de repintura local devem ser zero para 575 e os updates legados >= 144. A variavel ausente segue como OFF; 1/2 continuam referencias experimentais.

O script `COMPARAR_DRAG_CONTROLADO_575_19_9.ps1` aceita `-ModoOtimizado 3` (commit `e21f960`) e valida fallback em todas as rodadas ON3; testa a mesma geometria, Undo/Redo e ausencia de comandos laser. **Ainda não compilado ou executado no Windows.** Nao tratar volta ao modo legado como ganho comprovado ou algoritmo adaptativo otimizado. Mesmo se PASS, o proximo teste deve cobrir selecoes pequenas com imagens e Undo/Redo, e continuar com QA visual antes de qualquer promocao.

Com o editor fechado e maquina laser desconectada:
```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
if ($LASTEXITCODE -ne 0) { throw "Falha na atualizacao" }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_DRAG_CONTROLADO_575_19_9.ps1" -ModoOtimizado 3
```
O resultado fica somente em `RELATORIOS`, ignorado pelo Git; nao e preciso publicar nem conectar hardware. Mantem main, APLICATIVO, instalador e controle fisico intocados.


## 06/11/2026 — ON3 confirmado no Windows e instrumentacao por fases de pintura

Captura do operador após atualizar e rodar `ANALISAR_DRAG_CONTROLADO_575_19_9.ps1`: o analisador reconhece `Modo ON avaliado: 3` e classifica o teste como **`EQUIVALENT_REPAINT_POLICY_NO_SPEEDUP_CLAIM`**. Nas seis rodadas, OFF e ON3 tiveram bbox P95=1,0; ON3 apresentou 0 pedidos localizados e `dense_legacy_updates=144` em cada uma das três rodadas ON. O script confirmou `Balanceamento de amostras: 1,0000`. É **PASS do fallback de segurança**, não ganho de desempenho.

Os dados anteriores (pintura P95 OFF=26,5319/ON3=16,2325 ms; P99 OFF=28,6906/ON3=19,6974 ms) permanecem observações de execuções equivalentes quanto à política de repaint; **não** podem ser atribuídos a uma otimização de pintura ON3 para 575 objetos.

### Nova medição: fases internas Qt (sem mudança na estratégia de pintura)

Na função `controlledDenseDrag199Test()`, os vetores `backgroundMs` e `foregroundMs`, que já são instrumentados por `PaintPhaseProbe199`, passaram a ser limpos junto com `paintMs` imediatamente antes do primeiro dos 144 movimentos. O JSON de cada rodada agora inclui `background_samples`, `background_p95_ms`, `background_p99_ms`, `foreground_samples`, `foreground_p95_ms` e `foreground_p99_ms` (commit `dd29d0e`). O orquestrador valida amostras das duas fases, guarda os números por rodada e exibe medianas P95 de fundo e foreground (commit `b95db50`). Não houve alteração de renderização, lógica geométrica ou comunicação laser.

**ATENÇÃO:** P95 do fundo, foreground e pintura total são percentis de distribuições distintas; não se somam e não devem ser subtraídos para inferir custo residual, nem provam, sozinhos, qual fase causou regressões anteriores. Esse experimento testa a distribuição temporal com a política OFF equivalente ao ON3 em seleções grandes.

Executar somente após atualizar a branch, com editor e laser desconectados:

```powershell
cd C:\ProjetoAlencarLaser
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar' }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\COMPARAR_DRAG_CONTROLADO_575_19_9.ps1" -ModoOtimizado 3
```

Essa execução recompila a cópia Qt A/B isolada, repete 3 pares com 575 contornos e salva o relatório local em nova pasta `CONTROLADO_575_*`. **Estado atual: código registrado no GitHub, ainda não compilado/testado no Windows.** A versão OFF continua padrão; ON1/ON2 não homologados; nada foi enviado para produção.
