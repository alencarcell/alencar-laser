# Alencar Laser Windows 19.8 — Texto editável, Fill vazado, prévia responsiva e alinhamento

**Data:** 29/10/2026  
**Branch:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Status:** implementação e testes publicados no GitHub. **Ainda falta a primeira compilação Windows/Qt6 destas alterações e a validação visual na máquina do operador.** Não mesclar em `main`, não substituir `APLICATIVO`, não alterar firmware ou habilitar CAM/G-code/laser.

## Evidência real recebida

- Foto do editor 19.8: fontes do Windows, controles negrito/itálico, altura em mm, alinhamento e espaçamento estão visíveis, mas o texto anterior não pode ser reaberto para edição — clicar cria um texto novo.
- Foto da prévia OFFLINE: a janela ultrapassa a área útil do notebook; botões de reprodução/progresso/exportação ficam cortados; zoom pelo scroll não funciona.
- O operador requer que um círculo dentro de outro, **na mesma camada Fill**, vire um vazado, tanto na mesa quanto na prévia. Também solicita alinhamentos por bordas e centros como no LightBurn.

## Comportamento documentado no LightBurn

- Texto: https://docs.lightburnsoftware.com/2.1/Reference/Text/ — clicar no objeto com a ferramenta Texto, duplo clique ou Edit Text Shape para reeditar frase e atributos; fonte permanece sem converter para caminhos manualmente.
- Alinhar: https://docs.lightburnsoftware.com/2.1/Reference/Align/ — alinhar esquerda, direita, topo, base, centros verticais/horizontais e ambos, tomando como referência o **último objeto da seleção**, salvo exceções.
- Fill: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/FillMode/ — preenchimento interage por paridade dentro da **mesma camada**. Contorno interno retira material; contorno dentro desse vazado volta a preencher. Camadas Fill diferentes gravam independentemente.
- Zoom: https://docs.lightburnsoftware.com/latest/Reference/ZoomPan/ — roda do mouse amplia/reduz a partir da posição indicada; pan move a câmera, não o desenho.

## Alterações implementadas

### 1. Texto continua vetorizado, mas agora tem origem editável

- `src/editor_tools.hpp`: `EditorState.editableTexts` guarda por objeto raiz: **frase, nome da fonte, negrito, itálico, altura, espaçamento, alinhamento, âncora, limites originais e IDs de todos os contornos**. Não mudou o formato do documento de contornos; metadados opcionais foram adicionados ao `editor-meta-1` já existente.
- Projetos legados sem `editableTexts` continuam carregando (array opcional). **Contornos de textos antigos sem origem salva não são automaticamente convertidos de volta em texto.**
- `Editor::addEditableText` insere contornos e registro em uma única ação Undo, agrupando os glifos em uma unidade. `replaceEditableText` substitui a frase e os contornos em uma única ação Undo, preservando ID raiz e camada. Contornos novos recebem IDs livres. Ao apagar a forma e perder membros, a origem obsoleta é invalidada com segurança.
- `qt6_test6.cpp`: duplo clique próximo ao contorno do texto em Seleção/Texto ou botão **Editar texto selecionado** no painel Texto. Reabre o campo com a frase original e restaura fonte/altura/negrito/itálico/alinhamento/espaçamento; Enter confirma e Esc cancela. Se o objeto foi movido, o editor calcula o deslocamento da origem a partir dos limites existentes.
- **Limitações:** texto ainda é geometricamente vetorizado para a simulação e não é uma entidade de texto nativa completa igual ao LightBurn. Reedição após rotações, distorções ou alteração manual de nós pode regenerar as letras sem preservar o efeito da transformação; testar em cópia. A fonte deve existir no Windows para manter o mesmo visual. Textos antigos já convertidos antes desta atualização não possuem metadados para recuperar o conteúdo com segurança.

### 2. Visualização Filled por camada com buracos

