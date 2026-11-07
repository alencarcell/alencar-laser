# Fase 20j — telemetria de repintura por interação, sem cache
Data: 07/11/2026

## Por que foi criada
A Fase 20i mostrou P95 significativamente pior nos três pares com o repaint localizado ON (575 objetos), embora a prévia estivesse visualmente equivalente. Esta branch **não tenta outra aceleração sem diagnóstico**.

## Alterações
- Nova branch `test/phase20j-event-paint-trace-20261107`, derivada de `fix/phase20h-transform-repaint-20261106`.
- Opt-in `ALENCAR_EVENT_PAINT_TRACE_20J=1` no `QtEditorLab`. A GUI classifica o último evento de input como `transform`, `shape_drag`, `marquee`, `pan`, `zoom`, `mouse_press`, `mouse_release`, `native_edit`, `raster_move`, `scissors_hover`, `mouse_move` ou `idle`. Para cada paint nos 250 ms seguintes, registra duração e fração bbox de viewport (limite 4096 por grupo; conta descartes).
- Exportação manual de diagnóstico incorpora `performance.event_paint_trace_20j` com P50/P95/P99/máximo e contadores por categoria. Não exporta coordenadas, IDs, desenhos ou caminhos do arquivo. **Atribuição best-effort ao evento recente, não prova causalidade**.
- O modo normal continua OFF; `ALENCAR_TRANSFORM_LOCAL_REPAINT_20H` deve permanecer OFF, pois regrediu no teste denso.
- Teste Qt existente de transformação central em 40 objetos agora inclui campo event_paint_trace_20j.
- `SCRIPTS/TESTAR_EVENT_PAINT_20J_WINDOWS.ps1` compila um executável Qt separado e verifica coleta, Undo/Redo e ausência de comandos físicos. `-AbrirSessao` opcional abre a GUI para recolher a exportação real pelo menu Diagnósticos.

## Comando Windows
Feche o editor de desenvolvimento e mantenha fisicamente o laser desconectado.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch test/phase20j-event-paint-trace-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha ao mudar de branch; nao force' }
git pull --ff-only origin test/phase20j-event-paint-trace-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_EVENT_PAINT_20J_WINDOWS.ps1' -AbrirSessao
```

Na janela aberta, utilizar a captura de diagnóstico no editor, operar seleção, arraste, zoom e pan e **exportar manualmente** o JSON. O script não exporta automaticamente um arquivo real. Se quiser apenas compilar/testar, remover `-AbrirSessao`.

## Gate de aprovação
- Compilação + smoke no Windows (ainda pendentes no momento do commit).
- Confirmar `event_paint_trace_20j.enabled=true`, mais de uma categoria capturada e amostras suficientes no JSON de sessão real.
- Comparar categorias e frequência com P95 global da sessão; não somar percentis.
- Identificar ponto quente de fato antes de alterar drawForeground, clipping ou política Qt.
- Repetir regressões 20f, validação manual DPI/zoom, recuperação, cópia de projeto real e instalador antes de qualquer candidata offline.
- **Nunca considerar homologação da máquina nesta fase**.

## Limites
Telemetria opcional aumenta o custo da pintura e pode distorcer tempos. Timestamps representam o último input recente, mesmo quando o Qt agrega paints de diferentes causas. P95 da bbox retangular não é a proporção de pixels efetivamente rasterizados. Os resultados não autorizam ganho de velocidade ou ativação de opções experimentais.
