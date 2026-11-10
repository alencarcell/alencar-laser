# Fase 20t — Tesoura guiada por Trim Shapes do LightBurn (10/11/2026)

## Referência oficial
https://docs.lightburnsoftware.com/2.2/Reference/TrimShapes/

A referência documenta: o hover indica o trecho antes do corte; o clique remove até a interseção seguinte; sem interseções, elimina a forma inteira; agrupamentos exigem desagrupar; união automática de caminhos compatíveis (Ctrl desativa); Shift pode preservar o trecho em camada separada.

## Diagnóstico real
Sessão TESTE_19.8 de 10/11/2026: 17 cortes, 6 tentativas sem trecho seguro, 4 sem hit, 1042 movimentos hover; documento com 1737 contornos / 39913 nós. Dados agregados não permitem inferir a geometria exata dos seis casos ambíguos.

## Implementado nesta branch
- O planner passa a indicar motivos categorizados da recusa sem alterar o cálculo nem os contornos: `collinear_overlap`, `vertex_or_tangent`, `degenerate_edge`, `single_closed_intersection`, `boundary_proximity`, `no_interval`, `short_preview`, `invalid_segment`, `missing_contour`.
- A interface mostra instruções contextualizadas no erro de clique e registra a categoria no campo de detalhe da tentativa (sem coordenadas).
- Preservados a prévia, o cursor condicional, o recorte do intervalo contínuo e a exclusão integral de formas sem cruzamentos.
- Testes de núcleo para contorno cruzado em vértice, sobreposição colinear e forma isolada.
- NÃO foi acrescentado algoritmo de união automática: reproduzir união de caminhos em vértices pode alterar o desenho em geometrias densas e deve ser implementado isoladamente com testes e opção Ctrl. Também não implementados Shift preservação, nem ajuste automático de pontos de tangência.

## Gates obrigatórios
1. Compilar testes de Tesoura no Windows com `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura` e rodar a suíte completa `-Abrir` após PASS; laser sempre desconectado.
2. Validar hover/corte do mesmo arquivo original sem sobrescrita: contornos abertos, anéis com fecho virtual, cruzamentos e sobreposições, camadas ocultas/bloqueadas e grupos.
3. Verificar que recusas nunca geram novos estados Undo; sucesso gera uma única transação e Ctrl+Z restaura os bytes geométricos.
4. Medir desempenho em 575/1737 contornos e comparar com baseline, sem afirmar aceleração sem teste pareado.
5. Novos códigos de recusa devem ser diferenciados de falha de mouse e do caso `miss`.

Status: alterações no GitHub, sem compilação Windows nesta revisão, sem homologação ou merge. Main e APLICATIVO intocados.
