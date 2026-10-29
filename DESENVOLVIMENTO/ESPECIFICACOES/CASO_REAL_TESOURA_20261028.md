# Alencar Laser 19.8 — caso real TESTE TESOURA.alaser, camadas e LBRN2
Data: 28/10/2026. Status: CÓDIGO EXPERIMENTAL NA BRANCH, Windows NÃO COMPILADO / NÃO HOMOLOGADO.
Arquivo fonte do cliente: TESTE TESOURA.alaser (original preservado, não enviado ao GitHub).
SHA-256: 711c9cf4cf103611c9c208394a4b7ac33fe9a1a2ca1a05ca22286a844ba48026.

## Diagnóstico do pacote v5 enviado
- 575 contornos, 13415 nós, 0 curvas Bézier.
- Todos têm closed=false, porém 571 caminhos (>=4 pontos) repetem *exatamente* a primeira coordenada no último nó. Isso é fecho geométrico sem metadado de fechamento.
- 1 contorno de 2 nós termina a ~0,002525 mm do começo: NÃO converter automaticamente, porque só possui dois nós.
- 3 caminhos têm extremidades realmente separadas; permanecerão abertos.
- Nenhum dos 575 objetos está agrupado (group=0); Desagrupar NÃO resolve o problema de tesoura neste arquivo.
- O projeto contém seis camadas declaradas, mas seus 575 objetos estão atribuídos à camada 0.
- O JSON de diagnóstico registra 7 mudanças de camada e 4 movimentos, incluindo operações sobre 576 objetos, sem conexão com máquina.

## Correção da Tesoura
- `hasExactDuplicateClosure` reconhece somente repetição exata (1e-9 mm) em caminhos com >=4 nós e closed=false.
- `scissorLogicalContour` devolve cópia fechada sem nó terminal redundante somente para cálculo.
- Planner de interseções e preview usam o caminho lógico fechado; o documento **não é alterado ao mover o mouse**.
- `Editor::removeScissorSpan` aceita esse fecho virtual e aplica mudanças só no objeto clicado, com Ctrl+Z restaurando o estado exato anterior.
- Tangências, sobreposições colineares e cruzamentos ambíguos continuam recusados para impedir cortes incorretos; não prometer todos os recortes.
- Teste sintético regressivo `--scissor-closures-19-8-test`: anel isolado, anel cruzado, undo, ausência de efeitos colaterais.

## Interação LightBurn
Referências:
- https://docs.lightburnsoftware.com/latest/Reference/TransformControls/
- https://docs.lightburnsoftware.com/latest/Reference/CutsLayersWindow/
- https://docs.lightburnsoftware.com/2.2/Reference/UI/ColorPalette/
- https://docs.lightburnsoftware.com/2.2/Reference/TrimShapes/
- Seleção ganhou alça central 13ª (índice 12), com arraste em prévia, Shift encaixa movimento a múltiplos de 45°, commit como uma ação de histórico.
- Paleta agora: com seleção, clique simples muda a camada dos objetos selecionados; sem seleção, clique ativa a camada de criação; Shift+clique seleciona todos os objetos editáveis da camada.
- Fill é parâmetro do projeto, mas **a área gráfica mostra somente contornos por padrão**, como LightBurn. Checkbox opcional de prévia translúcida não cria operações reais; inclui formas com fecho duplicado.
- O usuário precisa testar esses gestos com mouse físico, inclusive objetos pequenos e seleções múltiplas.

## LBRN2
- **Não foi enviado arquivo .lbrn2 nesta mensagem**; o arquivo em anexo `TESTE TESOURA.alaser` é um arquivo próprio ZIP v5.
- O importador atual exige XML LightBurnProject com Rect, Ellipse, Path e Group; recusa outros Shapes ou arquivos com recursos não suportados integralmente para impedir importação com perda parcial.
- Foi adicionado Rect com raio Cr (cantos arredondados) e elipses aumentadas de 96 para 384 segmentos; teste sintético de importação incluído.
- Não considerar isso uma correção abrangente de LBRN2 sem amostra real rejeitada e mensagem do parser. Projetos com Text, Bitmap, arcos avançados, múltiplas cadeias de Path, certos transform ou espelhamentos podem continuar bloqueados.

## Comandos para testar (C:\ProjetoAlencarLaser)
- `git status --short`
- `git fetch origin`
- `git switch --track origin/fix/tesoura-fechos-lbrn2-selecao-19-8` (somente se puder trocar sem perder mudanças)
- `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteCasoReal`
- Depois, se PASS: `-SomenteCamadas`, `-SomenteImportacao` e `-Abrir` (executar separadamente).
- Testar o arquivo real sobre uma **cópia do projeto**. Manter máquina desconectada; nada libera G-code, GRBL, movimento ou emissão.
- Não fazer merge/pacote instalável até recompilação PASS, ensaio de mouse real, abertura/backup/reabertura e verificação do .lbrn2 real.
