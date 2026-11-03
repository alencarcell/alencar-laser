# Alencar Laser 19.9 — Camadas Bézier e custo de pintura (03/11/2026)

**Escopo:** branch experimental `fix/curvas-tesoura-preflight-19-8-20261030`, PR #13 (draft), issue #18.
**Regra:** não atualizar `main`, `APLICATIVO`, instalador homologado nem controlar hardware. Trabalhar com **cópia** do arquivo real. Não há execução em segundo plano.

## Evidência original do Windows

Arquivos enviados: `ampulheta.alaser`, `Alencar_Laser_Diagnostico_TESTE_19_7(20261103-210516).json`, `Alencar_Laser_Previa_19_8(9).json`.

- `.alaser` v5: 2.134 contornos Bézier nativos, 17.947 âncoras; a geometria é preservada no arquivo original, que não deve ser reescrito pela auditoria.
- Metadados de camada: `design/bezier.json` usa `layerId=1` em 2.130 contornos e `layerId=0` em 4; `design/layers.json` registra apenas camada 0. A prévia anterior aplicou 2.130 fallbacks.
- Diagnóstico Qt Windows 6.12.0: `paint_p50_ms=179.0366`, `paint_p95_ms=496.693`, `paint_p99_ms=527.2483`, `paint_max_ms=663.1266`, `input_to_paint_p95_ms=452.9751`; `hit_test_p95_ms=0.0061`; `rebuild_p95_ms=4.1371` (apenas quatro reconstruções); `rss_bytes=155820032`. Não inferir ganhos de desempenho sem condições comparáveis.
- Prévia offline: `native_bezier_missing_layer_fallbacks=2130`, `accel_estimated_seconds=956.4`; não é tempo físico nem parâmetro de produção.

## Implementação nesta branch — ainda NÃO homologada no Windows

1. `src/editor_tools.hpp`: adiciona `Layer.parametersVerified`, com persistência `editor-meta-1` e leitura de arquivos legados (valor padrão `true` para não redefinir camadas anteriores).
2. `src/integrated_project_edit.hpp`: `registerNativeLayerReferences` cadastra IDs de CutIndex ausentes como camadas editoriais, `parametersVerified=false`, sem alterar `CurveDocument`; `newEditorStateForNativeLightBurn` marca como não verificadas TODAS as camadas importadas; `editorStateForIntegratedProject` reconstitui metadados ausentes apenas em memória após abrir um v5 antigo.
3. `src/preview_native_19_8.hpp`: separa `missingLayers` de `unverifiedLayerParameters`. Não trata uma camada apenas cadastrada como se tivesse dados de potência/velocidade/Fill comprovados.
4. `qt/preview_dialog_19_8.hpp`: continua alertando sobre parâmetros não verificados e exporta `native_bezier_unverified_layer_parameters`.
5. `qt/qt6_test6.cpp`: conecta cadastro ao importador experimental, elimina a **construção DUPLICADA** de `QPainterPath` durante `createNativeItems`, e amplia o teste `--native-import-gui-19-9-test` para validar CutIndex 1/2, campo de verificação, roundtrip ZIP e reparo de metadados legados sem modificar curva.

**Limite de segurança:** os placeholders não recuperam parâmetros LightBurn de máquina. `speedMmMin`, `powerPercent`, `mode`, `lineIntervalMm` e `passes` continuam *virtuais*. Mesmo com `missingLayers=0`, esperar `unverifiedLayerParameters>0` e aviso na prévia. A operação física permanece bloqueada.

## Gates para Windows — NÃO executar automaticamente sem operador

Na branch experimental, com alterações locais salvas e a máquina desconectada:

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteBezierNativoUI
```

O gate precisa imprimir `BEZIER NATIVO GUI 19.9: PASS sintético` e gravar `RELATORIOS/AUDITORIAS/GITHUB_19_8/native_gui_19_9/RESULTADO_IMPORTACAO_NATIVA_GUI_19_9.json`.
Inspecionar `layer_ids_preserved`, `unverified_parameters_preserved` e `legacy_missing_layers_repaired_in_memory` iguais a `true`.

**Após PASS:** abrir uma cópia da ampulheta no modo experimental e gerar dois novos JSONs (prévia + diagnóstico), com mesmo zoom, tamanho de janela e gestos equivalentes. Esperado: `missing_layer_fallbacks=0`, `unverified_layer_parameters=2134` para nova importação do LightBurn (camada 0 também não possui parâmetros de produção confirmados). No antigo `.alaser` já salvo, o reparo em memória deve sinalizar **pelo menos 2130** curvas com parâmetros não verificados; não exigir 2134 porque o metadado legado da camada 0 era previamente declarado.

## Critérios antes de declarar melhoria

- A geometria nativa deve manter 2.134 contornos e identidade de âncoras/alças/IDs/ordem do arquivo original, salvar em **novo** `.alaser`, reabrir e comparar.
- Fazer três rodadas com condições iguais; comparar `paint_p50/p95/p99`, `input_to_paint_p95`, memória e número de amostras. A alteração de `createNativeItems` remove trabalho duplicado **na criação da cena**, não demonstra redução do custo de pintura a cada frame. Não alegar velocidade maior antes da evidência.
- Preservar undo/redo, seleção, Tesoura, modo de desenho, ausência de linhas fantasma e integridade do arquivo LightBurn.
- Todo resultado após esta publicação está **pendente da validação Windows**. GitHub Actions aparece com failures e logs indisponíveis nesta sessão; isso não equivale a compilação validada nem identifica erro de compilação.

## Reversão

As alterações estão isoladas em commits da branch PR #13. A `main`, `APLICATIVO`, backup e arquivo da ampulheta não foram modificados. Se o teste GUI falhar, manter a candidata anterior, preservar o log e diagnosticar a primeira falha; **não mesclar**.
