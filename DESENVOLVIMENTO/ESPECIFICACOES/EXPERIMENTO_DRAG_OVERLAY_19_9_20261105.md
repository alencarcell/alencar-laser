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

