# Alencar Laser — matriz LightBurn × TESTE 19.8 e critérios de homologação (29/10/2026)

> **Escopo e segurança:** comparação de funcionalidades, não cópia de código ou interface. Alencar Laser permanece 100% em português. Nenhum teste/feature deste documento libera GRBL, Wi-Fi de controle, serial, movimentos, M3/M4, potência ou emissão. Máquina laser desconectada. `main` e `APLICATIVO` homologado intocados.

## 1. Fontes e método

Análise combinada das páginas de referência oficiais do LightBurn 2.x, do código na branch `fix/validacao-real-19-8-20261029` e dos testes reais da PR #12. Categorias **Implementado**, **Experimental**, **Ausente/incompleto** ou **Não auditado** referem-se à candidata, NÃO indicam homologação.

Referências:
- Catálogo de funções: https://docs.lightburnsoftware.com/2.0/Reference/
- Seleção: https://docs.lightburnsoftware.com/latest/Reference/Selection/
- Transformação: https://docs.lightburnsoftware.com/latest/Reference/TransformControls/
- Texto e alça de curvatura: https://docs.lightburnsoftware.com/2.0/Reference/Text/
- Tesoura: https://docs.lightburnsoftware.com/2.0/Reference/TrimShapes/
- Traçado de imagem: https://docs.lightburnsoftware.com/latest/Reference/TraceImage/
- Ajustar imagem: https://docs.lightburnsoftware.com/latest/Reference/AdjustImage/
- Prévia: https://docs.lightburnsoftware.com/2.0/Reference/Preview/
- Booleanas: https://docs.lightburnsoftware.com/2.0/Reference/BooleanTools/
- Offset: https://docs.lightburnsoftware.com/latest/Reference/OffsetShapes/
- Camadas, Fill e imagens: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/

## 2. Matriz — o que importa para usar o editor no dia a dia

| Área | Função de referência LightBurn | Estado candidato Alencar Laser | P | Critério |
|---|---|---|---|---|
| Seleção | clique, seleção cruzada/contida, múltipla, grupo | Experimental para polígonos; Bézier com seleção separada; imagem não selecionável | P0 | mesmo comportamento e moldura nas três naturezas de objeto |
| Formas | linha, retângulo, elipse, polígono, desenho por nós | Implementado, ainda em refino | P0 | zoom não encerra gesto; fechamento e undo/redo |
| Tesoura | remover intervalo até próxima interseção; contorno isolado inteiro; destacar antes | Motor C++ e GUI em teste; caso real 575 tem cobertura | P0 | 2 recortes consecutivos exatos, sem apagar vizinhos, 2 Undo |
| Curvas | nós, alças e curvas nativas | Parcial; visual de seleção/tesoura não unificados | P0 | selecionar/mover/escalar/editar e salvar/reabrir 1 Bézier com geometria idêntica |
| Texto | fonte, altura, espaçamento, alinhamento, reeditar | Experimental com metadados textuais, regressões de zoom/ocultação | P0 | clique para reeditar, Esc restaura, Enter confirma; salvar/reabrir e zoom sem salto |
| Texto em caminhos | congelar texto como formas | Implementado em branch, não homologado | P1 | origem textual removida, geometria intocada, 1 Undo restaura |
| Curvar texto | bolinha azul para arco com opção de deformação | Ausente | P1 | frase continua editável e raio persistido; não exigir converter em caminhos |
| Texto em curva | aplicar frase a caminho selecionado | Ausente | P2 | sem perder origem e com Undo seguro |
| Imagens | selecionar, arrastar, dimensionar, camada | Ausente/incompleto — overlay temporário | P0 | objeto com ID, dimensão mm, origem original, Undo |
| Arquivo de imagem | bitmap original e propriedades ao salvar | Ausente no pacote `.alaser` v5 atual | P0 | roundtrip binário e metadados, hashes, compatibilidade legada |
| Traçado | threshold/limpeza/otimização, zoom, Fade, Show Points, preview | Experimental: zoom/pan, perfis, prévia; Fade e Mostrar nós adicionados no código | P1 | preview alinhado com original, fluidez, não altera imagem até confirmar |
| Ajuste de imagem | brilho, contraste, gama, original × resultado sincronizados | Não implementado integralmente | P1 | ajustes não destrutivos, side-by-side e parâmetros persistidos |
| Fotogravação | bitmap/P&B, tramado, DPI/intervalo, overscan | Simulação parcial, sem cadeia física homologada | P1 | raster offline previsível, perfis de material **não calibrados** |
| Camadas | Linha, Fill, Imagem, velocidades, potências, visibilidade e bloqueio | Linha/Fill e metadados experimentais | P0 | vazado correto, camada oculta não participa, roundtrip |
| Operações de formas | soldar, união, subtração, interseção | Não implementadas integralmente | P2 | anéis/furos corretos, sem perda topológica |
| Offset | contorno interno/externo, raio e junção | Ausente/incompleto | P2 | offsets coerentes e prévia segura em peças de MDF |
| Espelhar/distribuir | flip X/Y, alinhar/distribuir objetos | Alinhamento parcial, distribuição/espelho pendentes | P1/P2 | coordenadas exatas, histórico único |
| Rotação | moldura e alças acompanham ângulo | Moldura ainda AABB | P1 | caixa orientada durante e após rotação; Esc cancela |
| Prévia | corte/viagens, zoom livre, timeline, tempo estimado | Geometria virtual funcional; frontend vetorial Qt novo, sem PASS desta rodada | P0 | 1×/5×/10× nítidos, pan, desempenho medido, sem comandos |
| Projetos | salvar, abrir, restaurar e backup | `.alaser` v5 testado para vetores/texto; sem bitmap original | P0 | roundtrip cópia, arquivo original intacto, recuperação após falha |
| Importação | imagens e formatos vetoriais | Bitmap e `.lbrn2` parciais; demais formatos não auditados | P1 | transformações/escala/unidades consistentes |
| Laser físico | origem, framing, máquina, parâmetros reais, geração G-code | BLOQUEADO e não homologado | Fase futura | só depois de homologação separada, intertravamentos e testes físicos |