- `qt6_test6.cpp::QtEditorLab::rebuild()`: formas fechadas em camada `mode="Fill"` são reunidas visualmente num `QPainterPath` por camada com `Qt::OddEvenFill`.
- A pintura de fundo é um `QGraphicsPathItem` não selecionável e fica atrás dos contornos, que continuam individualmente editáveis. Não aplica operação booleana destrutiva, não remove formas nem mexe nos nós.
- Núcleo `src/preview_raster_19_8.hpp::appendFill` **já** calcula varredura horizontal par-ímpar na mesma camada; essa rodada corrige a divergência entre a pintura da mesa e a prévia.
- Mudar uma forma para outra camada Fill produz preenchimento independente (não gera buraco entre camadas). Visibilidade e bloqueio continuam respeitados.

### 3. Prévia encaixada no monitor, zoom e pan

- `qt/preview_dialog_19_8.hpp`: tamanho inicial adaptado a `QScreen::availableGeometry()`, em vez de sempre 880×800. Parâmetros, opções e estatísticas estão em `QScrollArea` com altura máxima 265 px; botões Reproduzir/Pausar, velocidade, progresso, exportação e Fechar ficam fora da área rolável.
- Novo `PreviewCanvas198 : QGraphicsView` encapsula imagem da prévia sem alterar plano/tempos. Scroll da roda do mouse em zoom 0,1–20× relativo à vista ajustada, ancorado sob o ponteiro; botão do meio move a câmera; **Ajustar imagem** redefine enquadramento. A reprodução mantém sua sequência temporal já existente.
- **Ainda não há equivalência plena à prévia do LightBurn:** é um modelo offline de linhas e bitmap simplificados, sem potência real, aceleração exata, G-code, emissões ou controladora. A visualização muito densa mantém limite de amostragem de traços, sujeito a revisão posterior.
- A janela ainda precisa ser avaliada com as resoluções reais do notebook e DPI escalado; esta alteração não é prova de que todos os tamanhos ficam visíveis.

### 4. Alinhamentos de formas e grupos

- `Editor::alignObjects(ids,mode,reference)` oferece sete modos: esquerda, direita, topo, base, centro X, centro Y e centros X+Y. Atua em contornos e grupos como unidade, preserva a forma e salva um único histórico Undo.
- Painel **Objetos → Alinhamento**, sete botões; menu **Organizar → Alinhar formas**, as mesmas opções.
- Referência: o objeto ativo/mais recente identificado pelo seletor, ou o último ID da lista quando o histórico de clique não está disponível. É uma aproximação do LightBurn; **a ordem exata de clique da seleção ainda não é preservada explicitamente em todos os modos**.

## Testes adicionados

- `tests/text_alignment_19_8_standalone.cpp`: criação/reedição/Undo/Redo, metadados de fonte, leitura de projetos legados, quatro alinhamentos em C++20, somente memória.
- `qt6_test6.cpp --text-fill-align-19-8-test` exporta `RESULTADO_TEXTO_FILL_ALINHAMENTO_19_8.json`, verificando o ciclo da interface de edição do texto, persistência dos atributos, Undo, compositor visual de dois círculos na mesma camada, alinhamentos e zoom pelo wheel.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao` compila/executa o teste portátil antes do Qt, roda as regressões Qt e exige JSON com status PASS. A suíte `-Abrir` também executa o teste novo. O workflow GitHub experimental inclui C++ portátil.

**Nenhum novo teste foi executado no PC nesta rodada.** Presença de código no GitHub não significa PASS de compilação nem verificação visual.

## Comandos de validação, mantendo versão homologada intocada

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Atualizacao interrompida; preservar arquivos locais" }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao
```

Se PASS, usar `-Abrir` com laser **desconectado** e testar: texto novo + duplo clique + fontes editadas + Ctrl+Z + salvar e reabrir em cópia `.alaser`; anéis Fill mesma e outra camada; janela de prévia 1366×768 e scroll/zoom/arraste, play/progresso/exportação; alinhamentos com duas formas e grupos.

Ao surgir erro de compilação, enviar a **primeira mensagem `error:`** dos relatórios `RELATORIOS/COMPILACAO/BUILD_QT_19_8_STDERR.txt` ou `BUILD_QT_19_8_STDOUT.txt`. Não apagar `.vscode/`, não realizar `git reset --hard`, não substituir a base estável.
