# Alencar Laser TESTE_19.8 — Camadas compactas, ajustes M3/M4 e roteamento virtual

Data: 28/10/2026. Branch: `feat/refino-curvas-rastreio-diagnosticos-19-8`. **Candidata experimental, sem homologação Windows desta etapa.** A versão homologada, `main`, `APLICATIVO` e conexão física permanecem inalterados.

## Fontes oficiais de referência

- LightBurn Cuts / Layers: https://docs.lightburnsoftware.com/latest/Reference/CutsLayersWindow/
  - Uma linha por camada com Output/Show/Air, modo, velocidade/potência; duplo clique abre Cut Settings Editor. Output e Show são independentes.
- LightBurn Main / Shared Settings: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/SharedSettings/
  - `M4`: **Variable Power / potência dinâmica**, reduz ou aumenta potência conforme velocidade, usual em GRBL compatível. `M3`: **Constant Power / potência constante**. Não é correto chamar M3 de "modo perfuração": a perfuração é uma operação/processo, não o significado do comando.
- LightBurn Optimization Settings: https://docs.lightburnsoftware.com/2.1/Reference/OptimizationSettings/
  - Otimização é global, com opções como Order by Layer, Reduce Travel Moves, Cut Inner Shapes First, escolha de ponto de início/direção e prioridades. Nossa heurística não reproduz esses critérios.

## Implementado nesta rodada (sem hardware)

### 1. Layout compacto da lista
- `qt6_test6.cpp`: linhas de **26 px**, caixa S/V/A sem texto de **20 px**, cabeçalho com três colunas alinhadas e botão de nome simples/elidido quando longo; tooltip mostra nome inteiro. A lista continua contendo camadas ocupadas e T1/T2.
- Um clique seleciona a camada. **Duplo clique** no nome abre janela modal de parâmetros; para evitar destruir widgets durante MouseButtonDblClick, abre no próximo evento Qt. T1/T2 não aceitam parâmetros de gravação. A lista não controla fisicamente o laser.
- A coluna S = saída na prévia virtual, V = mostrar no editor, A = anotação de ar sem atuador.

### 2. Editor de camada
- `qt/layer_settings_19_8.hpp`: formulário de nome, modo editorial Line/Fill, velocidade em mm/min (10–100000), potência nominal em % (0–100), 1–20 passagens e escolha M4 dinâmico / M3 constante.
- `src/editor_tools.hpp`: `Layer` ganhou `speedMmMin`, `powerPercent`, `passes`, `powerMode`; o método `configureLayer` aplica de uma só vez com um registro no histórico Undo. Também existe validação central de limites.
- `metadata()` e `fromMetadata()` serializam os quatro campos; ao abrir projeto legado sem eles, adotam velocidade 3000 mm/min, potência nominal 20%, uma passagem e M4. Esses valores são *defaults de interface*, NÃO parâmetros reais da máquina ou materiais.
- T1/T2 permanecem sem saída, e a API recusa configuração de potência nessas camadas.

### 3. Rota/estimativa virtual
- `src/preview_route_19_8.hpp`: heurística de **vizinho mais próximo**, agrupando por camada em ordem crescente de ID, aproximando o próximo contorno pelo ponto inicial mais perto. Permite inverter contornos abertos e começar contornos fechados em vértice existente. Não altera os nós de projeto e não reordena entre camadas.
- `qt/preview_dialog_19_8.hpp`: opção habilitável **"Reduzir deslocamentos (heurística local por camada)"**, compara milímetros de deslocamento Line originais e com heurística (sem misturar Fill/Imagem). A rota pode não ser globalmente ótima.
- O simulador usa **velocidade salva por camada** quando "Utilizar velocidades virtuais salvas por camada" estiver ativo. `src/preview_motion_19_8.hpp` oferece `estimateWithFeeds`.
- `src/preview_sim_19_8.hpp`: `repeatedByLayer` repete cada grupo contíguo de uma camada conforme suas passagens e também permite passagens globais; esta é aproximação editorial, NÃO a agenda de subcamadas real do LightBurn. Limites de movimentos são mantidos.
- Potência/M3/M4 são somente **metadados exibidos e salvos**. Não existe implementação de estimativa de potência em cantos (M4), acionamento M3, comando laser, G-code, ar assist ou emissão física nesta rodada.

