# Alencar Laser — Medição local RAM/CPU (03/11/2026)

## Proveniência
Dois CSVs enviados pelo operador; amostragem local somente leitura do executável Windows experimental, duração aproximada de 180 segundos cada, 350 amostras ao todo. Não foi enviado telemetria de comandos por gesto associada à linha temporal; não inferir causalidade direta das operações a partir dos CSVs isolados.

## Resultados
| Métrica | Coleta 1 (174) | Coleta 2 (176) |
|---|---:|---:|
| RAM residente inicial (MB) | 82.078 | 138.738 |
| RAM residente média (MB) | 92.13 | 145.48 |
| RAM residente P95 (MB) | 125.46 | 152.42 |
| RAM residente pico (MB) | 127.699 | 194.684 |
| RAM residente final (MB) | 127.680 | 141.641 |
| Memória privada pico (MB) | 99.941 | 169.309 |
| CPU média normalizada (%) | 6.68 | 8.72 |
| CPU máxima normalizada (%) | 48.0 | 26.0 |
| Handles inicial/final | 723 / 1245 | 1091 / 1095 |

Coleta 1: RAM residente ficou ~84 MB durante os primeiros ~2 minutos; houve incremento acentuado entre os segundos ~143 e ~173 e terminou 45.6 MB acima do início. Isso pode refletir carga/edição tardia do projeto; sem eventos sincronizados, não declarar leak nem impacto causal.

Coleta 2: RAM residente ficou tipicamente ~144 MB, pico curto de 194.684 MB em t=40.8 s, retornando a ~143.9 MB em ~10 s; início/fim +2.903 MB. O pico sugere alocação temporária; este trecho **não prova vazamento contínuo**.

## Relação com diagnósticos de render anteriores
Uma sessão anterior de projeto com ~120.081 nós reportou `paint_p95_ms=1062.0717`, `paint_max_ms=1615.7267`, `hit_test_p95_ms=159.6687`. Os CSVs de RAM/CPU **não contêm** tempos de render, reconstrução de cena ou eventos de mouse sincronizados. Não foi executado A/B controlado.

Conclusão conservadora: o editor continua apresentando gargalos de responsividade que os 2 CSVs **não explicam apenas por consumo permanente de RAM**. CPU normalizada de 25% pode coexistir com gargalo de um único thread em CPU multinúcleo.

## Intervenção registrada
Commit `33da459` na PR #13: instrumentação adicional de tempo de `QtEditorLab::rebuild()`, contagem de rebuilds, latência P95/máxima de reconstrução da cena e atualização visual da seleção; métricas disponíveis no JSON de diagnóstico. **Ainda sem compilação/validação Qt Windows desse commit.** Nenhum comando físico ativado.

## Próximo protocolo
1. Preservar alterações locais; atualizar branch experimental depois de conferir `git status --short`.
2. Validar compilação Qt local e regressões de Undo/Redo e seleção.
3. Abrir cópia do mesmo projeto denso (~120 mil nós), reproduzir seleção em massa, movimentação, zoom e Tesoura de forma cronológica. Com coleta local opt-in, exportar novo JSON do painel Diagnósticos.
4. Investigar `rebuild_p95_ms`, `rebuild_max_ms`, `render_rebuild_count`, `style_apply_p95_ms`, `paint_p95_ms`, `hit_test_p95_ms` e `rss_bytes`.
5. Só depois implementar cache/invalidação incremental onde os tempos provarem o gargalo; medir antes/depois no mesmo cenário.

## Proteção
PR #13 em rascunho; sem merge na main; APLICATIVO homologado não alterado; hardware e laser desconectados/bloqueados.
