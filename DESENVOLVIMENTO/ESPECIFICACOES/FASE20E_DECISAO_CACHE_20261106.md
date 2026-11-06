# Alencar Laser — decisão experimental Fase 20e (06/11/2026)

## Evidência Windows fornecida pelo operador
- Branch: `exp/phase20d-item-cache-20261106`. CMake/Ninja compilaram o alvo isolado e o script PowerShell terminou normalmente.
- Comparação sintética de 575 contornos: mediana entre seis rodadas OFF=81,044 ms, CACHE=82,563 ms (cache ~1,87% mais lento na execução capturada).
- 40 contornos e 128 contornos servem como controles sem itens elegíveis para cache; nesses conjuntos não houve diferenças de pixel.
- 575 contornos: 818 pixels diferentes por rodada, total de 4.908 nas seis rodadas. Diferença concentrada na etapa 5: zoom 1,0; pan (40,20); diferença máxima de canal 153.
- Comparação direcionada na primeira rodada: cache recém-criado versus baseline sem cache = 818 pixels diferentes; cache aquecido/reutilizado versus cache recém-criado = 0 pixels diferentes.
- O resultado descarta a hipótese específica de que os pixels divergentes surgiram *somente* devido à reutilização histórica do cache nesse cenário. Não prova causa interna precisa: antialiasing, quantização, transformações e clipping ainda não foram isolados.
- O benchmark é QGraphicsScene offscreen sintético, NÃO teste de interação real com o mouse nem de precisão de arquivos de produção.

## Decisão
**NÃO integrar DeviceCoordinateCache para os caminhos 129+ no editor principal.** O ensaio não demonstrou benefício consistente e revelou divergência visual em uma transformação. Preservar o experimento no histórico para reprodução, sem editar o programa instalado ou habilitar a flag em produção.

## Próxima frente recomendada (Fase 20f)
Auditar o trabalho redundante durante interação real com QGraphicsView (seleção, arraste, zoom/pan), preferindo medição de invalidação e regiões repintadas antes de introduzir qualquer cache/rasterização. Investigar update/rebuild e seleção incremental em cenários densos; não alterar a geometria canônica, caminhos vetoriais ou Undo/Redo. Validar testes unitários e Qt com pixel diff, regressão visual manual e medições A/B em múltiplas rodadas.

**Segurança:** main, APLICATIVO, instalador, projeto .alaser v5 e transporte físico do laser preservados. Nenhum ganho causal de performance homologado; não publicar os relatórios brutos locais.
