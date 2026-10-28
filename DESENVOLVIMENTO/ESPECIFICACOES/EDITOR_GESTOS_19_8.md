# Alencar Laser 19.8 — bloco de gestos do editor (candidata)
Data: 28/10/2026
Status: CÓDIGO PUBLICADO NA BRANCH EXPERIMENTAL, **SEM COMPILAÇÃO WINDOWS / SEM HOMOLOGAÇÃO**.

## Relato de uso que motivou o bloco
- A logomarca oficial ocupa altura excessiva no cabeçalho do notebook.
- Linha existente cria um único segmento a cada arraste; LightBurn mantém criação do mesmo contorno entre vários cliques.
- Desenho livre (P) é útil, mas é uma ferramenta distinta de linha contínua.
- Falta seleção retangular fora das formas e menu de contexto ao clicar com botão direito.
- Tesoura ainda não resolve todos os contornos, especialmente casos ambíguos; não prometer equivalência LightBurn.

## Código experimental
- Cabeçalho: mesmo arquivo da logo oficial, dimensão de exibição reduzida de 248x68 para 180x40 e altura de label de 70 para 42; linha redundante de subtítulo oculta. NÃO altera imagem PNG original.
- Linha: clique inicial + cliques subsequentes criam uma só polilinha; mouse move mostra preview; clique perto do primeiro vértice com >=3 pontos fecha o contorno; botão direito conclui aberto; Esc cancela, Ctrl+Z desfaz o objeto todo. Arraste longo inicial conserva a linha de segmento existente para não quebrar testes legados.
- Seleção: arraste iniciado fora de contorno; esquerda-direita exige contorno totalmente contido; direita-esquerda aceita cruzamento. Shift adiciona; Ctrl alterna; Ctrl+Shift retira; Ctrl+A seleciona todos os objetos editáveis. Não inclui curvas Bézier nativas gerenciadas pela sessão separada.
- Menu direito no editor: selecionar tudo, excluir, desfazer/refazer, linhas, Tesoura, desenho livre, Texto, formas, agrupamento, duplicação e transformações já existentes. No modo linha, botão direito conclui sem abrir menu.
- Tesoura: mensagem específica para ausência de hit, camada bloqueada/oculta, objeto agrupado e candidato de recorte ambíguo. Geometria não foi alterada neste bloco.
- Emissão física, G-code e controle laser continuam BLOQUEADOS. Não habilitar Wi-Fi/TCP.

## Gates de teste
1. Na raiz C:\ProjetoAlencarLaser, conferir `git status --short --branch`.
2. `git fetch origin` e mudar para `feat/editor-gestos-19-8` somente se não houver mudanças locais a preservar.
3. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteGestos`
4. Corrigir erro de compilação, runtime ou JSON primeiro. Diagnóstico: `RELATORIOS/AUDITORIAS/GITHUB_19_8/gestos/RESULTADO_GESTOS_19_8.json`.
5. Se PASS, rodar a suíte completa: `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -Abrir`.
6. Teste com mouse físico: criar linha com >=4 vértices e concluir com botão direito; fechar outra no ponto inicial; Esc durante linha e depois Ctrl+Z; seleção por cruzamento e por inclusão, Shift/Ctrl; menu de contexto e logo reduzida; Tesoura em projetos reais.
7. Se houver falha específica da Tesoura, salvar uma **cópia** do .alaser/.lbrn2 e registrar geometria exata antes/depois. Não aplicar hotfix geométrico por hipótese.

## Limitações
- Sem validação Windows atual da nova branch. Teste sintético pode não representar mouse real.
- Tesoura não alcança equivalência LightBurn em todos os cruzamentos, curvas Bézier nativas e geometria complexa; grupos deliberadamente bloqueados.
- LBRN2 conserva suporte parcial, importação atômica e recusas seguras.
- `main` e `APLICATIVO` aprovados intocados. Não publicar instalador nem mesclar sem PASS completo.
