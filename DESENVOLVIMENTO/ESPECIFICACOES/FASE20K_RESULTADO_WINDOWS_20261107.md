# Alencar Laser — Resultado Fase 20k no Windows (07/11/2026)

## Evidência do operador
Captura do terminal do VS Code de `SCRIPTS/TESTAR_ZOOM_575_20K_WINDOWS.ps1`, branch `exp/phase20k-zoom-repaint-20261107`. Compilação Qt terminada e três pares OFF/ON de zoom sintético com 575 contornos concluídos.

| Par | P95 original OFF (ms) | P95 experimental ON (ms) | Bbox P95 OFF | Bbox P95 ON | Visual |
|---|---:|---:|---:|---:|---|
| 1 | 43,6223 | 51,6104 | 1 | 1 | true |
| 2 | 32,5454 | 68,5141 | 1 | 1 | true |
| 3 | 33,971 | 63,8139 | 1 | 1 | true |

Os tempos ON pioraram nos três pares. Os hashes de screenshot coincidiram; os testes verificaram integridade de geometria e Undo. Isso não é evidência de aceleração, e não implica homologação em DPI múltiplo/mouse real.

**Decisão técnica:** NÃO habilitar `ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K`. A flag permanece OFF por padrão. O teste descarta a hipótese de que omitir o `viewport()->update()` adicional do wheelEvent, isoladamente, melhora o P95 nesse cenário. Não prova a causa da lentidão.

## Direção para a próxima investigação

Na sessão real de 07/11/2026 foram observados (telemetria de associação best-effort ao último evento): zoom P95 182,55 ms (154 amostras); global P95 156,40 ms; transform P95 32,91 ms (400 amostras); `drawForeground` P95 2,38 ms e prévia da transformação P95 1,25 ms. A categoria de zoom é um alvo de investigação, mas associação temporal não prova causalidade.

Priorizar **instrumentação separada** de `QGraphicsView::paintEvent`, pintura de contornos, composição e fundo/foreground, com amostras pareadas e preservação rigorosa dos pixels. Não repetir cache DeviceCoordinateCache nem repaint localizado 20h (já reprovados). Manter a versão principal, instalador, arquivos .alaser e máquina laser intactos.

## Estado
Teste Windows PASS em integridade/visual; **PERFORMANCE_REGRESSION**. Sem merge, sem produção ou interface física.
