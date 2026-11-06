# Fase 20i — Resultado real Windows (06/11/2026)

## Fonte
Captura do terminal fornecida pelo operador após rodar `TESTAR_TRANSFORM_575_20I_WINDOWS.ps1 -Pares 3`. Compilação Qt concluída; 3 pares OFF/ON com 575 objetos e 12 movimentos sintéticos por teste.

| Par | P95 OFF (ms) | P95 ON (ms) | bbox OFF P95 | bbox ON P95 | pixel hash |
|---|---:|---:|---:|---:|---|
| 1 | 35,2256 | 67,9235 | 1,0 | 0,371742... | igual |
| 2 | 50,6777 | 175,2448 | 1,0 | 0,371742... | igual |
| 3 | 16,0919 | 80,2291 | 1,0 | 0,371742... | igual |

**Decisão:** igualdade de pixels e regressão Undo/Redo aprovadas no sintético; desempenho P95 **piorou em todos os três pares**. Redução da área de atualização solicitada não demonstrou fluidez. Não habilitar `ALENCAR_TRANSFORM_LOCAL_REPAINT_20H` por padrão nem integrar à produção; flag OFF.

O ensaio não prova a causa da lentidão — Qt pode ter sobrecarga com regiões, clipping, seleção e invalidadores concorrentes; a hipótese precisa ser instrumentada antes de outra mudança. Diferentes amostras por ensaio e variação temporal exigem cautela nas comparações. O `P95` da sessão de mouse real anterior (157,4255 ms) não é comparável diretamente a este fixture sintético.

## Próximos alvos sem comprometer o editor
1. Instrumentar contagem e duração de invalidações e pintura por categoria de evento em sessão real (mouse/zoom/seleção/transformação).
2. Testar alternativas independentes que não alterem pixels nem a geometria: evitar atualizações redundantes, otimizar o custo de `drawForeground`, e verificar estratégia Qt viewport update; medir custo real.
3. Reexecutar os sete gates e usar cópias de projetos reais antes da candidata instalável.

Esta evidência altera a decisão de performance, não apaga as aprovações funcionais. Branch experimental preservada; `main`, instalador, `.alaser` e comandos físicos não alterados.