## 3. Escopo para a **primeira homologação: EDITOR OFFLINE**

**Obrigatórios (P0)**
1. Passar integralmente regressões C++/Qt, sem falhas/interrupções e sem ignorar testes;
2. Seleção, desenho, movimento, zoom, Tesoura, Bézier, Texto/Fill e Undo/Redo coerentes em mouse físico;
3. Projeto `.alaser` v5 real reabre sem perda de vetores, texto ou geometria; quando houver bitmap na sessão, não declarar "salvo com sucesso" se o bitmap não for persistido. Sem imagem como objeto, considerar módulo bitmap **não homologado** explicitamente;
4. Prévia offline nítida a 10×, progressiva e com controle de viagens, sem enviar nada ao hardware;
5. Recuperação e backups comprovados, nenhuma atualização de `APLICATIVO` até aprovação.

**Permitidos como próximos incrementos após editor offline básico**, desde que indicados como não disponíveis: curvar texto por bolinha, efeito de arco, booleanas completas, offset, matrizes, biblioteca de materiais e fotogravação com algoritmos avançados. O trabalho comercial de fotos/logos depende primeiro de imagens como objeto persistente, de modo que **homologação comercial com imagens exige P0 bitmap**.

## 4. Plano de verificação mais rápido e repetível

- **Gate 1 — portátil (CI GitHub):** regressões C++ de texto, Tesoura 19.7/19.8, desenho, Fill, rotas e movimento. CI não prova GUI nem hardware.
- **Gate 2 — Windows focalizado:** `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`, `-SomenteEdicao`, `-SomenteInteracao`, `-SomenteDesenho`, `-SomenteSimulacao`. Gates separados permitem isolar falha em minutos sem alterar a base.
- **Gate 3 — Windows integral:** `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -Abrir`; não chamar de homologado somente pelo PASS sintético.
- **Gate 4 — mouse real:** arquivo `formas trabalhadas...alaser` em CÓPIA; triângulo 575 primeiro/segundo corte/Undo, texto antes/depois do zoom, Bézier seleção/movimento, trace imagem zoom/fade/nós, imagem inserida, preview 10× e salvar/abrir.
- **Gate 5 — resistência:** várias sessões e projetos longos sem corrupção; medir `paint_p95`, `paint_max`, `input_to_paint_p95`, memória e versão. Referência observada 29/10: paint P95 ~54,6 ms e pico ~351,6 ms; metas iniciais orientativas: P95 <=33 ms na máquina de teste e sem picos prolongados >150 ms, exceto carregamentos identificados. Não fingir medição de performance sem relatório real.
- **Gate 6 — candidata/instalador:** backup; smoke test em diretório definitivo; nenhum atalho, patch ou migração sobrescreve `APLICATIVO` sem validação explícita; teste de reversão.

