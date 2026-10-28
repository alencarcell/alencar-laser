# Alencar Laser 19.8 — Contornos internos antes dos externos

Data: 28/10/2026. Branch experimental: `feat/refino-curvas-rastreio-diagnosticos-19-8`.

## Objetivo e referência

A partir de `Optimization Settings > Cut inner shapes first` no LightBurn, fazer com que a **prévia virtual** ordene primeiro os elementos estritamente dentro de um contorno fechado antes de considerar o externo; isso costuma ser importante ao cortar peças que podem cair, se mover ou perder referência após destacar a borda. A documentação oficial deixa claro que só há precedência se o **contorno externo for fechado** e que a otimização só opera quando a opção global de otimização está ativada.

Referências:
- https://docs.lightburnsoftware.com/2.1/Reference/OptimizationSettings/
- https://docs.lightburnsoftware.com/2.1/GetStarted/PreviewBeginner/

O LightBurn também dispõe de prioridades, grupos, direção, escolha de início e outros algoritmos. **Nossa abordagem não é equivalente à engine do LightBurn**.

## Comportamento implementado (offline, sem G-code)

1. `preview_nesting_19_8.hpp`: determina relações **estritas** de contorno externo fechado contendo um elemento interno da mesma camada, por:
   - caixas delimitadoras com margem conservadora;
   - rejeição de vértices sobre bordas;
   - ray casting de paridade e interseção de segmentos;
   - detecção conservadora de polígono externo autointersectante (rejeitado);
   - orçamento de até 4 milhões de verificações geométricas (erro explícito ao ultrapassar).
2. `preview_route_19_8.hpp`: estende `nearestNeighborLines` com `innerFirst` opcional (padrão `true`) e parâmetro opcional para contabilizar **pares de restrição**. Só escolhe contorno externo quando todos os objetos estritamente dentro dele tiverem sido tratados. Entre caminhos elegíveis, usa a heurística já existente de vizinho mais próximo.
3. **Precedência e opt-out**:
   - Só participa o que está em modo `Line` com `Output=true`; T1/T2 e Fill continuam fora dessa otimização.
   - Mantém a **ordem por camada**, sem mudar prioridades entre camadas. Cada camada com `optimizeTravel=false` preserva a ordem original de seus contornos, mesmo se a opção global `innerFirst` estiver ativada.
   - A ordem interna pode **aumentar** a distância total de viagens. Isso não é defeito: a precedência de cortes tem prioridade sobre a minimização local de distância.
   - Objetos internos abertos também têm precedência se todas as arestas estiverem estritamente dentro do contorno externo fechado. Formas apenas sobrepostas, tangentes ou tocando a borda não criam precedência.
4. `qt/preview_dialog_19_8.hpp`:
   - Novo checkbox **Contornos internos primeiro (corte virtual)**, ativado por padrão mas só habilitado se a otimização global estiver ligada.
   - Estatística apresenta distâncias de viagem Line original e da ordem escolhida, além do número de pares de restrição encontrados; JSON de prévia inclui `inner_first_enabled`, `strict_nesting_constraints` e `inner_first_only_same_layer`.
   - Não altera o documento, atributos de camada, histórico, imagem, geometria nem posição real da máquina.

## Testes acrescentados

- `qt6_test6.cpp --route-nesting-19-8-test` grava `RESULTADO_CONTORNOS_INTERNOS_19_8.json` e verifica:
  - Retângulos de três níveis criados externo → médio → interno, mas calculados na ordem interno → médio → externo;
  - preservação do comprimento total e estado imutável;
  - opção `innerFirst=false` conserva escolha de vizinho mais próximo;
  - `Layer.optimizeTravel=false` prevalece e conserva ordem original;
  - formas em **camadas diferentes** não criam dependências cruzadas;
  - linha **aberta** estritamente interna precede a borda fechada;
  - bordas tangentes não criam falsas dependências;
  - guias T1/T2 continuam excluídas.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`: `-SomenteSimulacao` e `-Abrir` chamam esse teste e exigem JSON com `status=PASS`. A suíte integral inclui as regressões anteriores.

## Limitações e pontos de atenção

- **Topologia conservadora**: só aceita contornos externos fechados simples; autointersecção ou degeneração não origina precedência. Não trata curvas Bézier contínuas como entidades topológicas completas, somente os pontos/segmentos vetoriais discretizados do editor.
- **Desempenho:** o limite global existente é 1.500 contornos; o cálculo estrito de aninhamento também limita o número de operações geométricas. Se excedido, exibe falha de prévia em vez de usar automaticamente outra ordem e declarar que está otimizada.
- Não prioriza pela distância a ponto de origem de máquina, não considera colisões, quedas, pontes/tabs nem otimização CAM; a regra só produz sequência para visualização.
- Nenhum novo módulo usa comunicação USB, TCP, GRBL, M3/M4 físicos, motor ou potência.

## Situação de validação

**Publicado no GitHub e testes Qt adicionados; compilação Qt6 Windows desta evolução PENDENTE.** O último PASS anterior do usuário era para simulação cinemática, não para esta implementação de contornos internos. Não marcar homologado sem imagem/log do usuário.

### Comando para Windows (sem laser conectado)

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw 'Git interrompido; preservar arquivos locais.' }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

Se o primeiro teste passar, executar a suíte `-Abrir` e conferir visualmente a ordem com três retângulos aninhados, um recorte interno aberto, mudança de camada e a diferença ao ligar/desligar a precedência. Manter sempre desligada/desconectada a máquina.

### Próximo passo

Aprimorar otimização por grupo/prioridade e testar a ordenação com LBRN2 reais e relatórios do LightBurn; trabalhos multimodais com arquivos e vídeos comparativos poderão se beneficiar do ChatGPT Work.
