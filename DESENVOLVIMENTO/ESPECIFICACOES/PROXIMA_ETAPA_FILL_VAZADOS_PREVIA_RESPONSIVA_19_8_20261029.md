# Alencar Laser Windows TESTE 19.8 — Próxima atualização: Fill com vazados e prévia responsiva

**Registro de requisito: 29/10/2026.** **Não implementar enquanto o operador valida a alteração de desenho Bézier, snap e Alt+J do lote anterior.** Este documento apenas prepara a próxima rodada de desenvolvimento na branch `feat/refino-curvas-rastreio-diagnosticos-19-8`. Não tocar em `main`, `APLICATIVO`, CAM, G-code nem controle físico.

## Evidências fornecidas pelo operador

Duas fotografias reais do editor Qt:
1. Na mesa, uma forma sólida com pequenos contornos internos (incluindo quadriláteros) que devem representar **vazios de gravação** em uma camada Fill — não nova pintura adicional por cima.
2. Janela `Alencar Laser 19.8 — Simulação OFFLINE`: mostra grade extensa de parâmetros, opções raster, estatísticas e prévia, porém os **comandos de reprodução/slider/fechamento ficam cortados abaixo do limite visível** do computador.
3. A imagem dentro da janela de simulação não aceita **scroll da roda do mouse para zoom**. O operador quer ampliar e reduzir apontando o cursor para uma área do desenho, como no LightBurn.
4. A animação deve mostrar as passagens de preenchimento varridas **linha por linha**, respeitando ilhas vazadas, deslocamentos desligados, intervalo em mm, bidirecionalidade e andamento pelo controle de reprodução.

## Referências oficiais LightBurn (consultadas em 29/10/2026)

- Fill Mode: https://docs.lightburnsoftware.com/2.2/Reference/CutSettingsEditor/FillMode/
  - Fill só se aplica a caminhos fechados.
  - **Mesma camada Fill**: contornos aninhados criam áreas negativas (vazados); trechos parcialmente sobrepostos seguem paridade.
  - **Camadas Fill diferentes**: cada camada é preenchida separadamente, potencialmente gravando áreas sobrepostas duas vezes. Portanto **não** criar vazados automáticos entre layers diferentes.
- Preview: https://docs.lightburnsoftware.com/2.0/Reference/Preview/
  - Roda do mouse, teclado +/− para zoom, botão do meio/Space+arraste para pan e slider temporal com estatísticas.
- View Style: https://docs.lightburnsoftware.com/2.1/Reference/ViewStyle/
  - Visão Filled é distinta da trajetória e não altera o arquivo/origem nem saída física.

**Não prometer equivalência CAM, cinemática ou resultados GRBL**; a 19.8 permanece simulação offline.

## Diagnóstico confirmado na fonte atual (29/10/2026)

- `src/preview_raster_19_8.hpp::appendFill` já possui paridade **even-odd**: agrupa por id da camada Fill, reúne interseções `x` de contornos fechados e gera trechos alternados ativos/desligados. `tests/preview_raster_19_8_standalone.cpp` verifica explicitamente 10×10 externo menos 4×4 interno (`cutMm=84` com intervalo 1 mm). Isso passou nos testes C++ reportados pelo operador em rodada anterior, mas não equivale a validação visual.
- `qt/qt6_test6.cpp::QtEditorLab::rebuild` atribui pincel **independente** a cada `QGraphicsPathItem` fechado com `layer.mode=="Fill"`; isso não compõe vários contornos como **um único caminho com QPainterPath::OddEvenFill**. Resultado visual da mesa pode ser incompatível com a simulação geométrica. Não converter isso em operação booleana irreversível: preservar nós e IDs canônicos.
- `qt/preview_dialog_19_8.hpp`: `QDialog::resize(880,800)`, controles grandes em `QVBoxLayout` sem `QScrollArea`; `QLabel` de prévia (mínimo 280 px), `QImage(740,440)` fixo e `canvas->setPixmap()` sem tratamento de `wheelEvent`. Sliders de tempo, `▶ Reproduzir`, exportação e Fechar são adicionados depois da imagem; podem desaparecer abaixo da área útil.
- A janela já usa `raster::appendFill` quando "Simular Fill vetorial" está marcado; ver o checkbox marcado **não garante** que a camada do objeto esteja em modo `Fill`. O estado de camadas e dos objetos deve ser inspecionado ao validar fotos.

## Escopo obrigatório para a próxima rodada (ordem)

