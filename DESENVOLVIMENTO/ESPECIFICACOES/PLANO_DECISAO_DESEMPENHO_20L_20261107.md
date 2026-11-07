# Alencar Laser — revisão de desempenho com critério de entrega (07/11/2026)

## Resultados reais já obtidos
Sessão Windows 07/11/2026 (arquivo JSON fornecido pelo operador): 575 contornos, 13.415 nós, Qt 6.12.0, 624 paints. P95 global 156,4005 ms; zoom P95 182,5478 ms (154 amostras); transformação P95 32,9149 ms (400 amostras); foreground P95 2,3771 ms; background P95 17,2984 ms; prévia de transformação P95 1,2452 ms. A instrumentação associa eventos ocorridos até 250 ms antes de cada paint, sem demonstrar causalidade.

O experimento anterior 20k removeu atualização completa após zoom: P95 piorou em três pares (OFF 43,6223/32,5454/33,971 ms, ON 51,6104/68,5141/63,8139 ms). Não promovido. Experimento de cache DeviceCoordinateCache 20e e repaint localizado 20h também não tiveram ganho validado.

## Mudança concreta implementada
Branch `exp/phase20l-minimal-viewport-20261107`: opção `ALENCAR_MINIMAL_VIEWPORT_20L=1` permite `QGraphicsView::MinimalViewportUpdate`. Sem a opção, mantém exatamente `BoundingRectViewportUpdate`. Isso testa se preservar regiões de atualização separadas é melhor do que fundir em uma caixa grande, quando diversos contornos distantes ficam sujos simultaneamente. Pode ser melhor, igual ou pior: não afirmar aceleração antes do Windows.

Foi criado `SCRIPTS/TESTAR_VIEWPORT_575_20L_WINDOWS.ps1`, build Qt isolado, 3 pares alternados OFF/ON, 575 desenhos, 12 eventos de zoom, hash exato dos 12 frames e checagens de geometria/Undo. As opções experimentais reprovadas 20h/20k ficam explicitamente OFF. Relatório local mantém resultados e não faz merge nem utiliza o laser.

## Comando
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch exp/phase20l-minimal-viewport-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha no switch. Nao usar force/reset/clean' }
git pull --ff-only origin exp/phase20l-minimal-viewport-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_VIEWPORT_575_20L_WINDOWS.ps1'
```

## Decisão de continuidade — evitar investigação sem fim
Se houver regressão visual, manter a opção OFF. Se o P95 ON piorar em todos os pares, rejeitar. Se houver ganho consistente, validar com mouse real + projetos reais + DPI, repetir os sete gates e preparar candidata offline com instalador/reversão. Se nenhuma política nativa de viewport trouxer ganho, suspender micro-otimizações da pintura e construir um protótipo isolado de renderização vetorial em lotes por camada com seleção separada, comparando fidelidade completa e tempo real. Evitar alteração do documento canônico e controlar dependências.

**Status:** mudanças publicadas no GitHub, sem build Windows nem ganho de velocidade comprovado neste commit. Main, APLICATIVO, instalador e controle físico do laser permanecem intactos.
