# Alencar Laser 19.8 — Tesoura por trecho contínuo (referência: comportamento relatado no LightBurn)
Data: 27/10/2026
Estado: CRITÉRIOS DE ACEITAÇÃO / NÃO IMPLEMENTADO / NÃO HOMOLOGADO

## Relato do teste real
O usuário observou que a Tesoura recorta segmentos minúsculos e em outras regiões não recorta.
A geometria de entrada pode representar curvas suaves com muitas arestas curtas, mesmo quando, na interface, os nós não aparecem individualizados.
No comportamento de referência do LightBurn, ao selecionar a ferramenta o cursor permanece como seta; quando alcança um trecho válido, o cursor muda para Tesoura e **todo o trecho a remover** fica em destaque. Um clique remove o trecho destacado por inteiro.

## Causa confirmada no código 19.8
- `qt/qt6_test6.cpp`: `setTool(Tool::Scissors)` define permanentemente cursor Tesoura; `mouseMoveEvent` não calcula nem destaca trechos candidatos; `mousePressEvent` chama `editor.removePolylineEdge(hit.key,hit.segment)`.
- `src/editor_tools.hpp`: `removePolylineEdge` remove somente **uma aresta entre nós adjacentes**. Em elipses poligonais com 64+ arestas e desenhos importados densos, isso remove um pedaço minúsculo; não identifica interseções/limites topológicos.
- O teste atual `editTools197Test` corta uma única aresta de um retângulo de 4 vértices, portanto não representa o defeito reportado.
- Essas são conclusões da leitura do código e do relato do usuário; os vídeos enviados ainda não tiveram os quadros comparados com confiabilidade.

## Comportamento obrigatório da nova ferramenta
1. Tecla K ativa Tesoura, cursor **seta** sobre espaço livre ou trechos sem operação válida.
2. Ao aproximar de um trecho **válido** dentro da tolerância visual (pixels de tela, independente do zoom), calcular os limites do **trecho contínuo** na geometria, ignorando a divisão artificial em dezenas de arestas.
3. O cursor muda para Tesoura apenas sobre candidato válido. Destacar todo o **caminho a remover**, com cor contrastante temporária. Destaque é overlay de prévia, nunca muda o documento, undo, camada, arquivo ou máquina.
4. Um clique remove exatamente o trecho destacado — não apenas a aresta sob o cursor — incluindo limites de interseção que caiam **no meio das arestas**, sem sobras sobrepostas e sem costuras falsas.
5. Interseções com outras geometrias visíveis podem delimitar o trecho. Calcular interseções reais (inclusive no meio do segmento), distinguir cruzamento de tangência e tratar nós quase coincidentes com tolerâncias geométricas adequadas.
6. Para trajetos abertos, extremidades do caminho também podem ser limites; para contornos fechados, exigir limites suficientes para escolher um trecho inequívoco. Se não houver candidato bem definido, manter seta normal e não excluir uma aresta arbitrária.
7. Garantir que a remoção respeite seleção, camadas bloqueadas/invisíveis e agrupamentos; novas peças preservam a camada do contorno original. Não modificar geometrias de outros objetos por serem usadas como fronteiras.
8. Desfazer/Refazer devem restaurar o estado completo numa única transação; não gerar IDs duplicados; preservação na serialização `.alaser` v5.
9. Se curvas Bézier nativas ainda não tiverem geometria/corte compatíveis, mostrar indisponibilidade com segurança — não converter silenciosamente para fragmentos nem afirmar suporte integral.
10. Esc retorna ao modo de seleção e descarta overlay; ao mover o cursor para espaço livre, limpar prévia e retornar à seta.
11. Zero acesso à controladora, USB/COM ou Wi-Fi nesta implementação de editor.
12. O desempenho do hover deve utilizar cache por geração da geometria, invalidado após edições, em vez de buscar interseções de todos os contornos a cada movimento do mouse.

## Cenários de aceitação obrigatórios
- A1: Linha aberta atravessada duas vezes por outro contorno. Ao pairar na seção intermediária, toda a seção entre cruzamentos acende e é removida em uma operação; outros dois trechos persistem, visual e geometricamente independentes.
- A2: Contorno fechado de 64 ou mais arestas (elipse) cruzado por outro contorno em dois pontos. A prévia abrange o arco completo entre interseções, não 1/64 do perímetro. Clique elimina o arco, preservando o restante e o contorno cruzador.
- A3: Interseção localizada no meio de uma aresta, não sobre um nó. O resultado termina exatamente nesse ponto e não gera aresta oculta/duplicada.
- A4: Linha sobreposta/colinear, tangências ou nós quase coincidentes não devem disparar exclusão ambígua; devem ser tratados especificamente ou recusados sem alterar o documento.
- A5: Sem interseções suficientes no contorno fechado: sem prévia de exclusão, sem edição destrutiva. Em trajetória aberta, testar limites em extremidades explicitamente.
- A6: Mouse entra/sai do trecho: seta ↔ Tesoura; overlay surge/desaparece sem alterar undo/documento; zoom e scroll preservados.
- A7: Objeto em camada bloqueada ou grupo não editável não corta. Nenhum contorno vizinho desaparece. Ctrl+Z e Ctrl+Y restauram os segmentos e metadados.
- A8: Projeto com 575 contornos / 13.415 nós: hover responsivo e sem travamentos; medir p95 de detecção e renderização antes/depois.
- A9: Salvar/reabrir arquivo `.alaser` v5 mantém exatamente os recortes, com IDs exclusivos, e não envia comandos à máquina.
- A10: Duas formas visualmente próximas mas não conectadas: destaque limitado ao objeto correto, sem mudança de geometria no outro.

## Sequência de desenvolvimento
1. **Modelador geométrico isolado**: encontrar cruzamentos, delimitar o intervalo paramétrico e gerar `ScissorsCandidate` (pontos, chave original, índices/limites, preview e viabilidade). Sem Qt, sem escrita do documento. Testes densos antes de integrar.
2. **Aplicador transacional**: remover um intervalo inteiro e distribuir os restos em contornos abertos válidos, com metadados e undo/redo.
3. **Interface Qt**: mouse tracking com cursor condicional e overlay do candidato, atualizando apenas quando candidato mudar. Nunca usar `removePolylineEdge` como substituto silencioso.
4. **Regressão Windows**: validar A1–A10 e os testes existentes, fotos/vídeos de antes e depois no desenho real. Não mesclar a branch antes do PASS.

## Proteção de versões
- Branch de engenharia `fix/scissors-continuous-span-19-8` criada a partir da candidata experimental.
- `main` permanece estável; não realizar merge até todos os testes.
- A antiga ferramenta por **aresta única** pode continuar no núcleo por compatibilidade com regressões antigas, mas não representar a nova Tesoura de trechos na interface.