### 1) Composição de vazados coerente: mesa + prévia
- **Mesmo layer Fill:** renderizar visualização composta por camada com regra `Qt::OddEvenFill` e sem obscurecer contornos/nós selecionáveis; grupos de caminhos fechados independentes continuam editáveis.
- Anel externo preenchido; filho interno = espaço vazio; neto interno = preenchido novamente. Sobreposições na mesma camada também seguem paridade, de acordo com a referência.
- Não criar vazados entre layers diferentes implicitamente; respeitar ordem, visibilidade e Output. `T1/T2` permanecem apenas guias.
- Planos `Fill` exigem caminho fechado. Evitar preencher polilinhas abertas ou supor fechamento por proximidade.
- Na prévia temporal, a varredura dentro do vazado é exclusivamente **movimento desligado** (viagem), nunca movimento ativo. Tempo e distâncias devem refletir esse desvio.
- Mostrar estado da camada/explicação curta quando o usuário tiver a opção visual de Fill marcada, mas nenhuma camada estiver realmente no modo Fill.
- **Não alterar `src/preview_raster_19_8.hpp` sem evidência de divergência**, pois o algoritmo even-odd atual já cobre o caso básico. Prioridade real é coerência visual, testes e casos complexos.

### 2) Janela responsiva, controles sempre acessíveis
- Limitar tamanho inicial ao espaço útil da tela (availableGeometry; considerando DPI e barra de tarefas); centralizar dentro da tela.
- Painel superior de parâmetros **rolável ou recolhível** (`QScrollArea` + "Parâmetros avançados"), mantendo **barra de progresso, ▶ Reproduzir/Pausar, velocidade, tempo, exportar e Fechar sempre visíveis** abaixo.
- A área da imagem deve expandir/reduzir com o redimensionamento da janela (mínimo funcional) e não forçar um tamanho fixo de 740×440.
- Conferir 1366×768, 1280×720 e 1920×1080 (com escala 100% e 125/150%) e rolagem quando necessária.
- Usar o próprio botão de reprodução em um `QDialog` sem depender de teclas de atalho globais.

### 3) Zoom, pan e reprodução do Fill
- Roda para zoom **ancorado sob o cursor**, sem alterar dimensões nem coordenadas do projeto, com limites adequados (ex.: 0,1× a 20×); botão do meio + arraste para pan e "Ajustar ao desenho" para reset.
- Reavaliar uso de `QGraphicsView`/widget de pintura escalável em lugar do `QLabel` estático. Zoom deve afetar **só a câmera**, não o plano nem o tempo.
- Mostrar movimentos ativos (Fill vetorial, Line, bitmap) e deslocamentos sem emissão com cores diferentes; nos vazados, desenhar viagens opcionais sem pintar a área vazia.
- O slider e ▶ devem reproduzir a **ordem e o tempo** dos movimentos do mesmo plano que fundamenta estatísticas e JSON. Evitar descartar trechos por `stride` quando isso distorcer visualmente pequenos vazados; usar cache/desenho incremental se necessário.
- Manter limites preventivos de densidade e trabalho para não travar a UI em desenhos com ~575 contornos e mais de 13 mil nós.

## Testes de aceitação a implementar

1. Outer 10×10, inner 4×4 na **mesma camada Fill**, pitch de 1 mm: 100 mm ativos sem vazado, **84 mm ativos** com o vazado; viagens desligadas passam no espaço interno.
2. Três caixas aninhadas da mesma camada (preenchido → vazio → preenchido), dois quadrados parcialmente sobrepostos, formas tangentes e contornos abertos. Medir ativação por segmento e continuidade física virtual.
3. Mesmas duas caixas em **camadas diferentes**: simulação tem preenchimentos separados e não elimina a parte interna do externo.
4. Mesa principal em modo Filled mostra o mesmo vazado que a prévia simulada; mudar para Wireframe ou selecionar contorno não corrompe geometria, seleção, Undo/Redo ou arquivos existentes.
5. Janela em tela pequena: slider, ▶/Pausar, exportação e Fechar ficam acessíveis sem reduzir a escala da interface do Windows.
6. Wheel mouse aumenta/reduz ancorado no ponto apontado; pan e Reset funcionam; slider/tempo/caminho não mudam ao alterar zoom.
7. Animação com intervalo e bidirecionalidade distingue trecho ativo, trecho vazio/laser desligado e overscan visual; estimativa e JSON são coerentes, sem comandos reais.
8. Testes Qt6 + C++20 automatizados; teste manual no Windows do operador com foto/vídeo e máquina desconectada. Não considerar homologado sem execução.

## Limite do compromisso atual

Este registro **não altera o executável nem a lógica de prévia na branch**. O usuário disse que primeiro vai testar o lote já entregue de Bézier, snap e união de pontas. Ao dizer `avance` após trazer o resultado, implementar estas correções com prioridade e executar os testes disponíveis; manter status honesto para o que só pode ser executado no Windows.
