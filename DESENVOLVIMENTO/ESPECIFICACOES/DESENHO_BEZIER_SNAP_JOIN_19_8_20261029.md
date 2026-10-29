# Alencar Laser Windows 19.8 — Desenhar linhas, Bézier por arraste, magnetismo e Auto-Join

**Data:** 29/10/2026  
**Branch exclusivamente experimental:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Situação:** código publicado e testes adicionados; **a compilação Qt6/Windows e o funcionamento visual das alterações deste lote ainda não foram confirmados**. Nada foi mesclado em `main`, alterado em `APLICATIVO` ou conectado ao laser.

## Comportamento de referência

Documentação oficial LightBurn (consultada em 29/10/2026):
- https://docs.lightburnsoftware.com/latest/Reference/DrawLines/
  - clique coloca nó de canto/reta; clique e arraste posiciona nó suave com alças; direita conclui, clique na origem fecha, Esc cancela; ao iniciar/terminar nas pontas de caminhos abertos, novos trechos podem ser conectados e os caminhos combinados;
  - `Ctrl` desabilita Auto-Join; snapping por proximidade tem indicação visual;
- https://docs.lightburnsoftware.com/1.7/Reference/AutoJoinSelectedShapes/
  - `Alt+J` une pontos finais quase coincidentes;
- https://docs.lightburnsoftware.com/latest/Reference/EditNodes/
  - edição de nós e alças após o desenho.

**Não alegar paridade total:** no LightBurn, Draw Lines é uma ferramenta integrada. Nesta 19.8, a interface ainda apresenta um botão de linha poligonal e outro de desenho de curvas nativas, ambos com interação semelhante de clique. Unificação completa de Bézier, polilinhas importadas e grupos é trabalho futuro.

## Implementação entregue

### Linhas e snap
- `src/editor_tools.hpp::nearestOpenEndpoint`: busca candidatos apenas em extremidades de **contornos abertos**, no layer ativo, que estejam visíveis, desbloqueados, sem grupo e com um único subcaminho. Em projetos com muitas formas, contagem de subcaminhos pré-calculada (busca linear).
- `qt6_test6.cpp`: no modo **Desenhar linhas**, o raio de snapping usa **10 pixels físicos da tela** convertidos em milímetros pela transformação da viewport e DPR; o marcador circular é **apenas uma prévia**, sem deslocar ou gravar geometria antes do clique. `Ctrl` inibe o snap; `Shift` continua restringindo ângulo a 45°.
- `Editor::commitJoinedPolyline`: confirma em uma transação Undo. Pode continuar um contorno existente a partir da ponta, unir duas formas abertas da mesma camada preservando identidade do primeiro objeto, ou **fechar o contorno original** se uma linha unir suas duas extremidades opostas. Nunca força fechamento em objeto fechado, agrupado, de outra camada ou travado.
- O editor de linhas conclui automaticamente o traçado quando o clique de destino se fixa em ponta de contorno aberto; botão direito ainda termina uma linha aberta e clique no ponto inicial fecha novo contorno.

### Bézier por clique e arraste
- `src/bezier_interaction.hpp::CurveDragSession::addEditablePath`: novo construtor de caminhos nativos com nós de canto (alças coincidem com âncora) e nós suaves (alças opostas com comprimento baseado no arraste), arestas 0=reta e 1=cúbica. Todas as novas âncoras são salvas como **um único passo de Undo**.
- `qt6_test6.cpp`: botão da barra lateral `new_bezier` agora ativa a ferramenta **Desenhar curva** em vez de inserir uma curva pré-fabricada. Primeiro clique coloca âncora; clique simples seguinte cria reta; pressionar e arrastar numa âncora posiciona tangentes refletidas e cria/condiciona a curva; botão direito conclui, clique próximo à primeira âncora fecha caminho, Esc cancela.
- Curvas continuam gravadas no `CurveDocument` nativo v5, sem achatar em polilinhas. O botão de edição de Bézier continua disponível separadamente para manipular alças e âncoras.

### Unir nós existentes
- `Editor::joinTwoOpenContours`: operação explícita selecionando **exatamente dois caminhos abertos** da mesma camada, desbloqueados e não agrupados. Escolhe o par de extremidades mais próximo (até 0,5 mm), inverte um contorno quando necessário, concatena a geometria, elimina a segunda referência e registra **um único Undo**. Para pontas perfeitamente coincidentes, remove nó duplicado.
- UI em **Editar → Unir pontas selecionadas (Alt+J)**, com mensagem de recusa quando objetos não são elegíveis.

## Testes preparados (não confundir publicação com PASS)

- `tests/preview_draw_join_19_8_standalone.cpp`: retângulo aberto faltando um lado, fechamento preservando objeto, Undo/Redo, duas linhas separadas unidas via ponte, extensão de linha, proibições de camadas/pontos, Alt+J com pontas coincidentes e longe demais, caminho nativo com retas/cúbicas e Undo/Redo.
- `qt6_test6.cpp --draw-join-19-8-test`: testa os modelos C++, presença do botão e menu, além de tentar simular cliques e arraste via QTest no viewport para um Bézier e o fechamento de um retângulo aberto. Produz `RESULTADO_DESENHO_UNIAO_19_8.json`.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteDesenho`: compila o teste C++ portátil, compila Qt6 em modo diagnóstico e executa o teste acima. A suíte integral `-Abrir` também exige o gate Qt.
- `.github/workflows/preview-19-8-offline.yml`: adicionou teste C++ de união ao workflow experimental; execução não confirmada por esta etapa.

## Fluxo de validação em Windows

Com máquina desconectada e sem alterações conflitantes na árvore local:

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Git interrompido; preserve a base e alteracoes locais." }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho
```

Se PASS, `-Abrir` para teste manual:
1. Criar quatro nós por clique no botão **Desenhar curva**; confirmar que os segmentos são retos, clicar no primeiro para fechar.
2. Criar nós clicando-arrastando; confirmar que aparecem curvas suavizadas e editar depois as alças na ferramenta Bézier.
3. Criar um retângulo **aberto** com três lados; selecionar **Desenhar linhas**, clicar na extremidade do último lado e depois na extremidade do primeiro; confirmar que resta um único contorno fechado.
4. Criar duas linhas separadas com as pontas quase coincidentes; selecioná-las e usar **Alt+J**; confirmar uma forma só, desfazer Ctrl+Z.
5. Verificar que não houve união com objetos de camadas diferentes, agrupados, invisíveis ou bloqueados; testar zoom e Ctrl para cancelar snap.
6. Testar salvamento .alaser v5 e reabertura, sem fabricar ou gerar G-code.

## Limitações conhecidas

- A união automática de pontas desta etapa opera sobre **polilinhas do Editor**, não mescla ainda caminhos de `CurveDocument` nativo, curvas importadas ou subcaminhos agrupados entre si.
- O novo desenho Bézier produz um contorno nativo separado: o Auto-Join entre curvas Bézier novas e outras formas/contornos ainda não está implementado. Não declarar que todos os tipos de geometria se unem.
- O `Alt+J` está limitado a dois caminhos selecionados com até **0,5 mm de separação**; é uma tolerância configurada na experiência 19.8, não a tolerância padrão de todos os comandos LightBurn.
- Sem comparação visual real dos gestos e sem compilação Windows posterior, comportamento e ergonomia permanecem experimentais.
- Não houve mudança em G-code, CAM, GRBL, M3/M4 físicos ou na controladora.