## Testes que acompanham a alteração

- `qt6_test6.cpp --layer-route-19-8-test`: verifica trajeto com deslocamento reduzido em caso determinístico, comprimento ativo preservado, geometria imutável, velocidade/potência/M3/passagens, metadados antigos com defaults M4, ida/volta Undo/Redo, recusa potência acima de 100% e T1 fora da rota.
- `qt6_test6.cpp --inline-transforms-19-8-test`: valida tamanho 26 px das linhas e 20–22 px dos três checkboxes, existência de propriedade usada pelo duplo clique.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`: `-SomenteSimulacao` e `-Abrir` exigem `RESULTADO_CAMADA_ROTA_19_8.json`.
- O usuário confirmou PASS Windows da **rodada anterior de simulação cinemática**, mas ESTA alteração **ainda não foi recompilada/rodada no computador**. Os commits e testes são trabalho preparado, não uma certificação de funcionamento.

## Validação segura

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Atualizacao interrompida; preservar mudancas locais" }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

Se passar, testar `-SomenteInteracao` e `-Abrir`, **sem máquina conectada**. Ao abrir a aplicação, confirmar alinhamento da lista de camadas, três checkboxes independentes, janela por duplo clique, Cancel sem alteração, Save e Undo, comparação de rotas e diferença de duração com velocidades distintas. Salvar em cópia `.alaser` e reabrir para validar metadata. Analisar os prints/vídeos, inclusive ícones da barra esquerda, sem inferir homologação de simples compilação.

## Pendências necessárias

1. Testar em Windows, com Qt6, duplo clique e arquivos reais.
2. Evoluir heurística para "Cut Inner Shapes First", ordens manuais, agrupamento e redução de mudanças de direção; não renomear heurística como "melhor rota garantida".
3. Testar desempenho em milhares de contornos; limite de 1.500 evita busca quadrática excessiva.
4. Calibrar velocidade, aceleração e timing com relatórios do LightBurn; implementar modelagem de potência M4 separada e sujeita a validação física futura.
5. Nenhum acionamento físico M3/M4 ou gravador CAM antes de autorização e homologação independente.

## Revisao da mesma rodada — otimizacao configuravel em cada camada

- O Cut Settings Editor experimental ganhou o checkbox **"Permitir otimizar rota desta camada na previa"**. Esse campo e agora guardado em `Layer.optimizeTravel` (padrao `true` para projetos antigos), com Undo/Redo e persistencia `.alaser`.
- O checkbox global de otimização da prévia precisa estar marcado para a heurística atuar. Se a camada desabilitar seu próprio checkbox, `preview_route_19_8.hpp` preserva a ordem e a direção originais dos contornos dessa camada, embora outras camadas possam ser reordenadas.
- O teste `--layer-route-19-8-test` agora confere: OFF mantem distância de viagem original; ON reduz distância da fixture; JSON salva e reabre OFF; remover a chave em metadata legada restaura default ON.
- Não equivale ao planejador completo do LightBurn; continuam pendentes otimização por contorno interno, agrupamentos e prioridades.

## Visualização conceitual de M3/M4 no editor

- `src/preview_motion_19_8.hpp::virtualPowerPercent` foi adicionado para **demonstração matemática**, não simulação fiel de PWM/GRBL: para M3 retorna potência nominal constante; para M4 calcula potência nominal × razão entre velocidade atual hipotética e solicitada, limitada a 0–100% do valor nominal. Em velocidade zero, M4 retorna zero e M3 mantém nominal na demonstração.
- `qt/layer_settings_19_8.hpp` mostra em tempo real o exemplo de potência se a velocidade fosse 50% da programada. Isso esclarece M3 x M4, mas **não permite inferir potência física, foco, perfuração, tempo de permanência ou resultado de material**.
- O harness C++20 `tests/preview_motion_19_8_standalone.cpp` ganhou verificações M3 a 50%, M4 a 50%, M4 em velocidade zero e M4 com velocidade acima da solicitada. Ainda não foram rodadas no Windows nesta rodada.
