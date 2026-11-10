# Alencar Laser — Tesoura 20x: seleção x prévia e vértices transversais

Data: 10/11/2026. Base isolada: `fix/tesoura-preview-clique-20w-20261110`.

## Evidência original
Capturas de 10/11: diversos segmentos da arte mostrados em laranja, mas o clique em alguns pontos não corta. No JSON anexado há 13 cortes, 3 recusas `vertex_or_tangent`, uma `single_closed_intersection`, um `miss`, sem comandos de máquina. Os dados agregados não permitem atribuir cada recusa à linha específica da captura.

## Diferença descoberta no código
- A seleção de objetos usa **laranja** (`QColor(239,109,25)`), inclusive quando a Tesoura é ativada.
- A prévia de corte usava outro laranja (`QColor(255,160,37)`). Um contorno selecionado podia parecer um candidato a corte quando a prévia não existia.
- A 20x muda **somente a prévia cortável para ciano** (`#22D3EE`), tanto para polilinhas quanto para Bézier. A seleção segue laranja. Mensagens explicitam: sem prévia ciano, não há corte seguro.

## Melhoramento topológico conservador
- A 20u já permitia cruzamento transversal num vértice interno de OUTRO contorno.
- A 20x permite também o cruzamento no vértice INTERNO do próprio contorno cortado, quando o segmento de referência cruza esse vértice pelo interior e os dois segmentos adjacentes do contorno cortado estão rigorosamente em lados opostos do segmento de referência.
- Ponta de linha aberta, tangência (ambas as arestas no mesmo lado), aresta degenerada, sobreposição e cruzamentos indeterminados permanecem recusados.
- Não se movem nós nem se fazem aproximações; o corte usa o mesmo intervalo contínuo e Undo existente.

## Testes de regressão adicionados ou alterados
- C++: cruzamento de um vértice interno do alvo em linha de referência; trechos de corte e prévia consistentes; guia preservada; Ctrl+Z restaura; tangência de vértice recusada.
- C++: antigo caso de vértice colinear do alvo contra referência transversal, agora reconhecido como cruzamento real.
- Qt: destaque de corte ciano; ausência de prévia fora de contorno; mensagens distintas de seleção laranja.

## Portões
1. Executar `powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura` e anexar saída em caso de falha.
2. Executar suíte completa após PASS.
3. Avaliar visualmente na arte REAL: contorno apenas selecionado laranja não deve ser confundido com prévia ciano; onde houver corte ciano, clicar no mesmo local e verificar Undo.
4. Exportar diagnóstico após tentar novamente os segmentos problemáticos. Recusas `single_closed_intersection` permanecem pendentes de estudo específico.
5. Não homologar nem mesclar `main` até validação visual, sem operação de máquina.

Status: mudanças em branch experimental; **Windows não testado nesta branch**.