## 5. Otimização do código sem sacrificar estabilidade

- **Hoje:** `qt/qt6_test6.cpp` ~354 KB contém janela, ferramentas, diálogos e muitos testes. Evitar reescrita maciça no final da 19.8. Organizar futura extração por componentes: `trace_preview`, `text_editor_overlay`, `scissor_interaction`, `selection_transform` e `diagnostics`, com alterações pequenas e testes regressivos.
- **Preview:** cache incremental de caminhos Qt; evitar QImage fixo 740×440 para zoom, separar movimento "laser" e "viagem". Comparar P95 com projeto de 8.345 movimentos.
- **Trace:** não recalcular traçado a cada pixel de slider (debounce ~220 ms), preservar zoom e manter nós de prévia limitados a ~2.000 marcadores na tela para arquivos densos; esmaecimento só muda a apresentação, sem alterar origem.
- **Persistência:** imagens e metadados devem ser serializados atomicamente; hashes para validar carga, não alterar arquivo original e garantir fallback ao ler arquivo legado.
- **Qualidade:** quando função mudar comportamento (ex.: cursor Tesoura), atualizar teste junto com código; testes não devem alegar PASS por compilar apenas uma modalidade.
- **Performance:** perfis por operação, medir com `diagnostics`, corrigir hotspots reais antes de micro-otimizações.

## 6. Alterações publicadas nesta rodada (aguardando PASS)

- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1` — `-SomenteTesoura` agora compila/roda Tesoura C++19.7, C++19.8 e GUI Qt em um mesmo gate.
- `qt/qt6_test6.cpp` — na janela de traçado, controles PT-BR **Esmaecer imagem original** e **Mostrar nós do traçado**, para comparar original e vetores; marcadores limitados para evitar congelar desenho denso; regressão GUI de controles.
- `.github/workflows/preview-19-8-offline.yml` — acompanha branch de correções e PRs para base experimental, inclui verificações portáteis Tesoura/texto; não substitui build Qt Windows.
- Sem alteração em engine de corte ou firmware. **Código alterado não compilado e não homologado no Windows nesta rodada.**

**Próxima ação do operador quando voltar:** `git status --short`; se seguro, `git pull --ff-only origin fix/validacao-real-19-8-20261029`; iniciar `-SomenteInteracao` para validar trace/fade/nós, em seguida `-SomenteTesoura`; só então `-Abrir`. Manter laser desconectado e `main`/`APLICATIVO` preservados.


## 7. Complemento da rodada — 29/10/2026

- **Traçado:** a janela agora permite alternar `Imagem original` (bitmap-fonte em visualização redimensionada, preservado fora do traçado) e `P&B` (máscara do resultado); os contornos vetoriais continuam sobrepostos em ambas. Separados controles `Esmaecer referência` e `Mostrar nós do traçado`; todos em português. O comando de criar traçado só confirma geometria após resultado válido, não altera imagem-fonte durante prévia.
- **Testes:** `-SomenteTesoura` agora confere C++ 19.7 + C++ 19.8 + teste Qt. GitHub Actions portátil Linux roda texto, Tesoura e desenho sem hardware, resultado PASS confirmado para commit `63ffc651e` (antes dos últimos ajustes de interface). Em seguida foi adicionado um gate **Windows Qt6/UCRT64 de compilação** com pacotes MSYS2; resultado do novo gate ainda pendente na redação, e nunca equivale a homologação com mouse físico.
- **CI:** workflow `.github/workflows/preview-19-8-offline.yml` usa agrupamento/concurrency para cancelar execuções obsoletas; a instalação `libminizip-dev zlib1g-dev` corrigiu falha anterior de cabeçalho ausente. Qt6 compilado em Windows via `msys2/setup-msys2@v2`; `-Abrir` continua sendo o gate real com aplicação e mouse.
- **Sem mudança de escopo:** recursos complexos — bitmap como objeto persistente no arquivo, seleção Bézier unificada, alça de curvatura de texto, matrizes, booleanas e controle físico — permanecem pendentes. Nenhum dos commits desta rodada foi incorporado à `main` nem à pasta `APLICATIVO`.
