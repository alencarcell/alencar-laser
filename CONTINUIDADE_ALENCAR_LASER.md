# ALENCAR LASER — CONTINUIDADE OFICIAL

## Projeto
Alencar Laser Windows, desenvolvido em C++ e Qt 6.
Pasta oficial: C:\ProjetoAlencarLaser

## Estrutura
APLICATIVO — versão aprovada
DESENVOLVIMENTO — fontes, candidatas e testes
DADOS — projetos, configurações e recuperação
BACKUP — cópias de segurança
RELATORIOS — diagnósticos e auditorias

## Histórico
19.6 — Zoom, layout e diagnóstico Wi-Fi.
19.7 — Seleção, tesoura e camadas.
19.7.1 — Correção do coletor; compilação Windows aprovada.
19.8 — Memória persistente e layout ainda pendentes.

## Estado atual
Estrutura limpa criada em 26/10/2026.
Nenhuma versão instalada na nova pasta APLICATIVO.
Atalhos 01, 02 e 03 ainda não funcionais.
Base 19.7.1 disponível em arquivos anteriores.
Pacote 19.8 do Grok não aprovado.

## Segurança
Não apagar fontes e backups sem auditoria.
Não sobrescrever a base anterior.
Wi-Fi e comandos físicos ainda não homologados.
Preservar projetos .alaser v5.

## Melhorias planejadas
Memória persistente e recuperação automática.
Ícones de ferramentas como no Alencar Laser Web.
Texto curvo, zoom, grade e edição Bézier.
Atualizações incrementais e reversão real.

## Histórico de avanços
- 26/10/2026 — Estrutura definitiva criada no VS Code.

## Próxima etapa
Reconstruir a base 19.7.1 na nova estrutura,
integrar a 19.8 e validar no Windows.

## Continuidade entre chats
Ler este documento antes de modificar o projeto.
Conferir os códigos e relatórios correspondentes.
Registrar mudanças, testes e pendências.

## Marco de versionamento e interface — 27/10/2026
- Repositório privado: `alencarcell/alencar-laser`; branch principal `main` preservada.
- Fontes 19.8 compilados no Windows com Qt6/MSYS2 e quatro testes iniciais aprovados (Bézier: 54 casos, observador GRBL, regras zoom/Wi-Fi e tesoura/camadas); interface abriu no notebook.
- A candidata 19.8 NÃO está homologada integralmente: memória persistente e recuperação ainda precisam de testes práticos. A comunicação física continua não homologada.
- Defeito relatado da Tesoura: após corte, desenho parece manter todo o contorno. A implementação atual abre/divide polilinha, mas não remove um trecho; definir operação de remoção real com teste de regressão.
- Branch `feat/toolbar-vertical-compacta-19-8`: proposta de barra lateral de coluna única e 52–64 px de largura, ícones de 38x36 px e recursos futuros mantidos desativados. Requer compilação/inspeção visual Windows antes de mesclar.
- Camadas permanecem no topo, conforme decisão do usuário.
- Não registrar a proposta visual como funcionalidade homologada até testes e revisão.

## Rodada de testes e correcoes propostas — 27/10/2026
- A barra vertical em coluna unica abriu no Windows e mostrou icones compactos; foram enviados prints dos temas, zoom, grade e camadas.
- Diagnostico TESTE_19.8: Qt 6.12.0, 579 contornos, 13.489 nos, RSS ~102 MiB, scroll P95 0,536 ms, renderizacao P95 59,663 ms, renderizacao maxima 348,158 ms.
- Wi-Fi: botao de conexao intencionalmente desativado no codigo 19.8 apos falhas anteriores; diagnostico indica TCP desconectado e zero probes. Nao liberar conexao fisica ate revisao de estabilidade.
- Tesoura: a operacao legada abre/divide a polilinha, mas preserva as arestas. Nova funcao experimental remove a aresta clicada, com undo/redo e guardas de camadas; ainda requer compilacao e avaliacao visual.
- Ctrl+R ativa retangulo; Ctrl+E ativa elipse; Shift restringe quadrado/circulo. Cursor da Tesoura usa glyph proprio.
- Paleta de 32 camadas passa a permitir criar camadas ao selecionar, com cores por camada no canvas. Camadas continuam no topo.
- Area de desenho e mesa continuam com dimensoes independentes; agora existe controle visual para ocultar o limite da area de desenho, sem alterar a grade nem os limites fisicos.
- Cabecalho mais compacto e tema claro com cores suavizadas; ainda necessita validacao ergonomica.
- Logomarca oficial: substituicao do texto no cabecalho pendente de anexar o arquivo vetorial/PNG original aprovado; nao redesenhar ou inventar logotipo.
- Testes adicionados ao nucleo para remocao de arestas e GUI para proporcao Shift, atalhos e criacao de camada.
- Estas alteracoes estao APENAS na branch feat/toolbar-vertical-compacta-19-8 e no PR #1 em rascunho. Nao foram compiladas nem homologadas. Main permanece preservada.

## Falha de compilacao observada e correcao — 27/10/2026
- Ao compilar a branch experimental no Windows, o g++ retornou varios erros sintaticos no arquivo Qt principal e a compilacao foi interrompida (codigo 1). Nao houve homologacao nem abertura da nova versao.
- Causa identificada no codigo: `if(!busy(){` no tratamento de Esc, em vez de `if(!busy()){`; os demais erros de identificadores eram em cascata.
- Corrigido no commit `432b784`; teste visual recebeu verificacao adicional de que Tesoura remove a aresta, e nao apenas abre caminho.
- Analise estrutural das chaves, parenteses e colchetes do arquivo Qt e dos fontes/testes da Tesoura: consistente. Isso **nao substitui compilacao C++**.
- Proximo passo: executar `git pull --ff-only` e o script Windows de build e testes ja existente. Preservar a branch `main`.

- **27/10/2026 — compilacao Windows interrompida no teste `test19_7_scissors_layers.cpp`, linha 84:** `auto` declarou simultaneamente uma string e uma contagem `size_t`, causando erro de deducao inconsistente. Corrigido no commit `3616752` separando as declaracoes. Ainda pendente de nova compilacao e homologacao; executar `git pull --ff-only` e `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -Abrir` sem limpar a pasta de build.

- **27/10/2026 — build Windows 19.8 avançou:** compilação e testes executáveis Bézier (54/0), GRBL, zoom/Wi-Fi, Tesoura/camadas (incluindo remoção real), policy de recuperação e codec `.alaser` v5 passaram. O teste **GUI de recuperação persistente `--recovery-19-8-test`** terminou com código 1; causa detalhada deve estar em `RELATORIOS/AUDITORIAS/GITHUB_19_8/recuperacao/RESULTADO_RECUPERACAO_19_8.json` no PC do usuário. Não confundir com a regressão separada do codec v5, que passou. O script `SCRIPTS/TESTAR_19_8_WINDOWS.ps1` foi atualizado para imprimir o JSON real em falhas, evitando compilações repetidas sem diagnóstico. Não considerar branch homologada nem mesclar antes de investigar e corrigir o relatório.

- **27/10/2026 — diagnóstico `autosave overwrote original path`:** o teste GUI de recuperação usava um destino fixo dentro da pasta de relatórios (`original_untouched.alaser`) e depois realizava `save()`, deixando o arquivo existente para execuções seguintes. Assim, o teste falhava no `QFile::exists(original)` mesmo sem evidência de sobrescrita pelo autosave. Correção commit `1807894`: cria `QTemporaryDir` exclusivo por execução, sem reutilizar nem excluir os relatórios antigos. A hipótese foi sustentada pela leitura do código e pelo JSON; aguarda nova compilação Windows e execução para confirmar PASS. Ainda não homologado.

## Diagnostico de uso 19.8 recebido — 27/10/2026
- Arquivo analisado: `Alencar_Laser_Diagnostico_TESTE_19_7(1).json`, esquema `alencar-laser-diagnostics-3`, identificacao interna `TESTE_19.8`, Qt 6.12.0, Windows 11.
- Duracao da sessao: 5.833.866 ms (~97,23 min). Coleta ativa, 54 eventos retidos, zero descartados; eventos: 50 `undo`, 2 `shapes_moved`, 1 `shape_created`, 1 `command_error`.
- Documento no instante da coleta: 575 contornos de polilinhas, 13.415 nos e zero contornos Bezier; RAM 105.979.904 bytes (~101,1 MiB).
- Performance: hit-test P95 22,3774 ms; input-to-paint P95 17,4978 ms; paint P50 20,7952 ms; paint P95 56,8687 ms; paint P99 69,4617 ms; paint max 87,1423 ms; wheel P95 0,5347 ms, 587 amostras.
- Maquina fisica nao conectada: transporte `none`, TCP desconectado, zero probes e zero respostas. Nao autorizar envio de comandos fisicos.
- O registro `command_error` nao contem o motivo; o contador de Tesoura `polyline_scissor_edge_removed` nao aparece. Esse JSON nao confirma se a remocao de aresta foi usada nem reproduz o comportamento visual de LightBurn.
- Comparacao de dois videos enviados pelo usuario (LightBurn versus Alencar Laser) segue pendente de inspecao quadro a quadro; nao inferir o comportamento exclusivamente pelos eventos agregados.
- Logo apresentada pelo usuario: iconografia metalica/prata com laranja/laser; retirar slogan "Precisao para suas criacoes". Aguardar uso do arquivo grafico aprovado no aplicativo Qt (nao redesenhar).
- Proxima verificacao: reproduzir tesoura sobre o mesmo tipo de contorno nos dois aplicativos, inspecionar antes/depois/undo e observar se o objeto original permanece; correlacionar `command_error` com acao sem exportar informacoes sensiveis.
- Esta atualizacao e documental, nao certifica correcoes funcionais nem homologacao da branch.

## Tesoura — correção de escopo funcional — 27/10/2026
- Usuário esclareceu a referência LightBurn: Tesoura selecionada mantém seta normal até pairar sobre trecho de recorte válido; então vira tesoura, destaca **todo o trecho** que será removido e um clique elimina integralmente aquela região.
- A 19.8 atual remove apenas a aresta `hit.segment`, dependente do número de nós; por isso falha com curvas poligonais densas e alguns trechos.
- Próxima Tesoura deve operar por **intervalos de contorno delimitados por interseções/extremidades**, com preview e cursor condicionais; sem candidatos inequívocos não deve fazer recorte parcial.
- Criada branch isolada `fix/scissors-continuous-span-19-8` e especificação `DESENVOLVIMENTO/ESPECIFICACOES/TESOURA_TRECHO_CONTINUO_19_8.md` com critérios A1–A10.
- **Somente especificação/planejamento nesta branch até aqui:** ainda sem implementação funcional ou aprovação Windows; não apontar como defeito resolvido. Main e candidata testada preservadas.

## Avanco Tesoura continua — 27/10/2026
- Branch independente `fix/scissors-continuous-span-19-8` derivada da `feat/toolbar-vertical-compacta-19-8`. `main` preservada.
- Novos arquivos de engenharia: `src/scissors_span.hpp` e `tests/test19_8_scissors_span.cpp`. `geometry.hpp`, `editor_tools.hpp`, `qt/qt6_test6.cpp`, CMake e script de teste Windows alterados.
- A Tesoura deixou de chamar `removePolylineEdge` na GUI; passa a calcular e remover o intervalo contínuo entre interseções com preview laranja e cursor condicional (seta fora, tesoura sobre trecho válido).
- Inclui busca por caixas usando BVH, identificação de autocruzamentos, filtros de visibilidade das camadas e recusa localizada de tangências/sobreposições ambíguas.
- Testes C++ de linhas densas, elipse 64 arestas, autocruzamentos, camadas invisíveis/bloqueadas e undo/redo adicionados; teste GUI de hover / cursor / recorte completo atualizado.
- **Estado real:** alterações foram inspecionadas estruturalmente, mas `build Windows` e testes executáveis ainda NÃO realizados para essa branch. Não declarar homologado, não mesclar em main, não reativar Wi-Fi nem comandos físicos.
- Próximo passo: no VS Code, trocar para a branch e executar `powershell -NoProfile -ExecutionPolicy Bypass -File .\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1 -Abrir`; corrigir logs de compilação e verificar vídeo antes/depois.

## Compilacao 19.8 — falha de hover circular e hotfix — 27/10/2026
- O usuario executou corretamente `git fetch origin`, mudou para `fix/scissors-continuous-span-19-8` e compilou no Windows. Todos os testes de nucleo passaram: Bezier 54/0, GRBL, zoom/Wi-Fi safety, Tesoura legada, nova Tesoura por intervalos, recuperacao policy e codec v5.
- O teste GUI parou com `hover nao iluminou arco completo` (resultado `RESULTADO_TESTE_19_7_GUI.json`). Nao foi falha de compilacao.
- Causa confirmada por leitura de codigo: `ScissorSpanPlanner::candidate`, em contorno fechado e cursor antes do primeiro cruzamento, usava `hi=edges.front()+m` enquanto `lo=edges.back()-m`. Isso produzia intervalo maior que uma volta e a funcao recusava preview.
- Corrigido: antes do primeiro cruzamento `hi=edges.front()`; apos o ultimo `hi=edges.front()+m`. Corrigido no commit `ad89dd3`.
- Criados testes de regressao do caso concreto: retangulo cruzado horizontalmente e mouse na aresta superior (intervalo que atravessa origem), verificacao geometrica de trecho inteiro, undo e cursor do ultimo arco, commit `169fd0b`.
- Estado: hotfix revisado estaticamente, **ainda nao executado no Windows apos correcao**. Proxima acao: `git pull --ff-only` e script existente `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -Abrir`. Nao homologar nem mesclar PR #2 sem PASS Qt e validacao manual.

## Rodada seguinte de hover — 27/10/2026, segundo FAIL
- Após os commits ad89dd3 e 169fd0b, usuário recompilou no VS Code; todos os testes do núcleo passaram novamente (incluindo nova Tesoura por intervalos, Bézier 54/0, GRBL e codec .alaser v5). **Teste GUI ainda falhou:** "hover nao iluminou arco completo", código 1; sem abertura do app por validação segura.
- Não tratar essa falha como resolvida e não criar mais hotfix geométrico por hipótese: o cenário geométrico de retângulo cortado já passa no teste de núcleo. Precisamos discriminar evento de mouse, posição viewport/scene, pick, planejamento e overlay na GUI Qt.
- qt6_test6.cpp recebeu contador de atualizações hover e mensagem diagnóstica em caso de FAIL com posição/viewport/scene, número e identificação de hits, candidatos, número de pontos do preview, cursor e item gráfico. Também salva captura de falha FALHA_HOVER_TESOURA.png.
- SCRIPTS/TESTAR_19_8_WINDOWS.ps1 agora aceita -SomenteTesoura para compilar somente alencar_test7_qt6 e executar o teste GUI em RELATORIOS/AUDITORIAS/GITHUB_19_8/tesoura_foco; **não é homologação completa**.
- Próxima execução no VS Code: git pull --ff-only; powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura. O erro detalhado aparece direto no terminal sem pedir arquivo.
- Ainda sem teste Windows desses diagnósticos, não declarar funcional. Main e demais branches preservadas.

## Terceira rodada Qt — diagnóstico conclusivo do hover 27/10/2026
- Captura real Windows: test GUI \`FAIL: hover nao iluminou arco completo\`; \`hover_events=0 mode=2 native=0 busy=0 point=(245,159) viewport=(891,479) scene=(110,100) hits=1 hit0=(object:1,edge:0,candidate:1,pts:4) preview=0 overlay=0\`.
- **Conclusão factual:** a posição, busca e cálculo do arco estão corretos (há candidato com 4 pontos). O teste de mouseMove não chegou a \`updateScissorHover\`; não é justificável mexer novamente na geometria do arco.
- Correção experimental em \`qt/qt6_test6.cpp\`: habilita explicitamente \`viewport()->setMouseTracking(true)\` e \`Qt::WA_Hover\` no filho da \`QGraphicsView\`; teste gera dois movimentos distintos, verifica recepção por contador e imprime tracking/hover do viewport caso ainda falhe.
- Não houve nova compilação Windows nesta revisão; \`main\` permanece preservada e o PR #2 segue em rascunho. Próxima ação \`git pull --ff-only\`, seguido de \`powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura\` e enviar log. Caso passe, exigir teste GUI completo, validação visual manual e somente depois considerar merge.

## Quarta rodada Qt — QTest mouseMove sem botão não gera evento — 27/10/2026
- Log real Windows confirmou: compilação da interface concluída, \`hover_events=0\`, \`tracking=1\`, \`hover_attr=1\`, \`hits=1\`, \`candidate=1\`, \`pts=4\`, \`preview=0\`. Não alterar novamente a geometria, pois o cálculo identificou corretamente o intervalo.
- Alteração experimental em \`qt/qt6_test6.cpp\` (commit \`0da3fcb\`): o teste GUI chama \`QTest::mouseMove\` e, caso **não incremente o contador**, despacha um \`QMouseEvent(QEvent::MouseMove)\` via \`QApplication::sendEvent(viewport())\` para testar o caminho Qt de \`viewportEvent\` até \`QtEditorLab::mouseMoveEvent\`.
- Se nem a via direta funcionar, o relatório inclui \`hover_events\`, rastreamento, coordenadas e candidatos. Caso passe, o relatório \`RESULTADO_TESTE_19_7_GUI.json\` inclui \`hover_delivery\` com \`viewport_synthetic_mouse_move\` ou \`qtest_mouse_move\`.
- **Um PASS com evento Qt sintético NÃO comprova comportamento do mouse real do notebook.** Após passar, executar suíte completa e validar manualmente cursor condicional, destaque laranja e recorte no desenho real. \`main\` preservada, conexão física bloqueada.
- Executar no VS Code: \`git pull --ff-only\`; \`powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura\`.

## Validacao Windows da Tesoura — 27/10/2026: teste focado PASS
- Captura enviada pelo usuario no VS Code apos `git pull --ff-only` e `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`.
- Saida verificada: CMake configurou, Ninja compilou e vinculou `alencar_test7_qt6.exe`; terminal terminou com `Teste focado passou; validacao completa ainda obrigatoria.` e retornou ao prompt PowerShell sem erro.
- Os warnings `-Wmisleading-indentation` em `lightburn_reference_inspector.hpp` e `qt6_test6.cpp` nao impediram a compilacao. Corrigir depois sem misturar com a homologacao da Tesoura.
- Nao consta no print se `hover_delivery` foi `viewport_synthetic_mouse_move` ou `qtest_mouse_move`; consultar o JSON do teste quando necessario.
- **Ainda pendente:** suite completa `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`, seguida de validação manual com mouse fisico no desenho real, verificando seta no vazio, cursor Tesoura no trecho, preview laranja do arco inteiro, clique remove sem fragmentos, Ctrl+Z e Ctrl+Y.
- PASS focado nao homologa desempenho em arquivos reais, nem conexao Wi-Fi; maquina deve permanecer desconectada e `main` preservada.

## Ampliação solicitada — arraste, LBRN2 e Tesoura isolada — 27/10/2026
- O usuario confirmou na interface Windows que a Tesoura melhorou, mas circulo/retangulo novos isolados nao eram reconhecidos; informou tambem necessidade de arrastar arquivos e abrir `.lbrn2`.
- Documentacao oficial LightBurn Trim Shapes indica: forma sem intersecoes pode ser destacada por inteiro e excluida num clique; mesmo para formas primitivas como retangulo e elipse. `Ctrl+K` e o atalho documentado.
- `src/scissors_span.hpp`: alterado para retornar candidato de intervalo completo em contorno sem intersecoes, inclusive fechado; exatamente 1 intersecao de forma fechada segue ambigua. Testes de circulo e retangulo isolados no nucleo e no Qt adicionados.
- `qt/qt6_test6.cpp`: Ctrl+K adicional, dragEnterEvent/dropEvent para UM arquivo local `.alaser` ou `.lbrn2`; Arquivo > Abrir tambem reconhece `.lbrn2`. Confirma substituicao e protege projeto nao salvo. Nenhuma conexao com laser.
- Novo `src/lightburn_geometry_import.hpp`: leitor parcial/conservador XML LightBurnProject; Rect, Ellipse, Group e Path simples em `VertList`+`PrimList` com L e B (Bezier amostrada em 24 segmentos por cubica), XForm e CutIndex 0..31. Cria copia editavel somente após decodificacao integral. Nenhuma operacao de laser importada. Recusa qualquer estrutura nao compreendida, paths compostos desconexos, espelhamentos MirrorX/MirrorY ativos, geometrias com limites excessivos e cantos arredondados. Nao apresenta compatibilidade completa com LBRN2.
- Regressoes de importacao Qt com fixtures sintéticas de retangulo, elipse, grupo, linhas, Bezier, layers, rejeicao de formato parcial e dragEnter foram adicionadas; script Windows recebeu `-SomenteImportacao` e teste automatico na suite completa.
- Importacao de bitmap, texto, referencias compartilhadas/variantes de caminhos complexos e reflexoes: PENDENTES de exemplo real do usuario, sem criar arte incompleta silenciosamente.
- Relatorio real da nova sessao: 575 contornos, 13.415 nos, 7 shape_created, 2 shapes_moved, 11 undo; sem conexao fisica. Render P95 ~58,83ms, P99 ~171,27ms e pico ~814,14ms: monitorar engasgos, sem conclusao de performance homologada.
- Estado da revisao: **commits no GitHub apenas; nao foi compilada nem testada no Windows depois destas alteracoes.** Branch experimental `fix/scissors-continuous-span-19-8`, PR #2 ainda rascunho, `main` nao alterada. Proxima acao: `git pull --ff-only` e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`; se falha, consultar JSON/importacao; nao homologar sem mouse e arquivo .lbrn2 reais.
- Fontes referencia: https://docs.lightburnsoftware.com/latest/Reference/TrimShapes/ ; https://docs.lightburnsoftware.com/latest/Reference/FileManagement/ ; https://forum.lightburnsoftware.com/t/lbrn2-file-documentation/52174 ; https://github.com/tscircuit/laser-deformation-fix .

## Correção do teste de Tesoura diante de camada oculta — 27/10/2026
- O usuario executou a suite completa no VS Code apos implementacao de drag-drop/.lbrn2 e Tesoura de forma isolada. O Ninja compilou os alvos e os testes iniciais passaram (54/0 Bézier, GRBL, regras, Tesoura 19.7), mas o executavel `alencar_test19_8_scissors_span` retornou `FAIL: contorno invisivel gerou limite de Tesoura`.
- Diagnostico de codigo: o teste de 19.8 continha uma assercao antiga que esperava NENHUM candidato se houvesse um retangulo isolado cruzado somente por uma linha numa camada oculta. O comportamento novo/esperado de Trim Shapes retorna agora o retangulo INTEIRO (5 pontos no preview), ja que a linha invisivel nao participa das intersecoes.
- Atualizado APENAS o teste `DESENVOLVIMENTO/FONTE/tests/test19_8_scissors_span.cpp` (commit `eda7d25`). Agora valida preview da forma inteira, remocao somente do retangulo visivel, conservacao da linha na camada oculta e restauração via undo sem reexibir a camada.
- **Sem validacao Windows apos este commit.** Proxima acao: `git pull --ff-only` e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`. A compilacao anterior nao foi erro; manter a branch experimental sem merge. Testar LBRN2 real e arraste na GUI somente quando a suite passar.

## Rodada de teste 19.8 — PASS geometrico / FAIL limpeza hover — 27/10/2026
- Captura Windows apos atualizacao da Tesoura e importacao LBRN2: todos os testes anteriores passaram (Bezier 54/0, GRBL, regras, 19.7, 19.8 continuous span, recuperacao e codec v5). O teste Qt GUI interrompeu com `Tesoura permanece ativa fora do trecho`. A fase de testes da importacao LBRN2 ainda nao foi executada nesta rodada, pois a suite interrompeu antes.
- Analise do codigo: `updateScissorHover` ja invoca `clearScissorHover` se a busca nao encontra candidato. Entretanto, o teste `moveHover` aceitava apenas incremento de contador, sem confirmar se `QTest::mouseMove` entregou **o ponto exato solicitado**; evento atrasado do Qt pode ser contabilizado apesar de estar na coordenada anterior.
- Commit `4604ea6` registra `scissorLastHoverPosition_`, acrescenta `scissorHoverReceivedAt`, exige a coordenada correta antes de considerar evento Qt entregue, usa um evento sintético via `viewport` se a coordenada for outra, e inclui diagnóstico de `last_hover` e captura `FALHA_SAIDA_TESOURA.png` caso o hover continue preso.
- **Isto corrige uma vulnerabilidade da simulação do teste, mas ainda NÃO está comprovado ser a causa da falha no Windows.** Se voltar a falhar, usar `last_hover`, `hits`, `candidate` e captura para distinguir erro real da GUI.
- Main preservada, PR #2 em desenvolvimento, hardware bloqueado. Proxima acao `git pull --ff-only` e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura`; apos PASS, executar `-Abrir` para suite completa e teste de arrastar/importar LBRN2.

## PASS GUI focado — hover com coordenada correta — 27/10/2026
- Usuario enviou captura do terminal apos compilar `alencar_test7_qt6.exe` e executar `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`. Resultado verde: `Teste focado passou; validacao completa ainda obrigatoria.` PowerShell retornou normalmente.
- Confirmacao limitada ao **teste automatizado focado Qt**. O teste anterior de nucleo `alencar_test19_8_scissors_span` tambem havia passado, incluindo circulos e retangulos isolados, cruzamentos, elipses e desfazer/refazer.
- Warnings sem bloqueio observados: `-Wmisleading-indentation` em `lightburn_reference_inspector.hpp` e `qt6_test6.cpp`, e construtor `QDragEnterEvent` obsoleto num teste. Deixar limpeza posterior, sem misturar com validacao funcional.
- Ainda SEM PASS da suite completa apos esse commit, SEM verificacao de mouse real, SEM confirmacao de arrastar soltar arquivo real e SEM compatibilidade LBRN2 em trabalho real.
- Proxima execucao: `git pull --ff-only` seguido de `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`; testar Tesoura manualmente com laser desconectado. Se falhar importacao, priorizar JSON `RELATORIOS/AUDITORIAS/GITHUB_19_8/importacao/RESULTADO_IMPORTACAO_LBRN2.json`.
- Branch `fix/scissors-continuous-span-19-8`, PR #2 ainda experimental; main intocada.

## Rodada Qt: teste focado da Tesoura PASS após validação das coordenadas — 27/10/2026
- Usuário mostrou nova execução Windows/VS Code com **`[2/2] Linking CXX executable alencar_test7_qt6.exe`** e a mensagem final **`Teste focado passou; validacao completa ainda obrigatoria.`**; retorno ao prompt PowerShell sem falha.
- A mudança anterior (commit `4604ea6`) passou o teste automatizado de cursor e remoção da prévia após sair do trecho. Ainda **não equivale à homologação com mouse físico**.
- Warnings de `-Wmisleading-indentation` em `lightburn_reference_inspector.hpp` e `qt6_test6.cpp` e aviso de construtor `QDragEnterEvent` obsoleto não impediram a compilação; limpeza deve ser feita separadamente.
- Próximo teste: `git pull --ff-only` e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`. Este roda a suíte completa, incluindo importação geométrica `.lbrn2` em fixtures, e abre a candidata apenas se passar. Testar mouse físico em retângulo/círculo isolados, contornos cruzados e arraste de arquivos reais. Máquina desconectada; nenhuma homologação até então.

## 27/10/2026 — nova branch Texto/Lapis/Logo/LBRN2
- Nova branch isolada `feat/texto-lapis-importacao-logo-19-8`, derivada da candidata Tesoura `fix/scissors-continuous-span-19-8`. `main` intacta, sem merge e sem novo ZIP.
- Usuário relatou no vídeo e mensagem: Tesoura melhorou mas falha em certas linhas; retângulos e círculos novos já cortam; arquivos .lbrn2 reconhecidos pela caixa de abertura mas não importados em alguns casos; logo pedida ausente; solicitou liberação das ferramentas Texto e Lápis.
- Implementado Editor::addContours (transação única), Tool::Pencil com desenho livre/polilinha aberta, atalho P, Tool::Text com fonte, altura em mm e contornos vetoriais da fonte, atalho T. Interface possui dois novos ícones e testes Qt em `--drawing-tools-19-8-test`.
- Tesoura com raio de hit-test de 9 px (antes 6 px) somente no modo recorte; lacunas topológicas permanecem, não prometer equivalência completa ao LightBurn.
- Logo fornecida pelo usuário está disponível localmente como `logotipo_neon_alencar_laser.png`; código exibe a imagem oficial do caminho `C:\\ProjetoAlencarLaser\\DADOS\\LOGO\\logo_alencar_laser.png`, fallback textual se não instalada. Script `SCRIPTS/INSTALAR_LOGO_OFICIAL.ps1` copia de Downloads com backup e validação SHA-256. A imagem binária NÃO foi incluída no GitHub nesta atualização; necessita download e execução do script.
- LBRN2 sem arquivo real anexado: quando parser rejeitar, agora exibe janela clara do motivo, sem alterar o documento; `SCRIPTS/ANALISAR_LBRN2.ps1` produz inventário de tipos de Shape sem dados de desenho; importador continua parcial.
- Regressões novas em `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteFerramentas` e na suíte `-Abrir`. **Ainda não compiladas nem testadas no Windows desde essas mudanças**.
- Especificação em `DESENVOLVIMENTO/ESPECIFICACOES/TOOLS_TEXTO_LAPIS_LOGO_LBRN2_19_8.md`. Antes de homologar, conferir visualmente texto vazado, desenho livre, recuperação e importação de .lbrn2 real. Máquina desconectada; emissão bloqueada.

## 27/10/2026 — PASS Windows focado Texto/Lápis e logo instalada
- Captura do VS Code enviada pelo usuário: CMake/Ninja compilou e vinculou `alencar_test7_qt6.exe` (2/2); teste `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteFerramentas` exibiu **"Texto/Lapis passaram nos testes sinteticos. Teste manual ainda necessario."**, retornando ao prompt sem erro.
- Após falha inicial por ausência da imagem em Downloads, o usuário executou `SCRIPTS/INSTALAR_LOGO_OFICIAL.ps1` com a imagem disponível. Terminal confirmou **"Logo oficial instalada com SHA-256 conferido: C:\\ProjetoAlencarLaser\\DADOS\\LOGO\\logo_alencar_laser.png"** e pediu reabertura do aplicativo para atualizar cabeçalho.
- Warnings de indentação enganosa e API depreciada `QDragEnterEvent` aparecem, sem bloquear compilação. Não considerar tais avisos erros fatais, mas programar limpeza.
- Estado: **testes sintéticos Texto/Lápis PASS; logo instalada/com hash verificado. Ainda NÃO homologados com mouse físico/edição real, nem validado layout da logo, nem importador LBRN2 real.**
- Próxima etapa no VS Code: `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`. Se os gates passarem, testar texto T (fonte e tamanho, vazados de letras), lápis P (arraste/undo), logo no cabeçalho, arraste/importação LBRN2 em cópia, mantendo laser desconectado.

## 27/10/2026 — confirmação do teste focado Texto/Lápis e da instalação da logo
- O usuário enviou captura do terminal VS Code, na branch `feat/texto-lapis-importacao-logo-19-8`.
- A compilação Qt concluiu: `[2/2] Linking CXX executable alencar_test7_qt6.exe`, e o script `-SomenteFerramentas` terminou com `Texto/Lápis passaram nos testes sintéticos. Teste manual ainda necessário.` Voltou ao prompt sem erro.
- O script `SCRIPTS/INSTALAR_LOGO_OFICIAL.ps1` confirmou `Logo oficial instalada com SHA-256 conferido: C:\\ProjetoAlencarLaser\\DADOS\\LOGO\\logo_alencar_laser.png`, seguido de orientação para reabrir o aplicativo. A imagem original já está no caminho definitivo: não pedir novamente download.
- Permanecem avisos não fatais `-Wmisleading-indentation` e `QDragEnterEvent` obsoleto. Limpeza técnica separada, sem bloquear a rodada.
- **Ainda pendente:** teste completo `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`; depois validar manualmente no aplicativo a logo, Lápis, Texto, Tesoura em contornos finos, e a leitura/arraste de `.lbrn2` real. Suporte LBRN2 continua parcial e não homologado; textos convertidos para contornos não são editáveis como palavras.
- `main` preservada; sem confirmação de funcionamento real do mouse, emissão laser bloqueada, não fazer merge antes de homologar.

## 27/10/2026 — regressão GUI: falha de destaque em retângulo isolado
- Captura Windows: execução completa `TESTAR_19_8_WINDOWS.ps1 -Abrir` compilou e passou todos os testes de núcleo (Bézier 54/0, GRBL, regras, Tesoura 19.7, Tesoura 19.8, recuperação e codec v5).
- Parou no teste Qt `Regressao GUI das ferramentas`, com JSON `{"error":"Tesoura nao destacou retangulo novo completo","status":"FAIL"}`. Assim **não abriu a candidata**, e os testes subsequentes de importação LBRN2 e Texto/Lápis completos ainda estão pendentes.
- Leitura do cenário em `qt/qt6_test6.cpp`: o próprio teste cria círculo e retângulo isolados e utiliza helper `pointHover`; este aceitava QUALQUER incremento de `scissorHoverEventCount()`, mesmo que fosse evento assíncrono da coordenada anterior. Este padrão já causara outras falhas de simulação na Qt Windows. Não há evidência de falha no cálculo de contornos isolados: os testes do núcleo passaram.
- Commit `79da9cb`: helper passa a exigir `scissorHoverReceivedAt(at)` e despachar evento pela viewport quando QTest não entrega a coordenada pretendida; asserções exigem `ScissorSpan::key.object` do círculo e do retângulo, não apenas quantidade de pontos; falha grava `FALHA_RETANGULO_TESOURA.png` e informa `scissorHoverDiagnostics(at)` (hits/candidato/posição/preview).
- **Ainda não compilado nem executado no Windows depois desse commit.** Executar primeiro `git pull --ff-only` e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura`. Se passar, executar novamente com `-Abrir`. Se falhar, conferir o erro detalhado sem alterar geometria indiscriminadamente.
- Branch `feat/texto-lapis-importacao-logo-19-8` preservada, `main` intacta e laser desconectado.


## 28/10/2026 — novo bloco experimental Texto/Alças/Curvas

- Usuário confirmou avanços com editor multimodal, mas solicitou escrever texto diretamente no canvas sem diálogos, alças de escala proporcional/livre e rotação, suavização de curvas e evolução da Tesoura.
- Documentação LightBurn verificada em TransformControls, Text e TrimShapes. Regras: escala por canto proporcional por padrão; Shift libera largura/altura; Ctrl altera a âncora para o centro; Tesoura de grupos exige desagrupar.
- Branch isolada `feat/texto-direto-transformacoes-19-8` derivada de `feat/editor-multiferramentas-19-8`; `main` intocada.
- Texto: campo QLineEdit transparente sobre a viewport da mesa, Enter/Esc finaliza, commit atômico de contornos com undo. **Limitado a uma linha e convertido a caminhos após concluir, sem semântica de texto editável.**
- Transformação: 8 alças de tamanho e quatro de rotação em seleção de polilinhas; prévia visual sem alterações na geometria canônica; commit atômico em release; Esc cancela.
- Elipses novas: amostragem elevada de 64 para 512 nós. **Ainda são polilinhas e podem facetar com zoom extremo**; solução definitiva requer arcos/Bézier nativos.
- Tesoura: permanece conservadora. Cruzamentos em vértices/tangências/colinearidade e curvas Bézier nativas seguem restritos; exigir arquivo real antes de modificar tolerância/topologia.
- Adicionado teste GUI `--inline-transforms-19-8-test`, e `-SomenteInteracao` ao script `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`, também inserido na suíte completa.
- **Estado real: código publicado no GitHub, ainda NÃO compilado/testado no Windows nesta nova branch.** Manter máquina desconectada, emissões bloqueadas; não mesclar nem homologar.
- Especificação detalhada em `DESENVOLVIMENTO/ESPECIFICACOES/TEXTO_TRANSFORMACOES_CURVAS_19_8.md`.
- Próximo comando após revisar mudanças locais: `git fetch origin`; `git switch --track origin/feat/texto-direto-transformacoes-19-8`; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`; depois `-Abrir` se PASS.


## 28/10/2026 — 19.8: Texto sem overlay, Desagrupar para Tesoura, Camadas Line/Fill (PR #7)

- Relato: texto transparente vazio permanecia após Seleção; grupos bloqueavam Tesoura; pedidas camadas com atribuição de formas, Linha/Preenchimento.
- Consulte a documentação LightBurn: Cuts/Layers Window, Grouping, Cut Settings Editor e Sub-Layers. A seleção de operações por cor e Ctrl+U são referências; Fill+Line antigo foi substituído por subcamadas.
- Branch isolada `feat/camadas-preenchimento-desagrupar-19-8`, PR #7 em rascunho; **main e APLICATIVO intocados**.
- Implementado na branch: troca de ferramenta e perda de foco encerram overlay de texto vazio sem escrever no documento; texto não vazio confirmado; largura de campo ajustável; atalhos de edição suspensos enquanto digita.
- Editor::ungroupSelection desagrupa todos os grupos tocados pela seleção em um único undo, conserva geometria e ID; Ctrl+U, menu contextual e botão "Desagrupar → Tesoura" sem cortar automaticamente.
- EditorState::Layer ganhou `mode` (Line/Fill), salvo em metadata `editor-meta-1`; leitura legada assume Line, com undo/redo. UI tem seletor de modo da camada; a paleta e o botão de envio já existentes transferem formas.
- Prévia Fill só pinta formas fechadas semitransparentes. **Não representa parâmetros, hatch, raster real, potência, velocidade ou saída física; Line+Fill moderno requer projeto posterior de subcamadas.** Letras com vazados podem divergir; não produzir com base na prévia.
- Testes: novo `--layers-ungroup-19-8-test` e `-SomenteCamadas`, integrado ao `-Abrir`; verificação de campo vazio adicionada a `--inline-transforms-19-8-test`.
- Estado: **GitHub atualizado, verificação estrutural de delimitadores PASS, mas SEM compilação Windows / SEM mouse físico.** Não homologar até ambos testes focados, suíte integral e revisão com projetos reais.
- Documentação: `DESENVOLVIMENTO/ESPECIFICACOES/CAMADAS_LINHA_FILL_DESAGRUPAR_19_8.md`.
- Comandos sugeridos a partir da raiz após confirmar estado do Git: `git fetch origin`; `git switch --track origin/feat/camadas-preenchimento-desagrupar-19-8`; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteCamadas`, depois `-SomenteInteracao` e então `-Abrir` se ambos PASS. Em conflito de arquivos locais: preservar antes de atualizar.


## 28/10/2026 — arquivo real TESOURA e gesto central — PR #8 em rascunho

- O arquivo v5 real `TESTE TESOURA.alaser` enviado nesta etapa foi inspecionado localmente; SHA-256 `711c9cf4cf103611c9c208394a4b7ac33fe9a1a2ca1a05ca22286a844ba48026`. A arte original NÃO foi adicionada ao repositório.
- Diagnóstico conclusivo de metadados: 575 contornos, 13.415 nós, nenhum objeto agrupado e nenhum Bézier; todos `closed=false`, apesar de **571** terem primeiro e último vértice exatamente iguais. Há outros 3 genuinamente abertos e 1 caminho com 2 nós a 0,002525 mm do primeiro.
- Nova branch `fix/tesoura-fechos-lbrn2-selecao-19-8`, PR #8 draft, sobre `feat/camadas-preenchimento-desagrupar-19-8`. Ainda não houve compilação Qt Windows nesta branch. Não mesclar nem homologar.
- Em `scissors_span.hpp`, reconhecer cópia virtual de anel fechado no hover sem tocar nos dados. No corte, `removeScissorSpan` usa a mesma cópia lógica somente no objeto clicado; undo exato. Tangências/colinearidades permanecem bloqueadas.
- Qt: alça central da seleção agora arrasta e move toda a seleção, Shift limita a 45°, prévia transacional. Paleta de cores segue LightBurn: clique simples com seleção transfere camada; sem seleção ativa; Shift seleciona objetos da cor. Fill mostra só contornos por padrão, com checkbox para prévia translúcida (ainda não é gravação real).
- LBRN2: importador permanece parcial, mas suporta `Rect` com cantos arredondados `Cr` e elipses com 384 amostras. **Nenhum .lbrn2 real foi anexado nesta mensagem**: necessita arquivo real recusado para ampliar cobertura de Text/Bitmap/paths complexos. Sem importação parcial silenciosa.
- Novo teste Qt `--scissor-closures-19-8-test` + switch `-SomenteCasoReal` e inclusão no gate integral; casos de anel isolado e cruzamentos, hover não destrutivo, undo, centro, paleta e XML sintético de Rect Cr.
- Validar no Windows primeiro `git status --short`, `git fetch origin` e (sem mudanças locais pendentes) `git switch --track origin/fix/tesoura-fechos-lbrn2-selecao-19-8`. Depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteCasoReal`. Com PASS, rodar `-SomenteCamadas`, `-SomenteImportacao`, `-Abrir`.
- Testar mouse no `TESTE TESOURA.alaser` original aberto em cópia de trabalho e fazer Ctrl+Z; não conectar a máquina física. O relatório técnico está em `DESENVOLVIMENTO/ESPECIFICACOES/CASO_REAL_TESOURA_20261028.md`.


## 28/10/2026 — LBRN2 real SUPORTE PRA ALEXA (PR #9 em rascunho)

- Arquivo recebido/analisado: `SUPORTE PRA ALEXA_backup.lbrn2` (42.185 bytes, LightBurn 2.1.00, sha256 `3336269e9bae2aa5cb2778123ff7ffea2ea1a8a515d2aca648fb44e22ebbad06`). Original do usuário preservado, NÃO inserido no GitHub.
- Resultado da análise independente Python: 20 Shape XML = 13 Path, 4 Rect, 3 Group; saem 17 formas editáveis/1.331 nós com curvas Bézier amostradas; 13 fechadas, 4 abertas, 3 grupos/6 objetos agrupados; 7 Path compactados em LineClosed, 4 Path referenciam PrimID compartilhado sem PrimList local (IDs 1,6,8).
- Diagnóstico de causa: importador anterior não aceitava `PrimList=LineClosed` nem buscava referências `PrimID`/`VertID` compartilhadas. Além disso, teste sintético antigo exigia elipse 96, apesar do importador ter evoluído para 384 pontos.
- Branch `fix/lbrn2-real-shared-primitives-19-8` derivada de `fix/tesoura-fechos-lbrn2-selecao-19-8`; PR #9 draft. Importador em `src/lightburn_geometry_import.hpp` já implementa LineClosed/LineOpen e duas passagens de resolução, rejeitando IDs conflitantes/ausentes; preserva grupos em EditorState.
- Qt ganhou fixture sintetizada (não copia a arte do cliente), teste `--lbrn2-file-19-8-test` com hash conhecido e contagens esperadas, modo de resultado JSON. Script Windows aceita `-ArquivoLbrn2` junto a `-SomenteImportacao` ou `-Abrir`.
- **Status: Código publicado, scanner estrutural PASS, leitura Python do arquivo real PASS; SEM compilação/teste Qt Windows nesta branch.** Não homologar, não fazer merge, não instalar antes de PASS.
- Referência: `DESENVOLVIMENTO/ESPECIFICACOES/IMPORTACAO_REAL_SUPORTE_ALEXA_LBRN2_19_8.md`.
- Comandos: conferir `git status --short`, `git fetch origin`, `git switch --track origin/fix/lbrn2-real-shared-primitives-19-8` (se não houver mudanças locais a preservar), depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteImportacao -ArquivoLbrn2 "CAMINHO_REAL_DO_ARQUIVO"`; se PASS, `-Abrir` com o mesmo parâmetro. Manter a máquina desconectada.


## 28/10/2026 — Falha Windows de regressão LBRN2 por elipse 96 versus 384

- Captura Windows mostrou build Qt e regressões de núcleo passando até `Importacao geometrica LBRN2 (codigo 1)`, JSON: `{"error":"elipse nao foi convertida para pontos","status":"FAIL"}`.
- Branch executada no Windows na imagem: `fix/tesoura-fechos-lbrn2-selecao-19-8` (PR #8), **não** a branch mais nova de importação LBRN2 real.
- Causa confirmada no GitHub: `src/lightburn_geometry_import.hpp` constrói elipses com **384** pontos, enquanto o teste em `qt/qt6_test6.cpp` ainda exigia **96**. NÃO era comprovação de erro geométrico do importador.
- Hotfix aplicado na branch antiga: commit `4c1ef6011850d7a6aa7dd3c24ed05d11918ae407`, alinhando teste a 384. A branch atual `fix/lbrn2-real-shared-primitives-19-8` já possuía a asserção de 384 e suporte adicional real a PrimID/LineClosed.
- Recomendação: com mudanças locais preservadas, `git fetch origin`; migrar para `fix/lbrn2-real-shared-primitives-19-8`, executar `-SomenteImportacao -ArquivoLbrn2 "<arquivo .lbrn2 do cliente>"`; só depois de PASS executar a suíte `-Abrir`.
- **Correção no GitHub sem execução Windows nesta resposta**; não marcar homologada. Arquivo original e versão homologada preservados.


## 28/10/2026 — refino visual e rastreio de bitmap (branch experimental; PR em rascunho)

- Vídeo real `20261028_135547.mp4` (~50 s) confirma facetas acentuadas em curvas da letra R após zoom, melhorias de Tesoura, movimentação e uso da área de trabalho. O usuário pede uso da documentação LightBurn e das experiências Web v0.10.58.6/.7.
- Nova branch isolada: `feat/refino-curvas-rastreio-diagnosticos-19-8`, base `fix/lbrn2-real-shared-primitives-19-8`. Web e `main`/candidata aprovada sem modificações.
- Texto: vetorização da fonte com leitura dos elementos QPainterPath e subdivisão de Bézier cúbica em ~0,016 mm, substituindo `toSubpathPolygons`. Mantém subcaminhos/vazados; ainda é polilinha ao salvar, não texto semântico.
- Bézier: novo botão `Nova curva` cria curva cúbica real com 2 âncoras e 2 alças, selecionada e editável na sessão nativa. Suporte a inserção de nós existente. Não converte polilinhas automaticamente.
- Rastreio: motor `src/raster_trace_19_8.hpp`; importação de PNG/JPEG/BMP/WebP por barra/menu/arraste; bitmap temporário sobre a mesa; clique direito > Traçar imagem ou Alt+T abre prévia P&B com vetores roxos e ajustes de limiar Otsu, ilhas, otimização, suavização opcional e largura física. Salva vetores só após confirmar, agrupados em um único undo. Limites 960 px raster de trabalho, 500 mil arestas, **50 mil nós/1000 contornos**. Bitmap original não muda; overlay não persiste no .alaser.
- Coleta da Tesoura: causa comprovada de eventos ausentes era a whitelist `DiagnosticsCollector` desatualizada. Agora aceita os eventos, anota até 100 tentativas com resultados categorizados, ID da forma, segmento, zoom e tempo, somente com coleta opt-in; não inclui coordenadas, arquivo, conteúdo.
- Clique na paleta com seleção transferia camada já na branch anterior; agora exibe confirmação no status. Faixa da mesa mais estreita (margem, título e botões curtos).
- Teste `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteRefino`, também no gate integral `-Abrir`, valida texto R/O, P&B e furo, ruído, Bézier nativa com undo/redo, eventos da Tesoura whitelist e clique de GUI sob coleta ativa.
- **NÃO COMPILADO no Windows nesta branch; sem homologação.** Segurança: nenhuma emissão, movimento, G-code ou controle físico habilitado; manter máquina desconectada. Testes visual/Windows e desempenho em imagens reais necessários.
- Documento: `DESENVOLVIMENTO/ESPECIFICACOES/REFINO_RASTREIO_BEZIER_DIAGNOSTICOS_19_8.md`.
- Trabalhos ainda pendentes: curvas/arcos matematicamente nativos para TEXTO e LBRN2, Trace Image mais sofisticado (sketch/alpha/corner detection e texto pequeno), importação/exportação real de imagens persistentes, conversão de polilinha para Bézier com undo integrado e prefill de parâmetros fabris (ainda bloqueado).


## 28/10/2026 — Hotfix de compilação Qt TESTE_19.8 (PR #10)

- Evidência da captura VS Code de 28/10: compilador reportou `QtEditorLab::rasterBounds() const` e `QtEditorLab::setRasterWidthMm(double)` **privados** quando usados por `LabWindow::traceImageDialog()`. O erro `ninja: build stopped: subcommand failed` e o PowerShell código 1 foram consequências desse erro C++, não problemas de LBRN2.
- Correção aplicada diretamente na mesma branch experimental `feat/refino-curvas-rastreio-diagnosticos-19-8`, commit `3676dbe0b4c242e6e183200c939dc331742bef9d`: `public:` apenas em frente à API raster (`setRasterWidthMm, setRasterImage, rasterImage, rasterBounds, hasRasterImage, clearRasterImage`); `private:` restaurado antes de `clearNativeItems`. Os membros `rasterSource_, rasterRect_, rasterOverlay_` permanecem privados.
- Foi conferido estaticamente no repositório que todas as seis APIs raster estão sob `public`, e os membros internos continuam privados.
- **A recompilação Windows ainda não foi executada após o commit.** Não afirmar PASS/homologação.
- No terminal Windows: `git status --short` e `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` (apenas se a árvore de trabalho estiver segura); depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteRefino`.
- Não sobrescrever alterações locais; em caso de novo erro de compilação, usar relatório `RELATORIOS/COMPILACAO/BUILD_GITHUB_19_8.txt` e a primeira linha `error:` para corrigir a causa, sem trocar para `main`.

## 28/10/2026 — Moldura de selecao nao seguia objeto durante arraste (correcao experimental)

- Evidencia: video real TESTE_19.8 de 28/10 e relato do usuario. A forma acompanha o mouse, mas o retangulo de selecao e as alcas ficam na posicao de origem ate mouseRelease.
- Diagnostico confirmado no Qt: `mouseMoveEvent` usa `QGraphicsPathItem::setPos(delta)` apenas na geometria exibida; `paintSelectionHandles` usava `selectedBounds()` calculado dos nos canonicos, que so mudam ao finalizar o gesto.
- Correcao em `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp` na branch `feat/refino-curvas-rastreio-diagnosticos-19-8` (commits `1d4fb799fa92b4db381e4aa04bf57191fd78e75b` e `9c13d4b54d8ac6475decaea1a79c646a2dca23e2`): `shapePreviewDelta_` e cache `shapePreviewBounds_` acompanham a forma; `selectionDisplayBounds()` alimenta desenho da moldura e 13 alcas. `viewport()->update()` invalida foreground durante o arraste. Transformacoes por alca tambem usam a matriz temporaria; nenhum no canonico e alterado antes de soltar.
- Limpeza do cache/transformacao no cancelamento; transacao de undo mantida ao soltar. Teste Qt automatico anexado ao existente `--inline-transforms-19-8-test`: movimento por contorno, alca central, invariancia do documento antes de soltar, persistencia da caixa no release e dois passos de undo. Captura de regressao `SELECAO_ARRASTE_DURANTE_MOVIMENTO.png` gerada pelo teste.
- Diagnostico de sessao `Alencar_Laser_Diagnostico_TESTE_19_7(4).json`: 5 eventos `shapes_moved` registrados; isso confirma operacoes de movimento, mas nao demonstra sincronizacao visual da moldura.
- **Estado: CORRECAO ESTATICA PUBLICADA, NAO TESTADA NO WINDOWS NESTA REVISAO.** Nao homologar nem mesclar. No Windows: `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` somente se seguro; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Depois de PASS, testar mouse fisico em retangulo, grupo e arquivo real e so entao suite `-Abrir`. Main e APLICATIVO preservados; hardware desconectado.

## 28/10/2026 — Regressao da barra lateral sem icones (captura Windows)

- Print de 28/10: topo com logo oficial e faixa de camadas visiveis, mesa central aberta e painel direito presente; faixa estreita esquerda escura, porem sem nenhum icone das ferramentas.
- Analise do codigo da branch: `leftPanel` limitado a 52–64 px, QScrollArea sem rolagem horizontal e `fileGrid` contendo botao `Imagem / Traçar…` com texto longo e largura minima maior que a coluna. Como todos os grids eram centralizados dentro do mesmo `leftContent`, esse minimo podia deslocar os botoes icon-first para alem da viewport. Esta e uma causa estrutural plausivel; precisa de confirmacao visual no Windows apos recompilar.
- Correcoes na branch experimental: botao Imagem/Traçar virou 38×36 px, icone + tooltip + nome acessivel; margem e rolagem vertical ajustadas, largura minima do painel 68 px (max. 76), para resistir inclusive a estado antigo do QSplitter. Nenhuma funcionalidade real da importacao foi removida.
- `tool_glyphs.hpp`: desenhados glifos antes vazios de `new_bezier` e novo `image`. Imagem da marca do cabecalho nao foi alterada.
- Teste adicionado a `--inline-transforms-19-8-test` (script `-SomenteInteracao`): valida existencia de 12 botoes, icones nao nulos e nao transparentes, largura do conteudo compativel com a viewport e posicao horizontal visivel de cada botao; captura `BARRA_LATERAL_19_8.png`. Isso NAO substitui teste com mouse e captura Windows.
- Commits `60dc9dc049059993593c15bc307248be34abe7b2`, `0e6dba5b332566f131c17a3bd40af6bff116fb28`, `7c8928acc9a7b895a0f1174004ded7a59d4f92af`.
- **Status: modificacoes publicadas, SEM teste Windows nesta rodada; main/APLICATIVO preservados, laser desconectado.** Atualizar branch com `git status --short` e `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` somente sem conflitos. Rodar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`; se PASS, `-Abrir` para suite completa, e conferir visualmente icones e rolagem da barra.

## 28/10/2026 — Prévia virtual, camadas S/V/A, T1/T2 e arrastar arquivos (etapa experimental)

- Pesquisa na documentação oficial LightBurn: painel Cuts/Layers usa Output (envio), Show (exibição) e Air (assistência de ar); T1/T2 são Tool Layers que nunca são cortadas/gravadas, embora a opção Frame possa afetar enquadramento; Preview mostra percursos Line e viagens sem laser e estima duração com parâmetros de velocidade/aceleração; importação de bitmap não é o mesmo que Trace Image; imagem original deve permanecer preservada.
- Código existente antes desta etapa: GRBL USB/TCP com leitura/consultas diagnósticas aprovadas no projeto; escrita de G-code, movimento e disparo fisico bloqueados. Editor já tinha LBRN2, .alaser e importação PNG/JPEG/BMP/WebP, mas drop no viewport central podia não chegar ao QWidget pai; rastreio P&B exigia Previsualizar manualmente.
- Motor local novo `src/preview_sim_19_8.hpp`: constrói percursos de contornos Line, distingue movimento com e sem laser, filtra Output false, ignora obrigatoriamente T1/T2 e exclui Fill. Estima tempo NOMINAL por distâncias / velocidades virtuais configuradas, SEM aceleração, overscan, imagem, hatch, G-code, sequenciamento otimizado ou transmissão. Limite de 300000 segmentos.
- Interface nova `qt/preview_dialog_19_8.hpp`: botão "Prévia virtual" na barra superior; mostra contornos claros, viagens vermelhas, controle de progresso, velocidade hipotética e estatísticas. Não representa simulação de laser homologada.
- `src/editor_tools.hpp`: metadados de camada incorporam `output` (padrão true) e `air` (padrão false), com defaults retrocompatíveis em documentos antigos. IDs reservados 1001=T1 e 1002=T2 nascem com `output=false`; parser também força false. Não altera lógica de CAM, que permanece bloqueada.
- `qt/qt6_test6.cpp`: lista vertical de camadas ocupadas com toggles S=Saída da prévia, V=Visibilidade no editor e A=anotação editorial de ar; criação explícita T1/T2. Evento toggle aplicado de forma enfileirada para não destruir widgets enquanto entregam sinal Qt.
- Arraste e solte na viewport com filtro de eventos, aceitando um arquivo local .alaser/.lbrn2/PNG/JPEG/BMP/WebP; para imagem, abre automaticamente diálogo de traçado com prévia P&B calculada via Otsu. Nenhuma conversão definitiva nem vetorização sem confirmação. Não anunciar SVG/DXF como suportados.
- Testes agregados à suíte existente: `-SomenteInteracao` valida simulação de retângulo, Output off, exclusão de T1 e roundtrip metadata Air/Output; `-SomenteImportacao` inclui eventos DragEnter na viewport para LBRN2 e PNG. Validações ainda NÃO executadas em Windows nesta etapa.
- Próxima etapa: calibrar tempo com logs reais LightBurn (aceleração X/Y, deslocamentos, passagens, preenchimento e raster); conferir compatibilidade de parâmetros, origens, modos e unidades; só depois criar gerador/validador de trabalho e homologação de máquina em ambientes controlados.
- **STATUS: arquivos publicados somente na branch `feat/refino-curvas-rastreio-diagnosticos-19-8`; SEM COMPILACAO/TESTE WINDOWS DESTA ETAPA; main e APLICATIVO intactos.** Executar `git status --short`, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se árvore segura e `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao` seguido de `-SomenteImportacao`; se ambos PASS, executar suite integral `-Abrir` com laser desconectado. Não afirmar sucesso antes dos resultados.

## 28/10/2026 — Continuidade sem Windows: prévia cinemática experimental com testes portáteis

- **Escopo aprovado:** trabalhar remotamente via GitHub na branch experimental `feat/refino-curvas-rastreio-diagnosticos-19-8`, sem mesclar `main`, sem alterar `APLICATIVO` e sem liberar emissão, movimento, ar ou G-code.
- Referência documental oficial LightBurn: Preview mostra caminhos, viagens e tempo estimado; Additional Settings usa velocidades máximas e acelerações X/Y. Não alegar equivalência física com LightBurn nem precisão do tempo até confronto com trabalho real.
- **Novo núcleo `src/preview_motion_19_8.hpp` (C++20 puro):** velocidades solicitadas e limites X/Y projetados na direção, aceleração X/Y por linha e viagem, agrupamento de segmentos contíguos com ângulo <=12° como simplificação, perfil triangular/trapezoidal repouso–repouso, duração e cumulativa temporal. Este é um modelo aproximado, não planner GRBL.
- **Múltiplas passagens:** `sim198::repeated` duplica plano Line completo (1–20) e insere viagem virtual entre passagens; máximo 300.000 movimentos; **não** reproduz ordem por camada/passagens do LightBurn. Output desligado, T1/T2 e Fill continuam excluídos.
- **Janela de prévia Qt:** controles de feed linha e viagem, limites máximos X/Y, aceleração X/Y por tipo, passes; dois tempos (nominal e aceleração), segmentação temporal, slider por tempo, Reproduzir/Pausar até 40x. Relatório JSON exportado localmente após escolha do usuário, contendo apenas métricas e hipóteses, sem geometria.
- **Testes adicionados:** C++ portátil `tests/preview_motion_19_8_standalone.cpp`, 1.000 trajetórias pseudoaleatórias + perfis, e modo Qt `--simulation-motion-19-8-test` gerando `RESULTADO_PREVIA_MOVIMENTO_19_8.json`. Script agora aceita `-SomenteSimulacao`, incluindo compilação portátil e Qt; suíte completa também roda regressão.
- **Evidência executada neste ambiente:** C++20 compilou com `g++ 14.2` (`-Wall -Wextra -Werror -pedantic -O2`), testes de perfis, aceleração, tetos X/Y, time slider, validação e 1.000 trajetórias passaram. Header verificado idêntico ao GitHub por SHA blob `7e4e8ab10daa4bd2627d0987c9359afd2afc034c`. Os testes Qt/Windows **não foram executados** aqui.
- **Spec detalhada:** `DESENVOLVIMENTO/ESPECIFICACOES/SIMULACAO_MOVIMENTO_E_TEMPO_19_8.md`.
- **Próxima ação local:** `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` somente se sem conflitos; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Se PASS, testar `-SomenteInteracao` e `-Abrir`, com laser desconectado. Jamais confundir PASS portátil com homologação Qt.
- **Pendências:** benchmark com LightBurn por trabalho idêntico e tempo real; raster Fill/Image, cantos/overscan, performance em projetos densos, dados físicos somente leitura, política de segurança para eventual controle.

## 28/10/2026 — Novo avanço experimental: Fill vetorial e varredura de imagens

- Usuário autorizou sucessivos avanços no GitHub enquanto não utiliza o computador, com análise de documentação LightBurn, testes disponíveis e registro de mudanças. A branch experimental permanece `feat/refino-curvas-rastreio-diagnosticos-19-8`.
- Referência oficial: LightBurn 2.1 Fill Mode preenche formas fechadas por linhas paralelas e usa regra paritária/vazados para contornos na mesma camada; Overscanning acrescenta trechos sem emissão; Image Mode oferece Threshold e modos de dithering. Esta implementação **não é cópia do LightBurn nem controlador de laser**.
- Novo módulo `DESENVOLVIMENTO/FONTE/src/preview_raster_19_8.hpp`: simulador C++20 **somente de geometria**, sem Qt/hardware, que usa interseções horizontais half-open e pareamento even-odd de contornos fechados por camada Fill, com reconhecimento de fechamento repetido de LightBurn. Exclui Output=false e T1/T2; emite segmentos de varredura virtual com overscan laser-off e possibilidade de bidirecional.
- Máscaras de imagem binárias sem alterações no bitmap original; novo diálogo Qt mostra limiar P&B ou dithering Bayer 4×4, em amostra limitada a 320×320, e respeita coordenadas e dimensões da imagem importada. Não implementa grayscale com potência variável nem exporta G-code.
- `preview_dialog_19_8.hpp`: novas opções Fill/Imagem, intervalo 0,05–5,00 mm, overscan 0–20 mm e varredura bidirecional; recalcula a estimativa nominal/cinemática já existente em cima do plano combinado Line+Fill+Imagem; relatório JSON inclui opções de prévia. A visualização gráfica é amostrada em planos muito densos para proteger fluidez da janela.
- `qt6_test6.cpp`: abertura da prévia recebe QImage e limites atuais do editor em modo somente-leitura; `--simulation-raster-19-8-test` adiciona regressões de quadrado 10×10, furo 4×4, overscan sem laser, camadas Output off, guias T1, máscara 4×2, repetição de passes, integridade da geometria e limites.
- `tests/preview_raster_19_8_standalone.cpp`: novo teste C++20 portátil sem Qt, acrescentado a `-SomenteSimulacao` (já tinha o movimento portátil + Qt). `-Abrir` também exige PASS da regressão Qt de raster.
- Fonte da documentação detalhada: `DESENVOLVIMENTO/ESPECIFICACOES/SIMULACAO_FILL_RASTER_IMAGEM_19_8.md`. Referências oficiais: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/FillMode/ , https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/ImageMode/ , https://docs.lightburnsoftware.com/2.0/Explainers/Overscanning/ .
- **STATUS: Código e testes PUBLICADOS; ainda SEM COMPILAÇÃO Qt6/Windows nem PASS do novo raster.** O PASS Windows da rodada anterior diz respeito apenas à simulação cinemática sem raster. Não confundir as duas etapas, não marcar homologada e não autorizar máquina.
- Próxima validação quando houver PC: `git status --short` (a pasta local não rastreada `.vscode/` deve ser preservada), `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se árvore segura, `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Depois de PASS, `-Abrir` e avaliação visual sem conexão física. `main`, `APLICATIVO`, dados e versões homologadas intocados.
- Futuro: refinamento real por imagens do cliente, otimização de raster, varreduras verticais/cross-hatch, análise de limites físicos antes de qualquer movimento, perfis de material, calibração de estimativas com logs LightBurn e GRBL observacionais; separar simulação e controle físico em revisão e aprovação independentes.

## 28/10/2026 — Camadas compactas, ajustes de potência/velocidade e M3/M4 + rota virtual

- Evidência do usuário: lista de camadas ocupadas funciona, mas "quadrados" largos quase escondem o conteúdo; deseja controles compactos S/V/A, duplo clique no item para potência/velocidade e otimização de rota; pesquisar LightBurn e nome correto de M3/M4.
- LightBurn oficial: a lista Cuts/Layers mostra uma linha por camada e Output/Show/Air, duplo clique abre Cut Settings Editor. M3 = **potência constante**; M4 = **potência dinâmica (ajusta potência à velocidade)**; M3 não significa automaticamente "perfuração". Otimização avançada LightBurn pode ordenar por camada/grupo, priorizar interiores e reduzir deslocamentos. Fontes: https://docs.lightburnsoftware.com/latest/Reference/CutsLayersWindow/ , https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/SharedSettings/ , https://docs.lightburnsoftware.com/2.1/Reference/OptimizationSettings/ .
- `qt/qt6_test6.cpp`: lista compacta 26px por linha com três checkboxes 20px sem texto, cabeçalho S/V/A alinhado e nome truncado somente visualmente. Duplo clique abre `qt/layer_settings_19_8.hpp`; clique simples apenas seleciona. T1/T2 não editam gravação.
- `src/editor_tools.hpp`: Layer guarda velocidade virtual (mm/min), potência nominal (%), passes e powerMode M4/M3, com validação e defaults compatíveis para projetos antigos. `configureLayer()` registra mudança de todos os campos em UMA transação undo/redo; `metadata()/fromMetadata()` persistem os quatro atributos. Não alterou canais de controladora.
- `src/preview_route_19_8.hpp`: vizinho-mais-próximo com ordem por camada, reversão de linhas abertas e escolha de vértice inicial em contornos fechados (máx. 1500 contornos). Heurística, não ótimo global nem "Cut Inner Shapes First". Não altera desenho.
- `src/preview_motion_19_8.hpp`: `estimateWithFeeds` usa feed da camada para estimar linhas virtuais. `src/preview_sim_19_8.hpp`: `repeatedByLayer` implementa contagem virtual de passes por camada e também global, respeitando limite total. Potência/M3/M4 **somente metadados**, sem modelagem real PWM/energia e sem transmissão física.
- `qt/preview_dialog_19_8.hpp`: checkbox para reduzir deslocamentos, checkbox para usar velocidades de camada, comparação de viagem Line original vs heurística, cálculo nominal/cinemático por camada, JSON de métricas. A configuração de potência não aplica potência real.
- Testes: `--layer-route-19-8-test` no Qt valida roteiro, imutabilidade, velocidade, passes, potência, metadata legado, M3/M4, undo e exclusão T1; `--inline-transforms-19-8-test` valida dimensões da lista e checkboxes. Ambos incluídos em `-SomenteSimulacao` e/ou `-Abrir`; `RESULTADO_CAMADA_ROTA_19_8.json`.
- Documento: `DESENVOLVIMENTO/ESPECIFICACOES/CAMADAS_COMPACTAS_M3_M4_ROTA_19_8.md`.
- **ATENÇÃO: APENAS Código publicado na branch experimental e inspecionado estaticamente neste turno; compilação Qt6 Windows não executada após estas mudanças.** A aprovação do usuário para simulação cinemática anterior não cobre camadas/M3/M4/otimizador. A main, base e APLICATIVO continuam preservados.
- Próxima validação: `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` somente se seguro; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`; depois `-SomenteInteracao` e `-Abrir` sem hardware. Não realizar G-code, ligação física, movimento/disparo nem alteração firmware.

### Complemento — opção de rota no próprio editor da camada

- Atendendo à solicitação de ter otimização também dentro da janela de duplo clique, `Layer.optimizeTravel` foi persistido como preferência booleana (padrao true em projeto legado).
- `qt/layer_settings_19_8.hpp` agora tem checkbox de autorização da heurística por camada. A opção global da prévia é o interruptor principal; se a camada optar por não otimizar, `preview_route_19_8.hpp` preserva ordem e orientação originais dos seus contornos.
- `--layer-route-19-8-test` ganhou regressão de OFF sem reordenação, ON com redução no caso preparado e metadata legado com default ON.
- Foram realizadas **41 de 41 verificações estáticas de integração no GitHub** (presença de símbolos e restrições de envio físico), mas **não foi executado o teste Windows de compilação/GUI nesta rodada**. Estas verificações não equivalem a PASS da nova candidata.

## 28/10/2026 — Otimizacao virtual: contornos internos primeiro (sem Windows)

- O usuario disse "Avance" para continuar a próxima prioridade da 19.8, previamente definida como ordenação de contornos internos antes dos externos e comparação de percursos.
- Documentação LightBurn 2.1 verificada: `Optimization Settings > Cut inner shapes first` exige contorno externo fechado; `Reduce travel moves` é outra otimização; ambas só atuam com otimização habilitada. Fonte: https://docs.lightburnsoftware.com/2.1/Reference/OptimizationSettings/
- Novo `src/preview_nesting_19_8.hpp`: relação de contenção **estrita** por bbox, ray parity, interseção de arestas; contorno externo deve ser fechado e não autointersectante, imagens/Fill excluídos; quantidade de testes geométricos limitada a 4 milhões. Limites/tangências tratados sem impor precedência.
- `src/preview_route_19_8.hpp::nearestNeighborLines` aceita `innerFirst` (padrão true) e contador opcional de relações. Precedência de formas internas antes da borda **somente na mesma camada Line**, e só se a camada permitir otimização. Entre formas elegíveis continua a heurística vizinho mais próximo. Não promete menor distância final; pode aumentar viagens. `Layer.optimizeTravel=false` continua preservando a ordem do arquivo e não é sobreposto.
- `qt/preview_dialog_19_8.hpp`: checkbox "Contornos internos primeiro (corte virtual)", subordinado ao botão geral de otimização; exibe número de dependências detectadas e compara viagens originais vs rota escolhida; exporta métricas em JSON sem geometria nem dados de hardware.
- `qt6_test6.cpp`: novo modo `--route-nesting-19-8-test` com três níveis aninhados, linha aberta interna, precedência sem cruzar camadas, opt-out da camada, opção innerFirst=false, tangência e exclusão T1; relatório `RESULTADO_CONTORNOS_INTERNOS_19_8.json`.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`: `-SomenteSimulacao` e `-Abrir` exigem novo teste de precedência após os gates anteriores.
- Documento técnico completo em `DESENVOLVIMENTO/ESPECIFICACOES/ROTA_CONTORNOS_INTERNOS_19_8.md`.
- **ESTADO:** alterações no GitHub na branch experimental, **sem compilação Qt6 Windows nem execução dos novos testes nesta rodada**. O último PASS Windows do usuário refere-se somente à simulação cinemática anterior. `main`/base homologada/`APLICATIVO` intocados; nenhum novo comando GRBL, físico, M3/M4, ar ou G-code. Não marcar como final.
- Quando estiver com Windows: `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se seguro (preservar `.vscode/`); `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Se PASS, `-Abrir` e conferir ordem com mouse real e Laser desconectado.
- Próximos trabalhos: consolidação dos testes de otimização para arquivos LightBurn reais e auditoria integrada de arquivos, logs e vídeos. Usar Work quando houver conjunto de evidências suficiente para auditoria multimodal.

## 28/10/2026 — Consolidação da 19.8 na branch experimental (sem PC)

- Usuário voltou o foco integralmente para **Alencar Laser Windows**, pediu "Avance" após sequência de implementações. Não redirecionar para publicação Web ou começar nova ferramenta sem estabilizar a candidata.
- Auditoria de módulos `preview_sim`, `preview_motion`, `preview_raster`, `preview_route`, `preview_nesting`, `editor_tools` e interface Qt. Prioridade: **corrigir e testar integração** em vez de anunciar novas funcionalidades como aprovadas.
- `src/preview_nesting_19_8.hpp`: cache triestado de teste `simpleClosed` reduz duplicação de verificações em polígonos aninhados; contorno interno fechado autointersectante não força precedência. Continua limitado por orçamento de verificações geométricas.
- `tests/preview_route_19_8_standalone.cpp`: novo teste C++20 independente do Qt, cobre três níveis, fronteiras tangentes, linha interna aberta, cruzamento entre camadas, T1/T2, opt-out da camada, caso autointersectante, passes globais e por camada e continuidade de todos os movimentos sem teleporte ativo.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteSimulacao`: agora compila/executa esse terceiro teste C++ portátil antes dos gates Qt. Não alterar scripts da base homologada.
- `.github/workflows/preview-19-8-offline.yml`: CI limitado à branch 19.8 e `contents: read`, com GCC C++20 e testes portáteis motion/raster/route; não usa Qt, G-code, laser, credenciais ou comandos físicos. O conector ainda não retornou resultado de execução, portanto **CI não homologado/não declarar PASS**.
- Relatório completo: `DESENVOLVIMENTO/ESPECIFICACOES/CONSOLIDACAO_PREVIA_CAMADAS_19_8_20261028.md`. Inclui evidência do PASS anterior do usuário **somente na simulação cinemática**, matriz de riscos, passos de Windows e limite de interpretação.
- **Status:** GitHub atualizado apenas na branch `feat/refino-curvas-rastreio-diagnosticos-19-8`. Correção e testes foram publicados; execução dos novos testes Qt e Windows **pendente**. `main`, `APLICATIVO` e controladora **não tocados**.
- Próxima operação quando houver Windows: `git status --short` preservar pasta local `.vscode/`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8`; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Se passar `-SomenteInteracao` e `-Abrir` com máquina desconectada.

## 28/10/2026 — Consolidação da consistência de simulação (próxima rodada)

- Usuário confirmou que a publicação do Alencar Laser Web fica para o futuro; foco **exclusivo no Alencar Laser Windows 19.8**. Após pergunta sobre Copilot Cloud Agent, autorizou "Avancemos": continuar auditoria e desenvolvimento na branch experimental, sem alterar `main` nem a base aprovada.
- Auditoria de integridade encontrou três riscos verificáveis na fonte:
  1. `qt/preview_dialog_19_8.hpp` mostrava tempo nominal por feed de camada, mas exportava `nominal_seconds` por feed global. Agora usa `motion::nominalSecondsWithFeeds` compartilhado e guarda `nominalSeconds` para **tela + JSON**; mantém cenário cinemático separado.
  2. `src/preview_raster_19_8.hpp::appendMask` redistribuía `ceil(altura/intervalo)` linhas ao longo da imagem, alterando o pitch solicitado. Agora posiciona linhas por espaçamento físico e adapta a amostragem de pixel ao `y` geométrico; uma imagem menor que meio intervalo recebe linha central. Regressões testam 21 mm/10 mm (linhas y=5,15) e 4 mm/10 mm (linha y=2).
  3. `appendFill` podia executar custo alto `O(linhas × vértices)` na thread da GUI. Adicionado budget preventivo de **6 milhões de avaliações de interseção**, com erro explícito antes do laço pesado; teste portátil de dois contornos de 4096 nós e 1000 linhas.
- Testes alterados: `tests/preview_motion_19_8_standalone.cpp`, `tests/preview_raster_19_8_standalone.cpp`, e `--simulation-raster-19-8-test` no `qt6_test6.cpp`. Mantidos testes de camadas, rota interna e G-code físico bloqueado.
- Relatório: `DESENVOLVIMENTO/ESPECIFICACOES/AUDITORIA_CONSISTENCIA_PREVIA_19_8_20261028.md`.
- **Estado:** correções PUBLICADAS no GitHub experimental; **NÃO afirmar que compilaram ou passaram** a suíte Qt/Windows nesta rodada. GitHub Actions ainda sem PASS verificado. O último PASS do usuário comprovava apenas estágio anterior da simulação. `main`/`APLICATIVO`/laser não alterados.
- Próxima ação Windows: `git status --short` (preservar `.vscode/`), `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` somente se seguro; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Após PASS, `-SomenteInteracao`, `-Abrir` e comparação com LightBurn. Não mesclar branch nem iniciar máquina nesta etapa.

## 29/10/2026 — HOTFIX de teste C++ raster invalidado por intervalo 10 mm

- **Evidência real do usuário:** captura VS Code com `TESTAR_19_8_WINDOWS.ps1 -SomenteSimulacao`. Configuração CMake PASS e teste C++ cinemático PASS (1000 trajetórias). Em seguida, teste C++ de Fill/bitmap lança `std::runtime_error: Intervalo ou overscan virtual fora dos limites`; PowerShell interrompe o gate. Não chegou à compilação Qt6 nesse ciclo.
- **Causa raiz confirmada no GitHub:** `src/preview_raster_19_8.hpp::validate` limita `intervalMm` a **0,05–5,00 mm**, em acordo com UI; o teste portátil `tests/preview_raster_19_8_standalone.cpp` e o equivalente Qt `qt6_test6.cpp::previewRaster198Test()` estavam configurando **10 mm** e esperando sucesso. Erro dos **testes**, não evidência de máquina/hardware.
- **Correção publicada** na branch `feat/refino-curvas-rastreio-diagnosticos-19-8`:
  - Fixtures raster com intervalo **5 mm** e dimensões ajustadas: imagem 40×10 para máscara P&B 4×2; passo exato com altura **12,5 mm**, centros `y=2,5` e `7,5`; imagem de 2 mm usa uma linha em `y=1`.
  - Overscan inválido de 21 mm testado isoladamente com intervalo válido de 5 mm.
  - Regressão do motor confirma **aceita 5 mm, rejeita 10 mm**. Nenhuma alteração nos limites físicos, no Qt ou hardware.
- Documento: `DESENVOLVIMENTO/ESPECIFICACOES/HOTFIX_TESTE_RASTER_INTERVALO_19_8_20261029.md`.
- **Status:** hotfix enviado ao GitHub; **ainda sem execução nem PASS no Windows após esses commits**. O PASS cinemático anterior continua comprovado; raster/Qt novos devem ser testados. `main`, `APLICATIVO` e comandos físicos preservados.
- Comando quando usuário estiver no computador: `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se seguro; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Preservar `.vscode/` e código local. Se houver outra falha, investigar primeira mensagem de erro e `RELATORIOS/COMPILACAO/BUILD_GITHUB_19_8.txt`, sem tratar todos os futuros erros como raster.

### Complemento — gate rápido para a falha do raster observada no Windows

- Novo `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteRaster`: compila o teste C++ do raster com G++ diretamente, antes do CMake/Qt; permite verificar o hotfix sem repetir o build inteiro. Não envia comandos ao laser.
- Revisão estática posterior ao hotfix confirmou **5 mm** nas fixtures que devem passar, **10 mm apenas no teste explícito de rejeição**, e mesmo ajuste no teste Qt; `src/preview_raster_19_8.hpp` mantém intervalo admissível `0,05–5 mm`.
- **Ainda falta executar os testes no Windows após o hotfix.** Primeiro `-SomenteRaster`; se PASS, `-SomenteSimulacao`. Preservar a base local e a pasta `.vscode/`.

## 29/10/2026 — Hotfix da compilação Qt: redeclaração de `header`

- **Evidência fornecida pelo usuário:** screenshot VS Code com erro em `qt6_test6.cpp:2249`: `conflicting declaration 'auto* header'` e indicação de `QFrame* header` declarado previamente em `qt6_test6.cpp:2165`. Aviso separado `lightburn_reference_inspector.hpp:156: warning: this 'if' clause does not guard...`.
- **Causa raiz:** a implementação do cabeçalho compacto S/V/A adicionou `auto* header=new QWidget` dentro do mesmo construtor de `LabWindow` que já possuía `auto* header=new QFrame` para o cabeçalho superior. Código GitHub mais recente localizava a segunda declaração próximo à linha 2424; o número exibido na captura era da revisão local compilada.
- **Correção:** renomeado o segundo ponteiro exclusivamente para `layerListHeader`, passando-o ao `QHBoxLayout` e ao `inspector->addWidget`. O `topHeader` original permanece idêntico. Commit `47ec6515257482d426bff047fb99c0002da96b4a`.
- **Limpeza do aviso:** `lightburn_reference_inspector.hpp` separa `if(s.firstTime.isEmpty()) { s.firstTime=time; }` de `s.lastTime=time`, mantendo a semântica e removendo a ambiguidade; commit `78e21d809ee653914ce5dc3850bf32c19c0335b2`.
- **Status:** código conferido no GitHub e atualizado só na branch experimental, compilação Windows **ainda não repetida após o hotfix**. Não declarar PASS antes do log. `main`, `APLICATIVO`, controle físico, M3/M4 e G-code permanecem preservados/bloqueados.
- **Roteiro Windows:** `cd C:\ProjetoAlencarLaser`; `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` com árvore sem alterações conflitantes; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao`. Se erro, enviar **primeiro erro** de `RELATORIOS/COMPILACAO/BUILD_QT_19_8_STDOUT.txt`/`STDERR.txt`, não só a exceção final do PowerShell. Não apagar `.vscode/`.

## 29/10/2026 — PASS real Windows do gate `-SomenteSimulacao` após os hotfixes

- **Evidência:** captura do terminal VS Code enviada pelo usuário em 29/10/2026, com prompt final `PS C:\ProjetoAlencarLaser>`.
- Git fast-forward da branch experimental realizado. CMake configure/generate/build files concluídos no diretório isolado `DESENVOLVIMENTO/FONTE/build_WINDOWS_GIT_19_8`.
- **PASS:** `profiles, axis caps/accel, continuity, 1000 random trajectories` (C++20 cinemática).
- **PASS:** `even-odd Fill/hole, image mask, overscan, tool layers, flags, bounds, read-only` (C++20 raster).
- **PASS:** `nesting, open line, cross-layer order, opt-out, guide, passes (offline)` (C++20 rotas).
- **PASS:** `Compilacao Qt concluida. Iniciando testes de refino.`.
- **PASS:** `SIMULACAO (MOVIMENTO + RASTER + CAMADAS/ROTA) PASS sintetico; controle fisico bloqueado.` Isso indica conclusão do roteiro Qt sintético para movimento, raster, parâmetros de camadas/rota e contornos internos conforme o script `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`.
- **Interpretação:** hotfix do teste raster (intervalo válido), correção da variável `header` duplicada e demais componentes necessários para esse gate foram compilados/executados no Windows do usuário. **Não significa** teste visual interativo, suíte integral `-Abrir`, simulação fiel do LightBurn, CAM, G-code ou controle físico homologados.
- **Próximo gate seguro (sem máquina conectada):** `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Se PASS, executar `-Abrir` para suíte mais ampla e inspeção visual, preservando `.vscode/`, projeto e base homologada. Conferir listagem compacta S/V/A, abertura por duplo clique, potência/velocidade apenas como metadados, ferramentas e prévia offline.

## 29/10/2026 — Desenho LightBurn-like: retas por clique, Bézier por arraste, snap e Auto-Join

- Usuário enviou foto real do Qt 19.8 aberto com 575 contornos/13.415 nós e explicou que deseja o comportamento do **Draw Lines** do LightBurn: clique põe reta; clique/segure/arraste cria curva cúbica; pontas abertas devem “magnetizar” e unir em um único caminho (ex.: retângulo aberto faltando um lado), também com opção explícita de unir nós.
- Referência oficial consultada: https://docs.lightburnsoftware.com/latest/Reference/DrawLines/ e https://docs.lightburnsoftware.com/1.7/Reference/AutoJoinSelectedShapes/ . A união automática foi projetada apenas para caminhos abertos elegíveis; formas fechadas, camadas distintas, ocultas/bloqueadas, grupos ou ambiguidade de snap permanecem intocados.
- **`src/editor_tools.hpp`:** `nearestOpenEndpoint` realiza snap de extremidades com filtro de camada e grupo; `Editor::commitJoinedPolyline` prolonga, combina ou fecha um caminho existente com um único Undo; `Editor::joinTwoOpenContours` junta dois contornos abertos selecionados com até 0,5mm entre extremidades, atômico, e recusa incompatibilidades.
- **`qt/qt6_test6.cpp`:** ferramenta Linha converte 10px físicos de snap em mm conforme zoom e DPR, usa marcador visual, Ctrl desabilita encaixe e termina automaticamente uma conexão numa extremidade; `new_bezier` virou **Desenhar curva** com cliques para nós de canto/reta, clique-arraste para nós suaves e alças Bézier, direita termina, Esc cancela e clique na origem fecha. Curvas são geometria nativa v5 (não achatadas). A ferramenta anterior “Bézier” permanece para editar alças; mudar de Desenhar curva para editar nós conclui caminho antes.
- **`src/bezier_interaction.hpp`:** `CurveDragSession::addEditablePath` salva um caminho multissegmento de reta/cúbica, cria IDs únicos para âncoras e um único Undo.
- **Ação explícita:** `Editar > Unir pontas selecionadas (Alt+J)` atua em **exatamente dois contornos abertos** na mesma camada não agrupada/desbloqueada, limiar de 0,5mm, preservando um ID.
- **Testes novos:** `tests/preview_draw_join_19_8_standalone.cpp` cobre fechamento de retângulo original, junção de dois caminhos, continuidade, filtragem de camadas, Alt+J, Bézier reta+cúbica e Undo/Redo; `qt6_test6.cpp --draw-join-19-8-test` usa QTest para validar interface e gestos de mouse com relatório `RESULTADO_DESENHO_UNIAO_19_8.json`. Script `-SomenteDesenho` compila e testa C++ e Qt; suíte integral `-Abrir` também incorpora esse teste. Workflow experimental inclui o teste portátil.
- Documento detalhado: `DESENVOLVIMENTO/ESPECIFICACOES/DESENHO_BEZIER_SNAP_JOIN_19_8_20261029.md`.
- **LIMITAÇÃO IMPORTANTE:** ferramentas Bézier e polilinhas ainda são duas representações. O Auto-Join magnético funciona nas polilinhas do Editor; o novo modo Bézier **não une automaticamente** um caminho cúbico nativo a outro caminho cúbico ou uma polilinha. No LightBurn isso é mais integrado; unificação de tipos requer fase posterior e testes adicionais.
- **STATUS:** código e testes publicados exclusivamente na branch experimental; **ainda NÃO houve compilação Qt6 Windows nem PASS para este novo lote**. A tela enviada prova que a UI antiga abriu, não comprova os novos gestos. `main`, `APLICATIVO`, ferramenta de corte físico e G-code preservados/bloqueados.
- **Próximo teste seguro com computador e laser desconectado:** `git status --short`, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` sem conflitos, depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho`. Se PASS, `-Abrir` para teste manual em cópia do projeto (575 contornos). NÃO tocar na versão estável nem ligar máquina.

## 29/10/2026 — Requisitos para PRÓXIMA ATUALIZAÇÃO: Fill com vazados, janela de prévia e zoom

- Usuário enviou **duas fotografias reais** do Alencar Laser 19.8: mesa com forma sólida com retângulos/contornos internos; e janela "Simulação OFFLINE" em laptop com diversos controles cortados na parte inferior. Vai testar primeiro a atualização anterior de desenho por clique/arraste, snap e Alt+J. **Não alterar o código desses recursos até chegar resultado do usuário**.
- Pedidos da próxima rodada:
  1. **Vazados (negative space)**: contorno fechado externo em modo Fill, contorno fechado interno na **mesma camada Fill** deve retirar gravação interna; se houver contorno dentro do vazio, paridade alterna de novo (preenchido). Área dentro do vazio deve ser laser virtual OFF e a animação respeitar isso.
  2. **Prévia responsiva**: janela hoje abre ~880×800 com parâmetros, imagem fixa e controles de reprodução/tempo/exportação/fechamento abaixo da tela; redimensionar ao espaço disponível, parâmetros com rolagem/recolhimento e controles principais acessíveis permanentemente.
  3. **Scroll do mouse na imagem de prévia** para zoom sob o ponteiro, pan do meio e ajuste à vista; animação de linhas Fill como no LightBurn, mantendo coerência de tempo, percursos e viagens virtuais.
- **Fonte conferida:** `src/preview_raster_19_8.hpp` JÁ implementa regra even-odd **por camada**, com teste portátil outer 10×10 menos inner 4×4 com `cutMm=84` (pitch 1 mm). A deficiência principal é que `qt6_test6.cpp::QtEditorLab::rebuild` preenche **QGraphicsPathItems individualmente**, podendo esconder vazados na visualização da mesa. Outra diferença: `qt/preview_dialog_19_8.hpp` usa `QDialog::resize(880,800)`, `QLabel` com imagem fixa 740×440 e **não** implementa zoom por roda ou scroll de parâmetros. O fato de `enableFill` estar marcado não significa que a forma esteja no modo `Fill`.
- **LightBurn oficial**: https://docs.lightburnsoftware.com/2.2/Reference/CutSettingsEditor/FillMode/ — contornos na mesma camada Fill se subtraem por paridade; em camadas diferentes, as áreas são preenchidas separadamente. https://docs.lightburnsoftware.com/2.0/Reference/Preview/ — zoom wheel, pan e slider.
- Especificação completa com cenários de aceitação e matriz de testes: `DESENVOLVIMENTO/ESPECIFICACOES/PROXIMA_ETAPA_FILL_VAZADOS_PREVIA_RESPONSIVA_19_8_20261029.md`.
- **Nesta mensagem**: só registro dos requisitos no GitHub experimental; **nenhuma alteração de código, compilação nova ou teste concluído**. Não afirmar que correção visual ou zoom já estão prontos. `main`, versão homologada, `APLICATIVO` e controle físico intocados.
- Quando o usuário concluir o teste atual e solicitar avançar, implementar na ordem: compositor visual de preenchimento por camada, modal responsivo com controles fixos, viewport independente com zoom/pan, reprodução coerente de trajetórias raster e testes guiados Windows.

## 29/10/2026 — Correção experimental: curvas Bézier inacessíveis após concluir desenho

- **Relato real do usuário:** criação de curvas e magnetismo funcionam, mas ao concluir a Bézier não consegue selecionar, editar, recortar ou desfazer; objeto permanece visível.
- **Diagnóstico na fonte:** desenho Bézier v5 salvo no `CurveDragSession`, enquanto modo Seleção usa `Editor` polilinhas. Troca para Seleção desligava `nativeMode_`; Ctrl+Z e Delete dependiam só do estado da ferramenta `bezierToolActive_`. Undo nativo só atualizava paths existentes e não removia `QGraphicsPathItem` de objetos desfeitos, causando risco de curvas fantasmas.
- **Hotfix publicado apenas na branch experimental**:
  - Hit-test do modo **Seleção** reconhece Bézier nativa, destaca a curva e permite mover contorno completo arrastando; troca para edição de Bézier mostra âncoras e alças sem alterar a forma.
  - `QtEditorLab::nativeHistory` reconstrói todos os objetos QGraphicsPathItem após Undo/Redo; `CurveDragSession::undo/redo` remove seleção pendurada se o objeto não existir mais.
  - Ctrl+Z/Y/Shift+Z e botões de histórico consideram a última edição nativa independentemente da ferramenta, com comparação de geração do Editor para escolher a pilha correspondente. **Histórico global unificado ainda não implementado**.
  - Delete ou botão Excluir em Seleção removem curva; edição de nós preserva Delete nó e Ctrl+Delete contorno.
  - Tesoura sobre Bézier nativa agora subdivide curva por de Casteljau em um ponto interno: uma forma aberta vira duas; curva fechada abre no ponto. **Não substitui a tesoura de remoção de intervalo entre cruzamentos**.
- **Regressões adicionadas** ao `tests/preview_draw_join_19_8_standalone.cpp` e `qt6_test6.cpp --draw-join-19-8-test`: ciclo completo desenho → seleção → Undo sem fantasma → Redo → editar alças → dividir com tesoura → Undo, e validação de subdivisão exata. Comando existente `-SomenteDesenho`.
- Documento detalhado: `DESENVOLVIMENTO/ESPECIFICACOES/HOTFIX_CURVAS_BEZIER_SELECAO_HISTORICO_TESOURA_19_8_20261029.md`.
- **Status:** código e testes registrados no GitHub; **SEM compilação Qt6 Windows nem confirmação PASS deste hotfix**. Testes anteriores do usuário aprovavam os gestos, mas comprovavam a falha pós-criação. Preservados `main`, base homologada, `APLICATIVO`, projetos locais, conexão física e emissão.
- Próximo passo: `git status --short`, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se árvore segura, `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho`; caso erro verificar primeira linha `error:` dos logs antes de novas features. Testar sem laser.

## 29/10/2026 — Painel Texto com fontes instaladas no Windows (lote pós-bug Bézier)

- Usuário informou que desenho Bézier e snap já funcionam visualmente, mas a curva concluída não podia ser selecionada, editada, recortada ou desfeita. Na mesma sequência, solicitou **formatação padrão para texto**, com caixa específica, fontes já instaladas no Windows, negrito, itálico e alinhamento, semelhante ao LightBurn.
- **Correção Bézier da rodada em andamento:** `src/bezier_interaction.hpp` reconcilia seleção após undo/redo e oferece corte nativo por subdivisão cúbica sem achatar; `qt6_test6.cpp` separa seleção de curva nativa da edição de alças, permite selecionar uma curva criada no modo Seleção, mover, excluir, desfazer/refazer com reconstrução de `QGraphicsPathItems` e tenta recortar pelo modo Tesoura. Regressões em `--draw-join-19-8-test` cobrem criação, seleção após troca de ferramenta, Undo/Redo sem curvas fantasmas, edição de alças e Tesoura.
- **Nova aba Texto no inspetor direito:** `QFontComboBox` usa as fontes **instaladas no Windows**; altura 1–150 mm; negrito, itálico, espaçamento relativo 60–200% e alinhamento esquerda/centro/direita; pequena prévia. Ferramenta `T Texto` abre a aba automaticamente; preferências de fonte salvas em `QSettings` local. Editor direto no desenho continua `QLineEdit`; selecionar formatação enquanto digita não deve converter texto prematuramente.
- `vectorTextOutlines` agora aceita alinhamento opcional e usa `QFont` com estilo/espaçamento para criar contornos vetoriais. **Limite honesto:** após Enter, o texto ainda é geometria vetorial; não é um objeto de texto reeditável. Não há multiline, SHX, line spacing, welded, Variable Text ou fonte incorporada. A conversão preserva o vetor no `.alaser`, não o conteúdo textual original.
- **Teste novo Qt** `--text-format-19-8-test`, relatório `RESULTADO_FORMATACAO_TEXTO_19_8.json` verifica controles, fontes do sistema, estilos, altura, espaçamento, alinhamento geométrico e persistência. A fixture restaura as preferências do operador após o teste, inclusive se ocorrer falha. Script Windows `-SomenteDesenho`, `-SomenteInteracao` e `-Abrir` agora incluem o teste.
- Documento técnico: `DESENVOLVIMENTO/ESPECIFICACOES/PAINEL_TEXTO_FONTES_WINDOWS_19_8_20261029.md`. Referência: https://docs.lightburnsoftware.com/2.2/Reference/Text/
- **STATUS:** apenas branch experimental GitHub atualizada, **sem execução Qt6 Windows e sem PASS para o lote Bézier/Text após as últimas mudanças**. Último PASS do usuário comprovava `-SomenteSimulacao` antes desses commits. Não modificar `main`, `APLICATIVO`, base homologada, firmware ou hardware.
- **Próximo passo seguro:** no Windows, com máquina desconectada: `cd C:\ProjetoAlencarLaser`, `git status --short`, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se a árvore estiver segura; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho`. Após PASS, inspecionar seleção de Bézier, Ctrl+Z, Tesoura, ajuste de fonte, alinhamento e texto convertido em contorno. Se falhar, enviar primeira mensagem de erro e logs.

## 29/10/2026 — Evidência Windows PASS do gate `-SomenteDesenho` (branch 357474b)

- **Evidência fornecida pelo usuário:** saída integral do terminal PowerShell no VS Code, após atualizar por fast-forward a branch `feat/refino-curvas-rastreio-diagnosticos-19-8` de `25b270a` para `357474b`. `git status --short` mostrava somente `?? .vscode/` (não rastreada: preservar). A alteração baixada incluiu `HOTFIX_CURVAS_BEZIER_SELECAO_HISTORICO_TESOURA_19_8_20261029.md` e `PAINEL_TEXTO_FONTES_WINDOWS_19_8_20261029.md`.
- **Resultado positivo:** `PASS: atomic join, closed rectangle, cross-contour merge, snap constraints, mixed native Bézier, undo/redo`; `Compilacao Qt concluida. Iniciando testes de refino.`; `DESENHO C++ + Qt PASS sintetico; teste manual do mouse ainda necessario.`
- **Confirmação pelo roteiro `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`:** em `-SomenteDesenho`, depois do teste C++ portátil, a janela Qt foi compilada, o teste `--draw-join-19-8-test` e também `--text-format-19-8-test` foram executados, cada um condicionado a `Checar-ResultadoQt` com JSON `status=PASS`. Assim, o PASS final atesta regressões sintéticas de magnetismo/Auto-Join, desenho Bézier, acesso à seleção/histórico/Tesoura nos cenários testados e painel de fontes/formatação (não uma verificação manual de todos os gestos e fontes).
- **Aviso inofensivo:** `preview_draw_join_19_8_standalone.cpp:64:6: warning: unused variable 'line'`. Não é erro e não impediu o PASS; pode ser limpo em próxima manutenção, mas não alterar código durante validação do usuário.
- **Limites de interpretação:** PASS de compilação e sintéticos **não** homologa seleção/movimentação/Tesoura em todos os tipos de curva nem edição persistente de texto, exportação/fabricação ou segurança de máquina. O usuário relatou anteriormente falha de selecionar/editar/desfazer curvas Bézier e solicitou fontes Windows, negrito, itálico, alinhamento; correções presentes nesse build devem ser inspecionadas com mouse real.
- **Próximo gate:** com máquina desconectada, executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir` quando apropriado para suíte completa, depois testar manualmente: criar curva Bézier, voltar à seleção, selecionar e mover, Ctrl+Z/Ctrl+Y, mudar alças, Tesoura; usar aba Texto com fontes Windows, negrito, itálico, alinhamentos. Salvar/abrir cópia do projeto. Enviar prints/vídeo de qualquer divergência, sem mexer na `main` nem na base homologada.
- **Status:** evidência operacional registrada; documentação atualizada na branch experimental apenas. A `main`, `APLICATIVO`, CAM, M3/M4 físicos, G-code e máquina permanecem preservados.

## 29/10/2026 — Lote experimental: reedicao de texto, Fill visual paridade, preview rolavel/zoom, alinhamentos

- Operador enviou **duas fotos reais** do editor 19.8 com painel de fontes/estilos e janela de simulacao cortada. Requisitos: texto que continua editavel, preenchimento vazado entre caminhos fechados na **mesma camada**, previa com controles acessiveis e zoom pelo scroll, ferramentas de alinhar bordas e centros ao estilo LightBurn.
- **LightBurn oficial pesquisado**:
  - https://docs.lightburnsoftware.com/2.1/Reference/Text/ (duplo clique para reeditar Text Objects);
  - https://docs.lightburnsoftware.com/2.1/Reference/Align/ (esquerda, direita, topo, base, centros usando normalmente ultimo objeto da selecao);
  - https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/FillMode/ (paridade: mesma camada cria vazado; camadas separadas preenchem independentemente);
  - https://docs.lightburnsoftware.com/latest/Reference/ZoomPan/ (zoom com roda e pan).
- **EditorState** em `src/editor_tools.hpp`: campo opcional `editableTexts` no metadata `editor-meta-1`, com texto/fonte/negrito/italico/altura/spacing/alinhamento/ancora/membros. O texto continua representado por contornos para a simulacao, mas agora guarda origem para reedicao. `addEditableText` e `replaceEditableText` sao atomicos com Undo e preservam o ID raiz. Formato antigo sem `editableTexts` continua abrindo. Texto antigo ja vetorizado **NAO** recupera frase de forma automatica.
- `qt6_test6.cpp`: duplo clique proximo a contorno do texto em Seleção/Texto, ou botão **Editar texto selecionado** no painel; carrega texto antigo e controles da fonte, refaz contornos, Undo restaura anterior. A integracao de persistencia `integrated_project_edit.hpp` foi conferida: `preserveEditorMetadata` copia chaves canonicas adicionais sem descartar campos extras.
- **Fill da mesa**: em `QtEditorLab::rebuild`, contornos fechados Fill sao compostos por camada com `QPainterPath::OddEvenFill`, em item de fundo que nao aceita mouse. Cada contorno continua editavel; isto corrige a incoerencia visual com o rastreador `src/preview_raster_19_8.hpp::appendFill` que ja tinha paridade.
- **Previa Qt** `qt/preview_dialog_19_8.hpp`: parametros e estatisticas numa `QScrollArea` limitada, linha do tempo e play/export/fechar sempre fora do scroll; tamanho adaptado a `availableGeometry`. `PreviewCanvas198` permite zoom wheel, pan com botao do meio e redefinicao "Ajustar imagem"; nenhuma alteracao de caminhos/tempos/comandos fisicos. Desempenho com planos muito densos ainda requer ajuste posterior.
- **Alinhamento**: `Editor::alignObjects` cobre esquerda/direita/topo/base/centros X/centros Y/ambos centros, com referencia no objeto ativo e grupo movido como unidade; painel `Objetos` + submenu `Organizar > Alinhar formas`. Aproxima o LightBurn; ordem exata do ultimo clique nem sempre se preserva em seleção desordenada.
- Testes novos:
  - `tests/text_alignment_19_8_standalone.cpp`: texto com origem, Undo/Redo, metadata legado, alinhamento e grupo;
  - `qt6_test6.cpp --text-fill-align-19-8-test`: fluxo real da UI de criar/editar texto, reabrir metadata, Ctrl+Z, circulos aninhados na mesa, alinhamento e zoom wheel;
  - `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao` compila C++ test e executa suite Qt adicional, que tambem foi incluida no `-Abrir`;
  - workflow offline GitHub atualizado para teste C++ portavel. Nao alegar PASS sem execucao observada.
- Relatorio detalhado e limites: `DESENVOLVIMENTO/ESPECIFICACOES/HOTFIX_TEXTO_FILL_PREVIA_ALINHAMENTO_19_8_20261029.md`.
- **STATUS HONESTO: CODIGO PUBLICADO NA BRANCH EXPERIMENTAL; COMPILACAO E TESTES DESSA RODADA NO WINDOWS AINDA NAO EXECUTADOS.** Os PASS anteriores de `-SomenteSimulacao` e `-SomenteDesenho` eram anteriores a este lote e não homologam estas correcoes. `main`, `APLICATIVO`, controle fisico, CAM e G-code permanecem preservados.
- Proximo passo no Windows, sem maquina conectada: `git status --short`; `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se seguro (preservar `.vscode/`); `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Se PASS, testar `-Abrir`, salvar/reabrir copia `.alaser`, reeditar texto e conferir janela da previa em tela menor.


## 29/10/2026 — Hotfix Qt: qualificar PreviewCanvas198 no teste de interacao

- **Evidência Windows:** captura do VS Code após `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao` registrou erro C++ em `qt/qt6_test6.cpp`: `PreviewCanvas198 was not declared in this scope` e erro em cascata `camera was not declared in this scope`, interrompendo Ninja com código 1. A suíte Qt dessa rodada não foi aprovada.
- **Causa confirmada na fonte do GitHub:** a classe está definida em `qt/preview_dialog_19_8.hpp` dentro do namespace `alaser::sim198`, enquanto o teste `textFillAlign198Test()` instanciava `PreviewCanvas198 camera;` sem o namespace.
- **Hotfix mínimo:** `sim198::PreviewCanvas198 camera;` no arquivo `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp`. Commit `64adac405015efe4ed2d42efa3534426871797e9` na branch experimental; validação de código lido novamente via GitHub confirmou a linha corrigida.
- **Não afirmar PASS:** alteração publicada e conferida estaticamente, mas não há recompilação Windows após o hotfix. Não modificar a geometria nem adicionar novas funções antes do reteste.
- **Próxima etapa:** preservar mudanças locais de `.vscode/` e outros arquivos, executar `git status --short`; se seguro, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8`; rodar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Se PASS, prosseguir com `-Abrir` e avaliação manual: texto reeditável, Fill com vazados, zoom/pan da prévia e alinhamento. `main`, `APLICATIVO`, máquina e emissão física preservados.


## 29/10/2026 — Falha de link Boost.JSON no teste isolado de texto

- **Evidência Windows:** `git pull --ff-only` concluiu, mas `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao` parou antes dos testes Qt, na compilacao do teste portatil `text_alignment_19_8_standalone.cpp` com `ld.exe: cannot find -lboost_json` e `collect2.exe: error: ld returned 1 exit status` (codigo 1).
- **Causa confirmada:** o script chamava g++ com `-lboost_json`, apesar de o projeto utilizar `src/boost_json_impl.cpp` (que inclui `<boost/json/src.hpp>`) para fornecer a implementacao no proprio target, como configurado nos alvos CMake. O erro nao prova ausencia de headers ou falha no codigo Qt.
- **Hotfix na branch experimental:** commit `d315c11501e176b2f8593a194930c9907755accf` troca a flag `-lboost_json` pela inclusao explicita de `src/boost_json_impl.cpp` na compilacao standalone. Confirmado por releitura do script remoto. `main` e `APLICATIVO` intocados.
- **Status:** ajuste estatico publicado; nenhum PASS de compilacao ou teste Windows depois deste commit. Proxima acao: `git status --short` e, se seguro (preservar `.vscode/`), `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8`; depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Somente se PASS executar `-Abrir`; manter o laser fisico desconectado, emissao bloqueada.


## 29/10/2026 — Hotfix do alinhamento de texto vetorial apos build Qt PASS

- **Evidencia Windows:** `git pull` atualizou a branch e o teste `-SomenteInteracao` ultrapassou a compilacao standalone Boost.JSON; `[2/2] Linking CXX executable alencar_test7_qt6.exe` confirmou link Qt. A suite parou no teste `--text-format-19-8-test` com JSON `{"error":"Alinhamento do texto vetorial nao corresponde ao ponto de ancora","status":"FAIL"}`; portanto nao houve PASS integral desta rodada.
- **Diagnostico de codigo:** `vectorTextOutlines` posicionava usando `QPainterPath::boundingRect()` de curvas Bezier da fonte, mas exportava polilinhas com aproximacao local de ~0,016 mm. O teste compara limites reais dos vertices exportados com a ancora em tolerancia 1e-6, tornando visivel a divergencia de extremos; a imagem isolada nao permite medir o deslocamento real.
- **Correcao na branch experimental:** commit `8456d9508ffbb4b3212a335770a558499df6b6e5` alinha a posicao X pelo envelope dos vertices finais (`actualLeft`, `actualRight`, centro), transladando todos os nos igualmente, sem mudar topologia, escala, fonte, ou geometria relativa. Fonte conferida novamente no GitHub, mas sem nova compilacao Windows.
- **Proximo gate:** preservar `.vscode/` e qualquer mudanca local, `git status --short`; se seguro, `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8`; executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Se PASS, `-Abrir` (suite completa), depois validacao manual de fontes Windows, texto reeditavel, Fill com vazado, alinhamentos e zoom da previa. `main`, `APLICATIVO` e comandos fisicos seguem intocados/bloqueados.


## 29/10/2026 — PASS Windows da suite focada de interacao apos hotfix de alinhamento

- **Evidencia recebida:** captura do VS Code, terminal PowerShell em `C:\\ProjetoAlencarLaser`, apos atualização e `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao`. A compilação Qt terminou em `[2/2] Linking CXX executable alencar_test7_qt6.exe`; o script encerrou com `Texto/alças PASS automático; mouse físico ainda obrigatório.` e retornou ao prompt.
- **Escopo do PASS:** testes automatizados do modo `-SomenteInteracao` passaram, incluindo o teste portátil de texto/alinhamento, regressões Qt de alças, painel de formatação de texto e texto reeditável/Fill/alinhamento/zoom previstas no script. A captura final não apresenta os relatórios JSON individualmente; o resultado é confirmado pela mensagem de sucesso do script.
- **Warnings:** `QDragEnterEvent` obsoleto e `-Wrange-loop-construct` no teste de preferências, sem interrupção. Limpeza posterior, separada dos gates de homologação.
- **Ainda pendente:** suite completa `-Abrir` e validação manual com mouse físico, fontes Windows e arquivos reais; máquina desconectada e emissão física bloqueada. Não mesclar `main`, nem atualizar a pasta `APLICATIVO` antes de homologar.
- **Próximo comando local:** após verificar `git status --short` e preservar a pasta não rastreada `.vscode/`, atualizar a branch se necessário com `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` e executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir` com laser desconectado. O app deve abrir somente se os gates automatizados completos passarem.


## 29/10/2026 — Suíte completa interrompida por assert legado da prévia Fill

- **Evidencia Windows:** `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -Abrir` compilou e passou os testes iniciais: Bézier 54/0, GRBL observador (read-only), regras zoom/Wi-Fi, Tesoura 19.7, Tesoura de intervalos 19.8, politica de recuperacao e codec `.alaser` v5 (`PASSED failed=0`). O gate Qt `Camadas e Desagrupar 19.8` terminou com JSON `{"error":"Previa opcional nao preencheu forma fechada","status":"FAIL"}`. O aplicativo nao abriu. **Suite integral ainda NAO passou.**
- **Diagnostico do fonte:** o teste legado `layersUngroup198Test` exigia `brush()!=Qt::NoBrush` em `v.pathItem({a,0})` apos `setFillPreview(true)`. Porem, o desenho foi alterado para manter contornos editaveis com `Qt::NoBrush` e gerar um unico `QGraphicsPathItem` de fundo por camada Fill, marcado `layer_fill_even_odd` e com `Qt::OddEvenFill`. O assert antigo e incompatível com o contrato visual novo; nao comprova falha de preenchimento real.
- **Correcao TESTE-ONLY** na branch experimental, commit `7afb83e00fb69d4244a106dc41925d5c201e2a70`: verifica que os contornos individuais continuam sem brush, que existe exatamente um backdrop Fill, que abrange dois retangulos da camada 2 e nao abrange um retangulo Line da camada 0, que usa paridade `OddEvenFill`, e que o backdrop nao recebe cliques/selecao. Nao alterou a implementacao de Fill nem CAM. Fonte conferida novamente no GitHub.
- **Status:** hotfix do teste publicado; nenhum novo build ou PASS Windows apos este commit. Antes de homologar, executar com maquina desconectada `git status --short`, preservar `.vscode/`, atualizar via `git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8` se seguro, rodar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteCamadas`. Se PASS, executar suite completa `-Abrir` e teste manual. `main` e `APLICATIVO` preservados.


## 29/10/2026 — PASS informado: teste focado Camadas e Desagrupar

- **Relato do operador:** apos receber orientacao de executar `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteCamadas`, o usuario respondeu **“Pass”**, indicando sucesso no teste focado com o hotfix do assert de Fill composto (commit `7afb83e`). Nao foi enviada captura nem JSON desta execucao, portanto registrar como **PASS reportado, nao auditado diretamente**.
- **Proximo gate:** executar a suite completa `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`, no Windows, com a maquina laser desconectada. Se todos os gates PASS, a candidata experimental deve abrir para testes manuais de texto reeditavel, vazados Fill, alinhamento, zoom/pan, Tesoura e curvas Bezier, usando copias dos projetos reais.
- **Sem homologacao ainda:** `main` e `APLICATIVO` preservados, nenhum controle fisico liberado; aguardar log da suite completa e validacao com mouse real antes de mesclar ou instalar.


## 29/10/2026 — auditoria real de interacoes e imagens — PR #12 experimental

- Usuario apresentou capturas do editor Qt6/Previa, relatorio JSON de simulacao, diagnostico local e arquivo `.alaser` real com texto, curvas e imagens de teste. Solicitou: previsualizacao menos pixelada; editor Bézier e Tesoura coerentes com Selecao; zoom durante desenho de linhas; reedicao por clique e conversao em caminhos; bitmap selecionavel, movel, gravavel com tratamento de imagem; moldura/alcas seguindo rotacao; ferramentas vetoriais e interface inteiramente em portugues.
- Auditoria independente do arquivo real (sem enviar desenho ao GitHub): 597 polilinhas/16.691 nos; 13 contornos Bézier/16 arestas cubicas; 576 marcadas `closed=false`, sendo 571 com fecho exato repetindo inicio/fim; um texto editavel `exemplo`, duas camadas Line/Fill; bitmap original NAO salvo no ZIP v5. Requer objeto de imagem persistente com backup e roundtrip antes de prometer salvamento real.
- Diagnostico JSON: 74 eventos/0 descartados, 10 recusas da Tesoura por agrupamento, 6 misses, 2 recortes, 7 movimentos de contorno Bézier, 4 de no; paint P95 51,0583 ms, max 415,543 ms; ~132 MB RSS; maquina desconectada. A simulacao reportou 76,1 s nominal vs. 322,5 s com aceleracao simplificada; nao usar estimativa como tempo real.
- Branch ISOLADA `fix/validacao-real-19-8-20261029` derivada da experimental; PR **#12 em rascunho**, base na branch experimental, nao em `main`. Ajustes publicados: zoom pela roda ao construir Linha/Bézier sem alterar nos anteriores; clique no interior do texto reeditavel; conversao explicita de texto em caminhos com Ctrl+Shift+C, Undo unico e geometria intacta; renderizacao de previa 2x adaptativa e pixmap suavizado. Regressões em teste portátil texto/alinhamento e testes Qt de editor, interação e conversao. Este primeiro lote NAO resolve ainda selecao unificada das curvas, Tesoura nativa por intervalo, bitmap como objeto salvo, fotogravacao, nem moldura OBB persistente.
- Documento de requisitos `DESENVOLVIMENTO/ESPECIFICACOES/AUDITORIA_REAL_20261029.md`; referencia LightBurn de selecionador, transformacoes, convert-to-path, bitmap, ajustes e preview. Funcionalidades futuras apenas especificadas: nenhuma API física habilitada.
- **Status honesto:** mudanças em fonte no GitHub; ainda sem compilacao e testes Windows na nova branch. Proximo gate preservando `.vscode`: `git status --short`, `git fetch origin`, `git switch --track origin/fix/validacao-real-19-8-20261029` se branch nao existir localmente e nao houver alteracoes conflitantes. Depois `-SomenteEdicao`, `-SomenteInteracao`, `-SomenteDesenho`, `-SomenteSimulacao`; com PASS, `-Abrir` e validar mouse real. Continuar proibido mesclar e instalar em `APLICATIVO` sem homologacao. Manter maquina laser desconectada, G-code, CAM, M3/M4 e disparos bloqueados.


## 29/10/2026 — PASS Windows informado de Desenho e Simulação na PR #12

- **Evidência de screenshot Windows:** terminal do VS Code na branch experimental, após `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteDesenho` e `-SomenteSimulacao`. O primeiro terminou `DESENHO C++ + Qt PASS sintetico; teste manual do mouse ainda necessario` após PASS de junção atômica, retângulo fechado, cruzamentos, magnetismo e Bézier com Undo/Redo. Há aviso `-Wunused-variable` em `preview_draw_join_19_8_standalone.cpp`, não bloqueante.
- **Simulação:** script terminou `SIMULACAO (MOVIMENTO + RASTER + CAMADAS/ROTA) PASS sintetico; controle fisico bloqueado`. Relatos no terminal: perfis, eixos, aceleração, continuidade, 1000 trajetórias aleatórias, Fill/hole, máscara raster, overscan e regras de camadas/ordem.
- **Gates da nova branch até agora:** `-SomenteEdicao` PASS na captura anterior; `-SomenteInteracao` PASS informado pelo operador; `-SomenteDesenho` e `-SomenteSimulacao` PASS nas capturas atuais. Estes resultados NÃO são homologação do aplicativo nem prova de que Tesoura Bézier por intervalos, bitmap selecionável/persistente e caixa de rotação orientada foram implementados.
- **Próximo gate:** executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir` com laser físico desconectado. Em caso de PASS integral, avaliar manualmente zoom durante linhas, reedição do texto, conversão para caminhos, nitidez/fluidez da prévia, preservação `.alaser`; usar cópia do projeto. Se houver FAIL, não prosseguir à homologação nem alterar `main`/`APLICATIVO`.


## 29/10/2026 — reedição do texto deslocada no zoom e proposta Curvar Texto

- **Relato do operador:** reabrir texto deixava contornos anteriores desenhados por trás; mover zoom/scroll desalinhava o campo de edição da posição do texto; solicita ponto azul de curvatura circular semelhante ao LightBurn.
- **Causa confirmada no código:** `QLineEdit(view.viewport())` era colocado somente uma vez com `mapFromScene` e coordenadas em pixels, com clamp aos limites do viewport; após zoom, transformações e scrolling o widget não seguia a origem de documento. Ao editar, os caminhos anteriores permaneciam renderizados, inclusive no Fill da camada.
- **Hotfix no PR #12 (branch `fix/validacao-real-19-8-20261029`):** sem modificar fonte original nem arquivo `.alaser`, IDs e paths do texto em edição são omitidos visualmente até confirmar/cancelar; overlay sincronizado pela âncora em mm em zoom/scroll/resize e controles de fonte/alinhamento; Esc cancela rascunho, Enter confirma; não finalizar automaticamente ao alterar zoom. Teste Qt de reedição, ocultação, âncora durante zoom/rolagem, Esc e Undo adicionado.
- **Pesquisa LightBurn oficial:** `Create and Edit Text` documenta `Bending Text`: ponto azul, drag para curvar ao redor de círculo, duplo clique restaura, opção `Distort`. `Apply Path to Text` é outro recurso e `Convert to Path` torna texto vetor não reeditável. No Alencar Laser a futura alça deve funcionar ANTES da conversão, manter texto/fonte + curvatura persistente no `.alaser`, opção sem deformar/deformar, histórico Undo/Redo, e aplicação a caminho como fase posterior. **Curvatura NÃO implementada neste hotfix.** Especificação em `DESENVOLVIMENTO/ESPECIFICACOES/AUDITORIA_REAL_20261029.md`.
- **Estado honesto:** código atualizado no GitHub e conferido estaticamente, mas esta rodada de hotfix requer compilação real Windows e testes guiados `-SomenteInteracao`, `-Abrir`, mouse e arquivo `.alaser` em cópia. Não mesclar `main` nem substituir `APLICATIVO`; máquina desconectada, comandos físicos bloqueados.


## 29/10/2026 — Teste real II: Tesoura obj.575, preview vetorial e zoom no rastreio

- O usuário enviou fotos do texto, prévia a 10x pixelada, traçado de imagem sem zoom, Tesoura com excesso inicial removido mas resto do contorno recusado, além de JSONs de simulação/diagnóstico e cópia do arquivo .alaser. Diagnóstico novo tem 3 cortes, 3 recusas `ambiguous` (objeto 575), 575 hover, 28 contornos de trace, paint P95 54.631 ms, pico 351.611 ms. Simulação nova tem 8345 movimentos e `machine_commands_sent=false`.
- Causa real verificada do obj. 575: caminho ABERTO de 4 nós com extremidade tocando o INTERIOR de segmento não adjacente (T-junction): o planner trata isso como tangência; recusa destacar trecho completo. Ajuste limitado de reconhecimento T de ponta externa + teste de regressão com coordenadas reais: cortar excesso, depois excluir triangulo remanescente fechado logicamente com Undo recuperando ambos. No cruzamento de vértice INTERIOR, Tesoura continua recusando. Outra melhoria permite recortar resto quando único toque terminal coincide com a linha usada como limite anterior.
- Previa offline: trocada rasterização fixa de 740x440 por `QGraphicsPathItem` vetorial (laser, deslocamentos, trecho atual), com cache incremental; deslocamentos vermelhos ocultos por padrão, reativáveis. Traçado de imagem: cena QGraphicsView/PathItem com zoom/pan reais, reset de enquadramento, atualização debounced 220 ms, presets de limpeza em português e preservação do zoom durante recálculos. Ainda não implementado salvamento de bitmaps/IA de rastreio/Bezier com tesoura completa. Testes GUI de zoom e preview adicionados.
- Branch `fix/validacao-real-19-8-20261029`, PR #12 RASCUNHO. Fonte publicada e verificada, **sem compilação e PASS em Windows depois desta leva de correções**. Não mesclar na main, não alterar APLICATIVO/homologada, manter hardware desconectado. Próximos testes na branch: `git status --short`; `git pull --ff-only origin fix/validacao-real-19-8-20261029`; `-SomenteInteracao` e `-Abrir` para incluir teste portátil `alencar_test19_8_scissors_span`. Depois inspeção manual do objeto 575 e prévia 1x/5x/10x. Arquivos do usuário continuam somente na conversa/área de testes; não estão no repositório.


## 29/10/2026 — hotfix de compilacao no teste da Tesoura (variavel duplicada)

- **Erro observado no Windows:** `TESTAR_19_8_WINDOWS.ps1 -Abrir` parou em `tests/test19_8_scissors_span.cpp:152`. O compilador reportou acesso `remainder->nodes` em `alaser::ScissorSpan`. Causa no proprio teste: `auto remainder=planner.candidate(...)` do caso triangulo 575 e `const auto* remainder=get(...)` do caso do circulo foram declarados no MESMO escopo.
- **Correcao minima publicada:** commit `3daed4305602d9b7c70e246be5bab4d60e63de31` renomeia exclusivamente o segundo ponteiro para `circleRemainder` nas tres referencias. Verificado por releitura no GitHub. `src/scissors_span.hpp`, codigo de corte, main, APLICATIVO e base aprovada permanecem intocados.
- **Status:** sem nova compilacao Windows apos o commit; nao alegar PASS. Operador deve cancelar prompt multiline `>>` com Ctrl+C se ainda estiver aberto, depois atualizar `git pull --ff-only origin fix/validacao-real-19-8-20261029`, executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir` e encaminhar primeiro FAIL se houver. Laser desconectado, versao experimental.


## 29/10/2026 — PASS C++ e falha de contrato legado da Tesoura no teste GUI

- **Resultado real Windows, suíte `-Abrir`:** build CMake/Ninja passou (`[3/3] Linking CXX executable alencar_test19_8_scissors_span.exe`). PASS: teste Bézier 54/0, observador GRBL read-only, regras zoom/Wi-Fi, Tesoura 19.7 e intervalos 19.8 (inclui caso real de dois recortes do objeto 575), recovery policy e codec v5 (0 falhas).
- **Falha de gate Qt:** `Regressao GUI das ferramentas` interrompeu antes de abrir editor com `Tesoura permaneceu ativa fora do trecho: ... hits=0 preview=0 overlay=0`. A geometria nao apresentava segmento sob o cursor, mas teste legado obrigava cursor `Qt::ArrowCursor`; implementacao nova conserva icone da Tesoura mesmo sem corte valido, com tooltip `sem intervalo seguro`. E uma discrepancia de contrato do teste, nao um recorte incorreto demonstrado.
- **Correcao de teste somente (commit `691c80ad12fe666be044875fc044532ba88a0526`):** requer ausê​ncia de highlight/preview quando `hits=0`, mantem cursor visual da ferramenta selecionada e verifica tooltip explicativo. Nao alterou processamento da Tesoura, posicoes ou documentos. Fonte conferida por nova leitura no GitHub, sem validacao Windows posterior.
- **Proximo gate:** com laser fisico desconectado, `git status --short`, se seguro `git pull --ff-only origin fix/validacao-real-19-8-20261029`, depois `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -Abrir`. Se falhar, usar o JSON e corrigir apenas a causa. Ainda nao homologar. Base aprovada, main e pasta APLICATIVO preservadas.


## 29/10/2026 — Comparativo LightBurn, traçado inspecionável e CI Windows Qt

- **Solicitação:** aproveitar parada do operador para estudar LightBurn e aproximar ferramentas/estabilidade da candidata 19.8; homologar o editor offline primeiro, deixando hardware como etapa independente. Pesquisa em documentação oficial LightBurn 2.x (catálogo, seleção, texto curvo, Tesoura, traçado, ajustes de imagem, booleanas, Offset, prévia e camadas). Plano no repositório: `DESENVOLVIMENTO/ESPECIFICACOES/COMPARATIVO_LIGHTBURN_GATES_20261029.md`.
- **Melhorias desta rodada na PR #12:** `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura` agora roda os testes C++19.7/19.8 ANTES do Qt, evitando PASS enganoso de apenas hover. Painel PT-BR de rastreio com alternância `Imagem original` e P&B, `Esmaecer referência`, `Mostrar nós do traçado` (até ~2000 marcadores para não congelar GUI), sobreposição vetorial com zoom/pan; testes sintéticos Qt adicionados. Não alterado o motor de corte nem persistência do bitmap.
- **CI GitHub:** workflow offline agora monitora `fix/validacao-real-19-8-20261029`, inclui Tesoura e texto C++ e dependências MiniZip/Zlib; execução do commit `63ffc651e` confirmou PASS portátil Linux. Adicionado gate de compilação Windows UCRT64 Qt6 via `msys2/setup-msys2@v2`, sem máquina; ainda não declarar PASS deste gate enquanto execução não concluir. CI não substitui mouse real. Fonte `qt6_test6.cpp` permanece monolítica (~354 KB); extração futura incremental com testes, evitando refactor amplo antes da homologação.
- **Próxima ação segura quando o operador voltar:** manter laser desconectado; `git status --short`, `git pull --ff-only origin fix/validacao-real-19-8-20261029` se sem conflito; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao` e `-SomenteTesoura`; se PASS rodar `-Abrir`. Depois testar com mouse físico cópia do projeto, 2 cortes do triângulo, texto/zoom e traçado original/P&B/fade/nós. Não mexer na main, pasta APLICATIVO e base homologada; nenhum comando físico.


## 29/10/2026 — Identidade Pégaso aprovada e integração opcional na candidata 19.8

- **Aprovação do usuário:** substituir visualmente a marca provisória por um pégaso marcante (asas laranja, corpo preto, casco com feixe laser), sem texto no símbolo. A identidade principal mantém assinatura ALENCAR LASER preta/laranja/prateada. Ref. imagens aprovadas na conversa: `1000896590.png` (símbolo) e `a_sleek_high_contrast_graphic_logo_branding_image.png` (horizontal). Não redesenhar a mascote nem inventar nova iconografia sem pedido.
- **Kit gerado fora do repositório:** `Alencar_Laser_Pegaso_Kit_v1.zip`, com original aprovado convertido lossless para PNG, ICO multirresolução, PNGs de 16 a 512 px, marca horizontal, cabeçalho compacto, splash com indicação offline, manifesto/hash SHA-256 e instalador PowerShell **simulacao por padrão**. Os binários ainda não foram adicionados ao GitHub; a ligação visual só acontece no Windows após instalar `DADOS/LOGO/PEGASO_V1`. O pacote foi vinculado na conversa, e não deve ser assumido como arquivo residente na maquina do operador.
- **Implementação isolada na PR #12:** `DESENVOLVIMENTO/FONTE/qt/branding_pegaso_19_8.hpp` localiza o kit a partir dos diretórios atual e de execução (com ancestrais), sem caminho absoluto obrigatório. `qt6_test6.cpp` carrega cabeçalho horizontal, ícone da janela e splash **somente no modo interativo**, mantendo fallback à marca anterior. A logo anterior não é alterada. O estado de segurança e a operação offline são preservados.
- **Verificação automática:** modo `--branding-19-8-test` cria fixture temporária e valida a descoberta, a prevenção de nomes fora da pasta e carregamento do ícone. Script `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteMarca` foi adicionado para compilar e executar o teste, gerando JSON. Workflow GitHub atualizado para incluir arquivos Qt, CMake e o script nos gatilhos; CI Windows está pendente de confirmação desta rodada.
- **Documentação operacional:** `DESENVOLVIMENTO/ESPECIFICACOES/IDENTIDADE_PEGASO_V1_20261029.md`, orientando simulação, cópia somente para o kit versionado, fallback e QA visual. Testar 16/32/48 px, escala 100/125/150%, barra de título, cabeçalho, splash e tema claro. Ainda falta incorporar o ícone ao recurso PE do EXE/instalador, se desejado; o `.ico` já existe no kit.
- **Não homologado:** nenhuma compilação real Windows nem inspeção visual da mudança nesta rodada; `main`, `APLICATIVO`, base aprovada e laser físico preservados.


## 29/10/2026 — CI Qt6 Windows verificada para marca Pégaso

- **GitHub Actions verificado na mesma rodada:** run `40983905046`, commit `83b3f66de`, com jobs `Qt6 UCRT64 - compile (sem laser)` e `C++20 offline: tesoura, texto, simulador` finalizados **SUCCESS** (passos de compilacao/tese passaram). Isto cobre compilação dos novos headers, interface e modo `--branding-19-8-test` no fonte presente nesse commit, mas não executa efetivamente a GUI com o kit de imagem.
- **PR run mais recente verificado:** `40983999178`, commit `ea70e68d5`, `SUCCESS`, somente teste portatil (Qt real no Windows ainda depende do gate local). A verificação de ícone no Windows com assets instalados, qualidade visual do cabeçalho/splash e reversão continua pendente.
- Fonte e documentação atualizados; **não declarar homologação** da versão 19.8 ou da aplicação da identidade em `APLICATIVO`.


## 30/10/2026 — Instalacao automatizada do kit Pegaso via pasta Downloads

- Usuário preferiu baixar somente `Alencar_Laser_Pegaso_Kit_v1.zip` em Downloads e executar comando simples no terminal VS Code, sem extração manual/menus.
- Criados `04_INSTALAR_PEGASO.bat` (raiz do projeto) e `SCRIPTS/INSTALAR_PEGASO_DOWNLOADS_19_8.ps1`. Após atualizar a branch experimental, comando único de instalação: `.\04_INSTALAR_PEGASO.bat`. `-Simular` opcional.
- Script valida branch e presença do header Qt, encontra ZIP em Downloads mesmo com sufixo `(1)`, valida SHA256 fixado `f1db7c250d72b9d702af3a1d493079d32e0fc3744465affee54edfa5bdaaecee` ANTES da extração e os 13 recursos do manifesto APÓS; extrai temporariamente, instala SOMENTE `DADOS/LOGO/PEGASO_V1`, move a identidade anterior para `BACKUP/IDENTIDADE_PEGASO` se necessário, e tenta restaurar em caso de falha. Não executa script embutido no ZIP, não sobrescreve logo original, `APLICATIVO`, main, EXE ou projetos .alaser.
- ZIP original validado na área de desenvolvimento por checksum e integridade ZIP/manifesto, sem discrepâncias. Script foi publicado/conferido no GitHub, mas ainda NÃO executado no PowerShell Windows do operador; não alegar instalação realizada nem homologação.
- Manual simplificado: `DESENVOLVIMENTO/ESPECIFICACOES/IDENTIDADE_PEGASO_V1_20261029.md`.


## 30/10/2026 — Diagnóstico real de seleção em massa, imagem e desempenho (sessão 19.8)

- **Origem:** JSON fornecido em conversa `Alencar_Laser_Diagnostico_TESTE_19_7(7).json` (apesar do nome externo, `version=TESTE_19.8`), schema `alencar-laser-diagnostics-4`; dados agregados apenas, sem caminho local, coordenadas de projeto ou conteúdo de imagem.
- **Ambiente:** Windows 11 21H2, Qt 6.12.0, máquina desconectada (`machine_transport=none`), zero sondagens de hardware, 48 eventos retidos e zero descartados; sessão de `1.123.721 ms` = ~18 min 44 s. 616 polilinhas / 18.427 nós / zero curvas Bézier no snapshot final; Undo disponível 35, Redo 5.
- **Ações registradas:** `marquee_selected` 8 (inclusive 614 e 583 objetos), `shapes_moved` 12, `redo` 14, `undo` 5, `shape_created` 2, `shapes_deleted` 2, `vector_text_created` 1, `objects_layer_changed` 1, `groups_ungrouped` 1. Importação/rastreio: `image_trace_started` 1 e `image_trace_created` 1 com 28 objetos; depois houve remoção de 27 objetos em evento distinto. **Não inferir qualidade do traçado apenas pelo número de objetos excluídos.**
- **Desempenho:** `paint_p50=9,8308 ms`, `paint_p95=46,1585 ms`, `paint_p99=52,9572 ms`, pico `338,7766 ms`, `input_to_paint_p95=44,3276 ms`, `hit_test_p95=25,8877 ms`, `wheel_p95=3,6374 ms`; RAM residente ~147,5 MiB; 2.438 amostras de pintura e 510 de roda. Esses tempos sugerem oportunidade de melhorar hit-test/seleção em massa e renderização, mas não são perfil por comando nem prova isolada de defeito na prévia.
- **Cobertura negativa importante:** `scissor_attempts=[]` e `scissor_hover_events=0` nesta sessão. Não afirmar que a Tesoura foi aprovada ou reprovada por este JSON. Também não mede diretamente objetos fora da mesa, redimensionamento/persistência do bitmap nem controles físicos.
- **Prioridades articuladas com prints do usuário:** (P0) tratar bitmap como objeto com seleção, alças, escala, Undo e save/reopen; (P0) tornar prévia restrita à área útil ou sinalizar excedentes explicitamente sem remover dados do projeto; (P1) otimizar seleção de centenas de objetos com índices/hit-testing e cache de desenho, protegendo interações >40 ms; (P1) refinar cabeçalho do Pégaso e agrupar ícones de alinhamento/espelho em faixa superior; (P1) separar ícones de edição dos controles de operação futura, sem habilitar emissão.
- **Estado:** análise documentada, **sem correção ou homologação inferida deste diagnóstico**. Continuar exclusivamente na branch experimental/PR #12, `APLICATIVO` e `main` preservados, laser desconectado.


## 30/10/2026 — Layout V2 + imagens reais + faixa operacional única (PR #12)

- **Pedido aprovado:** implementar o máximo do layout profissional inspirado em LightBurn, com ícones na barra superior (incluindo alinhamentos/espelhos) e **um único botão Pausar/Continuar**, além de cronômetros estimado/decorrido e comandos para futuro laser. Não abrir caminho físico nesta versão.
- **Interface:** cabeçalho PEGASO V1 reduzido para 48px, símbolo 38px com lettering Qt em vez de PNG horizontal com resíduos. Barra superior troca os textos mais frequentes por ícones Qt Painter com tooltips e nomes acessíveis; adicionados alinhamentos esquerda/centroX/direita/topo/centroY/base, espelhos X/Y e 90 graus. Menu Organizar preserva atalhos textuais. Barra inferior `operation_strip_19_8` reserva Iniciar, Pausar/Continuar (um só), Parar, Enquadrar e relógios estimado/decorrido — **todos os comandos físicos desativados** e tempos físicos mostrados apenas como travessão. Na PR, preview virtual alterna Pausar/Continuar no MESMO botão.
- **Área útil:** `src/preview_workarea_19_8.hpp` restringe percursos virtuais à mesa por Liang-Barsky; deslocamentos entre fragmentos válidos ficam OFF. Preview por padrão recalcula movimentos/tempo apenas dentro, com alerta de segmentos e mm ignorados; opção desmarcável para comparar plano completo. Não modifica documento nem emite G-code. Teste portátil introduzido, incluído em CI Linux e `-SomenteSimulacao` Windows.
- **Imagem:** `QtEditorLab` possui moldura selecionável, arraste, alça de redimensionamento proporcional e diagnóstico agregado; estrutura atual ainda oferece **apenas uma imagem** e não integra rotação, múltiplos bitmaps ou Undo/Redo de bitmap. A imagem se mantém sob os vetores na renderização. `qt/raster_project_19_8.hpp` guarda PNG, SHA256, pixelWidth/Height e posição/escala mm dentro do próprio ZIP .alaser v5 como campo versionado `raster_object_19_8` de `design/layers.json`. Reabre validando integridade ANTES de alterar documento. Recuperação também passa a salvar bitmap. Não enviar imagem física nem declarar fotogravação pronta.
- **Testes Qt novos:** `--raster-object-19-8-test` (pixels, escala, SHA corrompido, legado), `--layout-icons-19-8-test` (ícones, alinhamento, espelho vertical, comandos físicos desabilitados) ligados a `-SomenteInteracao`; porta de preview C++ `-SomenteSimulacao`. **Ainda sem PASS final comprovado no Windows após os últimos commits; pendentes mouse real, projeto de cliente, 1366×768 e escala 125%**.
- **Especificação:** `DESENVOLVIMENTO/ESPECIFICACOES/LAYOUT_V2_ICONES_IMAGEM_MESA_20261030.md` detalha aceitação e pendências. Branch experimental `fix/validacao-real-19-8-20261029` PR #12 rascunho. Não mesclar na main, não tocar `APLICATIVO`, laser desconectado.


## 30/10/2026 — CI final da rodada Layout V2: PASS (Windows Qt6 e C++ portátil)

- Workflow verificado: `https://github.com/alencarcell/alencar-laser/actions/runs/41061084518`, commit de código `365fa3802`, ambos os jobs **SUCCESS**.
- Job `C++20 offline: tesoura, texto, simulador` => compilação e regressões portáteis PASS, incluindo `preview_workarea_19_8_standalone.cpp` (corte a mesa).
- Job `Qt6 UCRT64 - compile (sem laser)` => compilação Qt6 Windows PASS; etapa adicional `Test PNG embedded and layout icons (Qt6 offscreen)` PASS em ambos os modos `--raster-object-19-8-test` e `--layout-icons-19-8-test`.
- Esse PASS é do runner Windows headless; **não equivale à validação visual nem à homologação em Windows do operador**. PENDÊNCIAS: testes `-SomenteInteracao`, `-SomenteSimulacao`, `-Abrir` na máquina real, seleção/movimentação/redimensionamento da imagem no mouse, salva/reabre cópia .alaser real, escala DPI 125/150%, clip de projeto externo e inspeção do cabeçalho Pégaso.
- Segurança permanece: `APLICATIVO` e `main` sem alterações, máquina desconectada e comandos físicos desabilitados; PR #12 rascunho. Não liberar versão final por este CI.


## 30/10/2026 — Curvas, Tesoura e pré-validação da mesa (experimento isolado)

- Após prints do operador, identificada causa comprovada no código: `CurveDragSession::cutContourAtEdge` dividia a Bézier sem remover nenhuma aresta, portanto podia aparentar que a Tesoura não funcionava. Reescrito para remover a aresta matemática clicada (não um intervalo topológico LightBurn), preservar outras cúbicas, gerar dois caminhos quando aplicável e manter Undo/Redo. Overlay Qt passa a destacar a mesma aresta inteira.
- Botões legados `nodes` e `bezier` ocultos da barra compacta, sem apagar as rotinas; `new_bezier` renomeado “Desenhar caminhos e curvas”, com novo glifo e tooltip. Duplo clique na curva em Seleção acessa edição de âncoras/alças. Seleção/escala/rotação completas ainda pendentes de testes e evolução.
- Prévia mostra banner visível independente da rolagem e da opção de corte; área de validação agora usa mesa física configurada, não a área de desenho. Diagnósticos distinguem laser-ON de travel-OFF. Operação real, CAM, G-code e controles físicos permanecem bloqueados.
- Teste portátil novo de Tesoura Bézier ligado aos gates Windows `-SomenteTesoura` e `-Abrir`, e teste Qt de desenho ajustado ao contrato de remoção real. Fonte e documentação publicadas na branch `fix/curvas-tesoura-preflight-19-8-20261030`, derivada da PR #12. Documento de escopo: `DESENVOLVIMENTO/ESPECIFICACOES/CURVAS_TESOURA_PREVALIDACAO_MESA_20261030.md`.
- **NÃO COMPILADO NO WINDOWS / NÃO HOMOLOGADO após essas alterações.** `main`, `APLICATIVO` e base aprovada preservados; não mesclar nem habilitar máquina sem teste completo. Executar com laser desconectado os gates `-SomenteTesoura`, `-SomenteDesenho`, `-SomenteSimulacao`, depois `-Abrir` e inspeção manual em cópia de .alaser.


## 30/10/2026 — Regressão Desenho após novo contrato da Tesoura (CI portátil PASS)

- Windows `-SomenteTesoura` do operador: PASS no motor C++ 19.7/19.8, Bézier nativa e GUI Qt (foto terminal). Após a correção de aspas do script (`a8ab3326`), o teste `-SomenteDesenho` chegou ao teste C++ `preview_draw_join_19_8_standalone.cpp:115` e falhou na asserção `lifecycle.committed().contours.size()==2` (retorno Windows `-1073740791`).
- **Causa comprovada:** contrato de teste desatualizado. Esperava a divisão não destrutiva da única aresta cúbica em dois caminhos, mas o novo motor remove efetivamente essa aresta. O teste não comprova regressão funcional por si só.
- Atualizados testes do ciclo de vida Bézier aberto e fechado em `tests/preview_draw_join_19_8_standalone.cpp` (commit `eabe2a28`) e regressão Qt `qt6_test6.cpp` (commit `e5820ced`): remoção real, seleção, ausência de contorno fantasma, Undo fiel, Redo e abertura do anel após cortar. Nenhum afrouxamento de asserção ou reativação da divisão antiga.
- Adicionado workflow offline de compilação+execução de três regressões C++ em Linux, `.github/workflows/portable-curves-19-8.yml`. GitHub Actions run `41096272907`, commit `fc417702`, job `portable-offline`: **SUCCESS** (desenho/união, tesoura nativa e prévia de mesa). Verificação de sintaxe PowerShell na mesma rodada também PASS. Isso NÃO equivale à validação GUI Qt em Windows nem homologação.
- Próximo gate local: `git status --short`, `git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030`, `-SomenteDesenho`; se PASS, `-SomenteSimulacao` e `-Abrir`. Laser desconectado e `main`, `APLICATIVO`, instalação homologada preservados.


## 30/10/2026 — Refinamento de alertas de mesa e alinhamento inspirado no LightBurn

- Usuário validou visualmente que o banner de limites funciona, porém esclareceu: **objetos totalmente externos são artes de reserva; não devem provocar alerta**. Na captura: 325 trechos externos + 4 cruzando a mesa. JSON de prévia separado: laser ON 197 externos, 4 recortados, 68,98 mm de trajeto excluído. Ajustado `preview_workarea_19_8.hpp`/ `preview_dialog_19_8.hpp`: somente laser ON que CRUZA a borda aciona aviso, inclusive quando a opção de clipping é desmarcada. Estatísticas de caminhos externos continuam exportadas mas não aparecem como perigo; preservação da geometria original. Testes incluem só fora + fora/interior + cruzamento.
- Alinhamento: acrescentados `quick_align_6` (coincidir centros X+Y com objeto de referência) e `quick_center_bed_19_8` (centralizar geometria selecionada na mesa física, não alterar formas de reserva não selecionadas). Ícones e dicas próprios; centralização considera grupos completos, não apenas membro selecionado. Adicionada ação no menu Organizar > Alinhar formas. Testes Qt de presença de botões, alinhamento central e reserva inalterada.
- Relatório de diagnósticos enviado: `command_error=1`, 0 curvas Bézier nativas no estado coletado, 575 contornos e 14.433 nós polilinha, paint P95 33,7742 ms, max 437,87 ms. A causa do erro de comando não está descrita; não atribuir a ferramentas. Performance é uma observação, ainda sem ajuste.
- Código na PR #13, `fix/curvas-tesoura-preflight-19-8-20261030`, sem mesclar main. `APLICATIVO` e hardware físico intactos. Revisão Qt/Windows ainda precisa de `-SomenteInteracao` e `-Abrir` + mouse real e projetos .alaser; não declarar versão final.


## 30/10/2026 — Teste Qt de alinhamento após centralização na mesa

- Print do operador em `TESTAR_19_8_WINDOWS.ps1 -SomenteInteracao`: compilação Qt concluiu, mas o caso `--layout-icons-19-8-test` retornou `Espelhamento vertical nao refletiu os nos do objeto`. A etapa seguinte `-SomenteSimulacao` já havia iniciado e não possui resultado final no print; não inferir PASS.
- Análise do código: teste Qt usava indevidamente a soma vertical fixa `35`, válida somente antes da nova operação “Centralizar seleção na mesa”. Depois da centralização, as posições são transladadas; `Editor::mirrorY(ids,axisY)` usa o eixo atual fornecido pela interface. Não existe evidência neste print de erro real no motor de espelhamento.
- Ajuste na branch experimental: capturar `boundsOf` **atuais** do objeto, verificar reflexão de **todos** os nós usando `y' = minY + maxY - y` e `x'=x`, garantir uma única transação no histórico e Undo de geometria exata (commit `3812a2e9`). Não foi alterado o algoritmo `mirrorY` nem o código de controle físico.
- Regressão portátil adicional `tests/alignment_mirror_19_8_standalone.cpp`: alinha ambos os centros, translada seleção ao centro da mesa, garante que objeto de reserva fora da mesa permaneça intocado, reflete com eixo atualizado e faz Undo. Incluída no workflow CI Linux.
- Teste Qt Windows ainda obrigatório: `git status --short`; `git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030`; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`. Se PASS, concluir `-SomenteSimulacao` e depois `-Abrir`. Não homologar sem prova.


## 30/10/2026 — Relatório real: Bézier sem redimensionamento e editor preso em gesto

- Dois screenshots do editor Qt 19.8 em Windows: uma curva Bézier v5 selecionada e movida, mas sem alças de escala/rotação, enquanto Tesoura e exclusão funcionam. Outro print registra a mensagem `Conclua ou cancele o arraste antes de outro comando.`; o operador relata bloqueio ao deixar ferramenta Desenhar curva com geometria em curso. Eventos do coletor incluem `bezier_contour_moved` e recortes de Tesoura, mas não demonstram redimensionamento/recuperação.
- **Causa no código:** seleção nativa era excluída do desenho e do hit test das 13 alças (`paintSelectionHandles`, `beginSelectionTransform`), ainda que a curva fosse selecionável. O clique no botão de outra ferramenta era processado dentro de `run()`, que abortava se `busy()` estivesse ativo; drafts Bézier e movimentos cuja liberação do mouse não chegasse bloqueavam a transição.
- Alterações codificadas **apenas na branch da PR #13**: `CurveDragSession::transformSelectedContour` aplica afim sobre âncoras e ambas as alças Bézier em uma única transação Undo/Redo, sem achatamento poligonal. Qt permite usar alças de redimensionamento e rotação da seleção também para Bézier nativa, fazendo preview e commit no mouseRelease; Undo deve restaurar geometria original.
- Troca de ferramenta passa a concluir rascunho válido e descartar incompleto; movimentos interrompidos são cancelados para liberar `busy()`. Esc mantém função de cancelamento. Atualizados tooltip e status do editor. Nenhum comando físico, G-code, transporte laser, main ou APLICATIVO alterado.
- Regressões: `tests/native_transform_19_8_standalone.cpp` (escala, rotação, deslocamento, controles exatos e Undo/Redo) incluído em CI Linux e em `-SomenteDesenho` Windows; o teste Qt `--draw-join-19-8-test` agora arrasta alça e reproduz rascunho Bézier abandonado e liberação de mouse perdida.
- **Pendência obrigatória:** compilar Qt6 Windows e testar as novas alças com mouse físico, zoom diferente, Ctrl+Z, clique fora, troca de ferramenta, fechamento e reabertura de cópia .alaser. Ainda não homologado; PR #13 permanece rascunho.


## 30/10/2026 — Intervalo entre linhas (Line Interval) por camada — preview offline

- Pedido: configurar o espaçamento físico nominal entre linhas de preenchimento com valores como 1,000 / 0,900 / 0,800 / 0,700 mm, principalmente nas gravações identificadas como M4. **Distinção essencial:** M4 (potência dinâmica) é independente de Line Interval (distância entre as linhas raster/Fill); não alterar M4 automaticamente ao mudar o intervalo.
- Implementação experimental: `Layer::lineIntervalMm` com padrão 0,200 mm, faixa validada 0,050–5,000 mm; entrada numérica de três casas em **Configurações da camada** e indicador LPI teórico (25,4/intervalo). Parametrização registrada no mesmo metadado `editor-meta-1`, sem migração destrutiva; projetos antigos sem a chave assumem 0,200 mm. Configuração/Undo/Redo de camadas preservados.
- `raster::appendFill` aceita opcionalmente `useLayerIntervals`: cada camada Fill usa seu próprio pitch em varreduras simuladas. O modo global permanece disponível e a imagem bitmap mantém seu intervalo global independente. Na prévia, opção `Fill: usar intervalo entre linhas salvo por camada`, acionada automaticamente quando existe Fill configurado com espaçamento diferente do padrão, sem mudar o comportamento inicial de projetos antigos.
- Exportação JSON inclui `scan_line_interval_mm` global, `fill_layer_intervals_enabled` e `fill_layer_intervals_are_machine_commands=false`. M3/M4/potência seguem metadados ilustrativos; nenhum G-code, hardware, movimento ou disparo foi habilitado.
- Regressão portátil adicional `tests/line_interval_19_8_standalone.cpp`: Fill 1,000mm (M4) em uma camada versus Fill 0,500mm (M3) em outra, fallback global 0,200mm, metadados antigos, Undo/Redo e rejeição de 0,010mm. Integrada ao CI Linux e ao gate `TESTAR_19_8_WINDOWS.ps1 -SomenteSimulacao`. Teste Qt `layerRoute198Test` ampliado com persistência, validação e fallback.
- NÃO HOMOLOGADO: aguardar compilação Qt Windows, gate `-SomenteSimulacao`, relatório de prévia e teste manual. A pendência anterior da Tesoura Bézier (registro da coleta e Undo/Redo/linha cinza) permanece ABERTA, sem presumir correção.


## 30/10/2026 — Correção focada: Ctrl+Z/Y e diagnóstico após Tesoura Bézier

- Último print do operador: `-SomenteSimulacao` **PASS** no Windows; console informa `PASS: 1.000/0.500mm per-layer Fill, 0.200mm global fallback, old metadata, undo/redo; M3/M4 independent; offline only` e `SIMULACAO (MOVIMENTO + RASTER + CAMADAS/ROTA) PASS sintetico; controle fisico bloqueado`. Nenhuma homologação manual implícita.
- Relato anterior: Tesoura Bézier nativa não aparecia em `scissor_attempts`; após cortar, Ctrl+Z/Y não restauravam/refaziam e aparecia apenas traço cinza.
- **Causa observada no código:** `QtEditorLab::keyPressEvent` enviava Ctrl+Z/Y ao histórico das polilinhas quando `nativeMode_` ficava falso após um corte que removia a última Bézier. Botões, atalhos e menu também possuíam encaminhamentos distintos. `cutNativeAt` não atualizava `lastEditorGenerationSeen_` nem chamava `scissorDetail`, por isso o relatório não incluía `cut_native`.
- Correção isolada na branch `fix/curvas-tesoura-preflight-19-8-20261030`, PR #13: `history(bool redo)` torna-se despacho único para histórico Bézier/polilinha; teclado, toolbar, atalhos e menu compartilham a mesma lógica. Após corte, o histórico nativo continua ativo mesmo sem curvas visíveis. `cutNativeAt` grava `lastEditorGenerationSeen_`, informa o corte ao coletor opt-in por `scissorDetail("cut_native", objeto, aresta)` e registra evento de Tesoura. `nativeHistory` também emite eventos `bezier_undo` e `bezier_redo`. Resumo do JSON ganhou contadores separados de undo/redo Bézier.
- Em `drawAndJoin198Test`, teste real do Qt com mouse sintético foi ampliado: ativa coleta, corta Bézier de uma única aresta, comprova ausência da curva, verifica `scissor_attempts`, usa Ctrl+Z/Ctrl+Y e botões Desfazer/Refazer, confere o caminho restaurado exatamente e os contadores no JSON. CI Windows do próprio projeto usa `--draw-join-19-8-test`.
- **Estado:** alterações commitadas; CI Qt Windows da rodada deve ser consultado antes de declarar PASS. Teste com mouse físico e cópia `.alaser` ainda obrigatório. PR rascunho, `main`, `APLICATIVO`, comandos de máquina e repositório privado preservados.
- Para retomar: `git status --short`, `git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030`, `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho`, seguido de `-Abrir` somente se a suíte automática passar; testar coleta e Ctrl+Z/Y com laser fisicamente desconectado.

- Ajuste de feedback visual adicional: após Undo/Redo Bézier, barra de status informa ação executada, quantidade de curvas e que a linha cinza é uma curva não selecionada (commit `c4fe237f`). Esse texto não substitui o teste de geometria; serve para evitar ambiguidade para o operador.


## 31/10/2026 — Falha isolada do coletor na Tesoura Bézier (print real)

- Operador executou `-SomenteDesenho` na branch experimental: compilação Qt completou, teste portátil de união de caminhos e teste de transformação Bézier passaram; a etapa `--draw-join-19-8-test` falhou com `Coletor nao contou Tesoura e historico nativos` (código 1). O print mostra que o teste avançou pelas verificações geométricas de corte/Undo/Redo antes de chegar à coleta.
- **Causa comprovada no código:** `DiagnosticsCollector::record` só aceita eventos de uma lista explícita. `QtEditorLab::cutNativeAt` e `nativeHistory` enviavam `bezier_contour_scissor_cut`, `bezier_undo` e `bezier_redo`, mas os nomes não apareciam na lista de permissão de `src/diagnostics_collector.hpp`. `LabWindow::captureEvent` captura a exceção de evento desconhecido e não a propaga; por isso geometria funciona, mas o relatório fica sem contagens.
- Correção na branch da PR #13: acrescentados nomes permitidos para edição Bézier nativa sem relaxar o filtro, sem coordenadas e sem coleta automática. Novo teste `tests/diagnostics_native_events_19_8_standalone.cpp` exige eventos de corte, Undo, Redo, rejeita nomes arbitrários e confirma que coleta OFF não registra. Integrado ao GitHub Actions C++ portátil, CMake e ao gate Windows `-SomenteDesenho`.
- A regressão Qt existente continua exigindo coleta ativa, `cut_native` em `scissor_attempts`, contagens de corte/Undo/Redo e restauração idêntica da geometria. Nenhuma asserção foi desativada.
- **Estado:** alteração publicada, aguarda os resultados automáticos e teste Windows com mouse real. Controle físico continua bloqueado; `main`, `APLICATIVO` e repositório privado preservados.
- Próximo teste local após confirmar `git status --short` sem alterações conflitantes: `git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030`; `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho`.


## 31/10/2026 — Relatórios reais: Bézier fora da prévia, Fill, ícones, nós e rastreamento

- **Fontes do operador:** capturas do editor Qt 19.8 e dos painéis de simulação e rastreamento; `Alencar_Laser_Previa_19_8(4).json` e `Alencar_Laser_Diagnostico_TESTE_19_7(10).json`.
- **Evidência:** Bézier fechada selecionável/editável no editor, mas ausente na prévia, pois `showPreviewDialog` montava `Plan` somente do `EditorState.doc` (polilinhas), separado de `CurveDocument`. O relatório da prévia não tinha campos de curvas nativas. A simulação exportada é experimental, `machine_commands_sent=false`, Fill ligado e intervalos por camada ativados. A coleta registra 9 cortes Bézier, 6 Undo, 1 Redo e 4 `command_error` (motivos não incluídos no JSON; não atribuir sem reprodução).
- **Preview / Fill resolvido no código experimental:** `preview_native_19_8.hpp` produz uma **cópia temporária** de polilinhas só para prévia, a partir das curvas cúbicas nativas (flatten com tolerância em mm, limites de nós e IDs independentes). Ela preserva a propriedade `closed`, aplica um objeto temporário por contorno, passa por rota Line ou Fill e respeita a camada. A curva matemática original não é alterada, exportada como polilinha ou quebrada em centenas de formas.
- **Camada Bézier persistida:** `CurveContour.layer` e chave opcional `layerId` no `bezier-1` do projeto v5; arquivos antigos recebem camada 0. Novas Bézier herdam camada ativa e o botão `Enviar seleção para camada ativa` também transfere curva nativa. A troca de camada usa uma transação Undo/Redo nativa, sem `attachNative` que apagaria o histórico.
- **Prévia:** `showPreviewDialog` passa a receber a sessão de curvas nativas, utiliza `planningSource` unido em `build`, otimização de rotas, Fill e passagens. Relatório JSON ganha `native_bezier_contours`, `native_bezier_closed`, `native_bezier_open`, `native_bezier_virtual_segments`, `native_bezier_missing_layer_fallbacks`. Não habilita G-code/GRBL.
- **Edição de nós:** clique direito no botão Desenhar caminhos e curvas mostra ações para editar alças, armar clique para inserir âncora e excluir nó ativo (também Shift+clique e Delete). `armNativeInsertNode()` usa a mesma divisão cúbica exata e permite Esc para cancelar; histórico nativo e evento `bezier_node_inserted` preservados. Resta testar menu e interação manual no Windows.
- **Barra superior:** `open_other` e `save_as` passam a ter glifos diferentes dos de abrir/salvar; 32px de altura homogênea, agrupamento e separador dos alinhamentos, ícones um pouco maiores.
- **Rastrear imagem P&B:** novo parâmetro opcional `antiStaircase` reduz microdegraus de pixels somente em trechos com vários giros curtos; não altera máscaras originais nem cria nós; a UI oferece Anti-serrilhado ligado para perfis normal/logotipo, desligado em texto/traço fino. Arredondamento de cantos continua separado e opt-in. **Não é vetorização Bézier perfeita:** rever textos pequenos e vazados; o resultado é aproximado e editável.
- **Regressões adicionadas:** `tests/preview_native_bezier_19_8_standalone.cpp` (fechada Fill, aberta Line, layer metadata/codec legados, fonte intacta e Undo/Redo); CMake, Linux CI e `-SomenteSimulacao` Windows. GUI `--draw-join-19-8-test` ampliado para inserção de nó na aresta/Undo. Teste Qt de rastreamento inclui anti-serrilhado e preservação de furos.
- **Portões de homologação:** compilar Qt6 Windows e executar `-SomenteDesenho`, `-SomenteSimulacao`, `-Abrir`; inspeção manual em cópia `.alaser`, Bezier em Fill na camada transferida, visual de traçado, zoom e seleção; CI não substitui mouse físico.
- **Segurança:** somente branch `fix/curvas-tesoura-preflight-19-8-20261030` PR #13; `main`, `APLICATIVO` e repositório privado preservados. GRBL/emissão física continuam bloqueados.


## 31/10/2026 — Análise de cinco arquivos reais de referência (inspeção privada, sem importação destrutiva)

- Entraram relatório de prévia JSON, diagnóstico JSON, projeto LightBurn LBRN2, log extenso de LightBurn e pacote de suporte codificado. **Não copiar os arquivos originais nem dados de licença/sistema/usuário para repositórios ou fixtures.** O pacote de suporte tem dados de licença/preferências e deve permanecer exclusivamente local.
- **Prévia 19.8:** `native_bezier_contours=1`, `native_bezier_closed=1`, `native_bezier_virtual_segments=41`, `native_bezier_missing_layer_fallbacks=1`, `fill_layer_intervals_enabled=false`, `scan_line_interval_mm=0.2`, `partial_workarea_warning=false`; nenhuma emissão física. **Pendência identificada:** uma curva nativa foi redirecionada a camada existente porque seu `layerId` não correspondia a `EditorState.layers`; parâmetros de Fill/intervalo/velocidade podem divergir. Adicionado banner específico `preview_native_layer_warning_19_8` e campo `native_bezier_layer_mapping_requires_review`, sem modificar geometria ou habilitar GRBL.
- **Diagnóstico 19.8:** 3 `bezier_contour_scissor_cut`, 3 `bezier_undo`, 11 `undo` polilinha, nenhum `bezier_redo` na janela coletada. `scissor_attempts` tem os 3 cortes `cut_native`, `events_dropped=0`; não afirmar Redo real validado nessa sessão.
- **Projeto LBRN2 real:** 22 caminhos Path em 2 grupos, 361 primitivas cúbicas B e 2561 linhas L. Um Path reúne 2515 primitivas L desconectadas (2514 transições de cadeia), incluindo 356 segmentos com a mesma origem/destino: o importador conservador, que exige uma cadeia contínua por Path, **deve rejeitar o projeto completo sem modificação parcial**. Configuração registrada no arquivo (não prescrição física): Scan, intervalo 0,08 mm, 2 passes, velocidade bruta 200, potência máxima bruta 40 e secundária 20. Não importar settings de máquina.
- **LightBurn Log real:** enorme volume de pacotes e eventos de status; inclui respostas observadas `<Run`/`<Idle`/`<Alarm`, consultas e comandos. Não confundir com operações do Alencar, que continua offline; resposta no log não prova a execução correta de uma gravação. Nenhuma licença ou identificador foi registrado aqui.
- **Correção no inspetor somente leitura:** `reference::readLightBurn` agora inventaria grupos aninhados usando pilha de frames, conta transições desconectadas e segmentos de índice igual, e informa incompatibilidade sem afirmar importação integral. Novo teste Qt com LBRN2 completamente sintético (quatro Shapes, dois Groups) exige relatório correto e recusa atômica do importador quando houver cadeias desconectadas.
- **Teste C++ da prévia:** fixture sintética de curva com camada inexistente exige contagem de fallback preservando `layer=999` no original.
- **Próximos gates:** GitHub Actions Qt Windows e C++ portable; no Windows local `-SomenteDesenho`, `-SomenteSimulacao`, e `-Abrir` sem laser conectado. Para projeto real com muitos traços desconectados, primeiro inventariar ou preparar importador de subpaths seguro com representação explícita de pontos/traços, sem descartar segmentos.
- **Segurança:** `main`, `APLICATIVO` e controle físico não alterados; branch experimental PR #13, repositório privado.


## 31/10/2026 — Edição cirúrgica: remover só nó/lado da forma selecionada

- Solicitação do operador: adicionar ação que remova **apenas um nó ou um lado** da forma selecionada, não a forma inteira nem outros objetos. O relatório local `Alencar_Laser_Diagnostico_TESTE_19_7(20261031-170205).json` mostra `bezier_node_moved=6`, `bezier_undo=4`, `command_error=2`, `marquee_selected=3` e uma exclusão de `575` formas na mesma sessão. As duas falhas de comando não têm causa detalhada; **não inventar explicação**. O relatório permanece fora do Git.
- Ações existentes no motor: `Editor::erase(key,nodeId)` remove nó de **um subcontorno**; `Editor::removePolylineEdge(key,edge)` remove só uma aresta de polilinha e abre um contorno fechado (ou separa duas partes válidas de caminho aberto); `CurveDragSession::deleteAnchor` e `cutContourAtEdge` fazem operações nativas no Bézier. Cada operação integra histórico, Undo/Redo. Não foi reescrito o motor.
- **Implementado no Qt, ainda experimental:** clique direito sobre geometria do objeto **já selecionado** mostra `Remover só este nó da forma selecionada` ou `Remover só este lado da forma selecionada`; entradas ficam desativadas se o cursor não encontra nó/segmento; requer exatamente uma seleção poligonal e subcaminho ativo. No modo Bézier, menu de contexto local só reconhece âncora/aresta pertencente à curva previamente selecionada. Cortes Bézier contextuais passam chave explícita ao cortador (protege curvas coincidentes). Nenhum comando toca automaticamente outras formas.
- Novos eventos opt-in permitidos: `polyline_node_deleted` e `polyline_edge_deleted`, sem coordenadas. A barra de status descreve o comando e informa `Ctrl+Z`; tooltip da ferramenta Seleção ensina o clique direito.
- Teste portátil `tests/selected_local_edit_19_8_standalone.cpp` cobre nó de retângulo, lado de contorno fechado, lado interno de caminho aberto que separa duas peças, objetos vizinhos intactos, operação inválida sem efeitos colaterais, Bézier nativa, Undo/Redo e privacidade da coleta. Incluído no CMake, CI portátil e `-SomenteDesenho` no Windows.
- **Pendências de segurança e ergonomia:** diálogo de confirmação antes de excluir seleção múltipla grande (575 objetos no relatório), confirmação manual de menus/hover/DPI em tela real e rastreio mais específico dos dois `command_error` **sem registrar texto sensível**. Essas pendências não foram ocultadas nem declaradas resolvidas.
- **Gate:** o projeto permanece exclusivamente na branch `fix/curvas-tesoura-preflight-19-8-20261030`, PR #13 não homologada; `main`, `APLICATIVO`, liberação pública e comandos físicos GRBL intocados. Testar offline com laser desconectado e cópia de `.alaser` após passar CI Qt Windows.

- **Complemento de segurança nesta rodada:** quando a ação `Excluir formas selecionadas` receber **25 ou mais objetos**, o Qt solicita aprovação explícita, com `Não` como padrão. Se o usuário cancelar, nenhum objeto é apagado, nenhuma transação de Undo é consumida e o cancelamento não vira `command_error`. Essa confirmação nunca interfere na remoção localizada de um nó/segmento. O teste Qt `drawAndJoin198Test` cobre cancelamento, confirmação e recuperação dos 25 objetos por Undo. Status: modificação publicada, ainda sujeita à compilação e teste manual.


## 31/10/2026 — Integração autorizada do pacote Work na branch experimental

- Base conferida no GitHub: `2739b2b43d9147844e782d06139eb0e3e2d258da`, branch `fix/curvas-tesoura-preflight-19-8-20261030`. Os 83 blobs do ZIP de fontes originais coincidem com a árvore remota; não havia commits posteriores na branch no início da integração. Atualização protegida por expected_sha, sem force.
- Integradas somente quatro diferenças funcionais do pacote: CMake (Qt6::Network nos testes 15/16/17 e alvo de estresse), qt6_test6.cpp (contagem de grupos na exclusão em massa, regressão de menus locais e versão de diagnóstico 19.8), qt6_test17_mission.cpp (contrato atual de clique vazio/Escape) e novo teste local_edit_stress_work.cpp. Diferenças de newline final em outros arquivos foram descartadas; nenhum fonte anterior foi substituído em bloco.
- O workflow Qt Windows foi ampliado para compilar os alvos alterados e executar a suíte offline: desenho/menus, 20 modos adicionais de GUI, TESTE_17 interação/singleton em DPR 1/2, persistência em processos separados e estresse de 3.230 casos. Evidências em artifact windows-work-regression.
- Estado neste commit: testes Windows aguardam execução no GitHub Actions. Resultados Linux do pacote são históricos e não são prova Windows. Registrar os links e conclusões reais em complemento após a execução.
- Main permanece em `397341d139d974e0f3d90b79ed28604e697a81b1`. Sem merge em main, sem instalação em APLICATIVO e sem liberação física. Validação manual com mouse/DPI, projeto real e recuperação após queda de energia permanece pendente.

## 31/10/2026 — Bloco experimental de atalhos offline (pendências da barra lateral)

- Captura Windows do operador: a interface TESTE_19.8 abriu com 575 contornos, 13.415 nós, diagnóstico opt-in ativo e banner `MÁQUINA DESCONECTADA | EMISSÃO BLOQUEADA`; a imagem indica funcionamento visual, NÃO certifica ferramenta, desempenho ou persistência.
- A barra lateral tinha entradas cinza para funcionalidades **já existentes em outros menus**. Esta revisão liga `⊞ Alinhar` às mesmas sete ações de Organizar (esquerda/direita/topo/base/centros X/Y/ambos) e ao centralizador da mesa; não promete distribuição automática.
- A nova ação `⌗ Medir seleção` consulta a caixa envolvente do selecionado (inclusive grupos expandidos e prévia de curva nativa quando disponível), informando largura/altura e origem em milímetros na barra de status. É somente leitura: não calcula área real, comprimento do contorno, folga ou percurso laser.
- O atalho `L` reutiliza o importador conservador `.lbrn2`: exige confirmação antes de substituir arte e recusa qualquer projeto incompatível integralmente, sem importação parcial silenciosa e sem importar parâmetros físicos. Não é suporte completo ao LightBurn.
- O botão `Pré-visualizar percurso` no painel CAM passa a abrir a **mesma prévia virtual** já existente, SEM geração de G-code, disparo ou transmissão. Os outros controles CAM continuam bloqueados.
- **Continuam propositalmente desativados:** Offset, operações booleanas (União/Subtração), SVG, DXF, potência, velocidade, passes de trabalho e qualquer comando físico; implementar só com motor próprio, rollback e regressões.
- Teste Qt existente `--layout-icons-19-8-test` ampliado: verifica presença/estado dos quatro atalhos, submenu de alinhamentos, medição vazia e com seleção, e invariância da geometria após medir. **Pendente comprovar PASS Qt Windows desta alteração no Actions**, inspeção manual do botão, teclado, zoom/DPI e arquivos reais.
- Rollback: apenas `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp` e este documento modificados. `main`, `APLICATIVO`, backups, Wi-Fi/TCP e operações físicas permanecem intactos. PR #13 em rascunho.

## 01/11/2026 — Correção do gate Qt e distribuição de objetos offline (PR #13)

- Evidência real de CI Windows do commit `c2091e8`: compila Qt e passa `--draw-join-19-8-test`, mas o modo `--layout-icons-19-8-test` falhou com `Medir sem selecao alterou documento ou nao instruiu usuario`. Causa no código: `run()` chama `refresh()`, que substitui imediatamente a mensagem de medição no status. Não era falha geométrica nem bloqueio de compilação. Correção: o comando de medição é inteiramente somente leitura, sem `run()`, mantendo o texto visível e impedindo operação durante arraste.
- Implementado `Editor::distributeObjects(ids,horizontal)`: distribui **espaços entre caixas envolventes** de pelo menos três unidades editáveis, mantendo primeira e última fixas; grupos completos movem juntos. Seleção com menos de três grupos independentes, grupo bloqueado/oculto, coordenadas inválidas ou sobreposição com área insuficiente é recusada ANTES de modificar o documento. É uma transação única de Undo/Redo; outros objetos não mudam. Não inclui Bézier nativa nem raster, não modifica a física da mesa.
- Dois novos atalhos compactos (↔ horizontal e ↕ vertical) e menu `Organizar > Distribuir formas`. Não ativam offset, união, subtração, SVG, DXF, energia, G-code ou comunicação física.
- Teste novo `tests/distribution_19_8_standalone.cpp`: espaçamento X/Y, extremos fixos, vizinhos intactos, grupos, camadas bloqueadas, seleção inválida, 3 ciclos Undo/Redo. Executado offline em Linux com `g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic`: **PASS**. Incorporado ao CMake, CI portátil e CI Qt Windows. O `--layout-icons-19-8-test` agora verifica os botões e aciona distribuição real, com Undo.
- **PENDENTE:** PASS integral do GitHub Actions Windows para este commit (não declarar antes), teste manual com mouse físico e DPI, validação em cópia real `.alaser`. `main`, `APLICATIVO`, arquivos originais e envio físico permanecem preservados. PR #13 continua experimental.

## 01/11/2026 — Referência LightBurn: seleção contextual, etapa 1
- Pesquisa externa na documentação oficial: seleção normal com clique, Shift adiciona, Ctrl alterna; arraste direita exige inclusão total e esquerda aceita cruzamento; alças para mover/escalar/girar; barra Numeric Edits exibe X/Y/L/A, cadeado proporcional, referência 9 pontos e rotação; propriedades mudam conforme tipo e grupo. Fontes: https://docs.lightburnsoftware.com/2.0/GetStarted/SelectingMovingSizing/ ; https://docs.lightburnsoftware.com/2.1/Reference/NumericEditsToolbar/ ; https://docs.lightburnsoftware.com/latest/Reference/ShapeProperties/ . Referência funcional, sem copiar imagens/identidade.
- Auditoria do Qt 19.8: moldura de seleção e 13 alças, retângulo direita/ esquerda, alinhamentos, distribuição e Undo/Redo já existiam. Clique com Shift alternava/removia objeto, divergente do LightBurn. Corrigido Shift=adiciona e Ctrl=alterna, sem alterar a lógica de corte.
- Barra contextual experimental `selection_context_19_8` abaixo dos atalhos: escondida sem seleção, exibe contagem, caixa envolvente X/Y/L/A mm somente leitura para polilinhas/grupos e Bézier nativa selecionada, mostra Agrupar/Desagrupar/Duplicar/Excluir com ativação dependente do tipo e quantidade. Reutiliza as ações e a confirmação de exclusão existentes; nada executa comandos físicos. O teste Qt `--layout-icons-19-8-test` foi ampliado para testar a visibilidade, medidas, estados dos botões e modificadores de mouse reais.
- Não declarar paridade total: campos X/Y/L/A **editáveis**, cadeado de proporção, grade de referência de 9 pontos, rotação numérica, destaque distinto de grupos e propriedades por subtipo ainda precisam de projeto e regressões; Bézier/raster não terão transformação numérica forçada como polilinha.
- Estado: integração exclusivamente experimental na PR #13; **CI Windows e mouse/DPI em tela real ainda pendentes nesta revisão**; `main`, `APLICATIVO`, backup e laser físico intocados.

## 01/11/2026 — Edição numérica e referência de nove pontos (fase 2)
- Faixa contextual ganhou X/Y/L/A mm em QDoubleSpinBox (duas casas decimais), cadeado de proporção inicialmente ligado e grade real 3×3. A referência selecionada determina a coordenada X/Y mostrada e o pivô fixo da escala. Entrada confirmada gera operação atômica reversível.
- Reutilizados `Editor::moveObjects` e `Editor::scaleObjects`, que expandem membros de grupos. Camadas bloqueadas/ocultas, seleção vazia, texto com metadados editáveis, imagens raster, curvas Bézier nativas e dimensões degeneradas não são transformados numericamente; valores inválidos são recusados sem modificar o original. Nenhuma mudança no formato `.alaser` ou no código GRBL.
- A nova regressão Qt `--layout-icons-19-8-test` confere escala proporcional e Undo/Redo, referência inferior direita, X/Y, escala sem trava, valor repetido sem histórico, desativação sem seleção. **CI Windows e teste com mouse real ainda exigidos para este commit.**
- Apenas branch experimental e PR #13 rascunho. `main`, versão estável, backups, comunicadores e emissão física intocados.
 
## 01/11/2026 — Propriedades contextuais de texto e imagem
- Texto vetorial com metadados mostra `Texto editável selecionado`, mantém X/Y/L/A somente leitura e apresenta o botão `Editar texto` que reutiliza o editor original de conteúdo/fonte, evitando alterações sem atualização de metadados; duplicação como polilinha desabilitada nesse tipo.
- Imagem raster selecionada mostra posição e dimensões de sua caixa em modo somente leitura, sem habilitar agrupar ou transformar como vetor. A seleção de imagem notifica a interface mesmo em clique sem movimento; continua sem comandos físicos. Curvas Bézier permanecem somente leitura no painel numérico.
- A regressão Qt `--layout-icons-19-8-test` agora verifica estados de texto editável e imagem raster e impedimento de controles incompatíveis. CI Windows pós-commit e validação manual ainda necessários.

## 01/11/2026 — Reorganização compacta da seleção (feedback visual do Windows)
- Captura do operador: a faixa X/Y/L/A em duas linhas ocupava largura e altura excessivas e deslocava a mesa quando a seleção mudava. Agora propriedades são um **inspetor lateral direito**, aba `Seleção`, com abertura manual pelo botão `Propriedades`. A seleção comum não abre o painel, não muda aba e não altera a altura do desenho. A aba Camadas permanece selecionada por padrão.
- Layout compacto: resumo curto, ações Agrupar/Desagrupar/Duplicar/Excluir em duas colunas, campos X/Y/L/A organizados 2×2 e proporção/grade 3×3 abaixo; usa scroll vertical na lateral. Sem seleção, painel manual permanece aberto com instrução e ações desativadas.
- Ícones secundários repetidos, e os de funções incompletas (SVG/DXF/Offset/booleanos), foram ocultados na barra lateral (mantidos internamente apenas para callbacks e regressões). Alinhar/distribuir permanecem no menu Organizar; medir seleção ganhou ação no menu Organizar; importação LightBurn continua em Arquivo/Abrir.
- Suíte Qt `--layout-icons-19-8-test` ampliada para garantir que selecionar não reabra painel, não altere altura do canvas, botão manual funcione, limpeza da seleção não feche a aba escolhida e atalhos redundantes não estejam visíveis. **Pendente CI Qt Windows desta revisão e validação visual/mouse/DPI em computador real.** `main`, máquinas, backup e G-code intocados.

## 01/11/2026 — Refinamento de usabilidade da aba Seleção (19.8)
- Feedback de captura Windows: a disposição lateral foi aprovada, mas campos X/Y/L/A continuam mostrando números antigos com 0 objetos selecionados, botões pequenos e abertura de Propriedades pouco clara. Corrigido: na seleção vazia, ocultar as áreas de ações/edição, limpar quatro campos com `QSignalBlocker` e mostrar instrução inequívoca; nunca modifica geometria. Re-selecionar restaura os campos com os dados reais.
- Botão `Propriedades` agora **alterna** entre as abas Seleção e Camadas e mantém o estado marcado sincronizado mesmo na troca manual de abas. Sem mudanças na posição/altura do canvas.
- Melhorias visuais em temas claro/escuro: cabeçalho `POSIÇÃO E TAMANHO · mm`, QDoubleSpinBox com contraste e foco visível, sem sufixos de unidade em cada campo (reduzia área útil), 2 colunas compactas, ações com maior legibilidade.
- Teste Qt `--layout-icons-19-8-test` atualizado para alternador, canvas estável, ausência de números residuais e retorno das propriedades na próxima seleção. PR #13 rascunho, `main` e máquina física preservadas. Testes Windows deste commit ainda necessários antes de homologar.
- Ajuste fino da regressão Qt: verificar `isHidden()` nas áreas internas em vez de `isVisible()`, que também depende da aba pai estar atualmente selecionada. Assim a suíte confirma ocultação explícita sem exigir que a aba Seleção esteja aberta.

## 01/11/2026 — MÓDULO ROTATIVO POR ROLOS — ETAPA FUTURA
- O usuário perguntou se configurações e suporte ao rolo rotativo estavam no cronograma. Auditoria do documento e da árvore Git em 01/11/2026: não havia especificação de rolo rotativo, nem função implementada/homologada.
- Registrado no planejamento como **módulo subsequente à estabilização e homologação manual da candidata 19.8**, sem inventar prazo ou marcar pronto. Especificação em `DESENVOLVIMENTO/ESPECIFICACOES/PLANEJAMENTO_ROLO_ROTATIVO_19_9.md`.
- Fase R1: perfil de rolo e formulário de configurações **100% offline**, com diâmetro do rolete/objeto, circunferência, mm/rotação quando aplicável, eixo/direção condicionados ao hardware real, valores salvos e modo plano preservado.
- Fase R2: prévia em superfície cilíndrica desenrolada e aviso de ultrapassagem de circunferência, testes matemáticos/persistência e regressões Windows, ainda sem controle físico.
- Fase R3: somente após validação separada, identificar o modelo exato do rolo e sua ligação à AlgoLaser Alpha MK2 20W, calibrar segundo manual e aprovar testes com movimento físico em missão específica. Não enviar comandos, alterar firmware ou emitir laser na 19.8.
- Referência oficial funcional: https://docs.lightburnsoftware.com/2.1/Reference/RotaryMode/RotaryModeGCode/ .
- Registro de cronograma apenas, nenhuma alteração em código do editor, `main`, `APLICATIVO`, dados, arquivos originais ou controladora.
 
## 01/11/2026 — R1 rotativo Ortur offline + movimentos da seleção por página (referências reais)
- Fontes fornecidas: vídeos LightBurn com moldura/13 alças, rotacionar e posicionar formas, menu `Mover seleção`; fotos da tela Rotary Setup com Roller/Chuck, eixo, mm/volta, rolete, objeto, circunferência; fotografia de rotativo Ortur do tipo rolos. **Modelo e valores de calibração NÃO confirmados**. Fotos e vídeos pessoais NÃO publicados no GitHub.
- Criados `src/rotary_geometry_19_9.hpp` e `tests/rotary_offline_19_9_standalone.cpp`: fórmula de circunferência, graus percorridos e giro do rolete, validação de valores finitos e de perfil incompleto. Valores desconhecidos são zero/inativos, sem inventar eixo nem coeficiente.
- Criado `qt/rotary_dialog_19_9.hpp`: acessível pelo menu `Laser > Configurar rotativo — OFFLINE`, com Rolo/Mandril, eixo candidato Y/Z/A, mm/rotação, diâmetro do rolete, diâmetro do objeto e circunferência sincronizada; espelho e perfil somente para a prévia offline. QSettings separado do projeto e testes de perfil salvo/reaberto. `Testar rotação` explicitamente desativado, sem USB/TCP, G-code, movimento ou laser.
- Editor Qt: menu `Organizar > Mover seleção para...` implementado em nove posições da ÁREA DE DESENHO (centro, cantos, centros das bordas) com geometria atômica/Undo e isolamento de objetos não selecionados; grupo expandido e seleção bloqueada seguem regras do motor. Alças de rotação desenhadas como arcos, sem alterar 13 zonas de clique ou os algoritmos.
- `CMakeLists.txt` e CI portável/Windows integram o teste portátil. `--layout-icons-19-8-test` ampliado para perfil Ortur offline, eixo/diâmetro/circunferência, persistência, bloqueio de botão motor e posicionamento da forma com Undo.
- **ESTADO:** código integrado apenas à branch experimental PR #13; depende do resultado final das execuções Windows deste commit e de teste real com mouse/arquivos de cópia. Não é homologação física, nem versão 19.8 definitiva. Main/versão estável/backups/firmware/equipamento intocados.
 
## 01/11/2026 — Revisão de seleção orientada e rodapé fixo (vídeo real do operador)
- Feedback com vídeo 1280×720: seleção com contorno laranja mudava de tamanho quando o objeto girava, pois `selectionDisplayBounds()` empregava `QTransform::mapRect()` e redesenhava uma caixa **alinhada à mesa**. O usuário também considerou a aparência de alças/ícones pouco acabada e solicitou trazer posição/tamanho/proporção para a faixa inferior junto a Estimado/Decorrido.
- Novo quadro de seleção usa quatro vértices orientados, ligados à identidade da seleção e à `geometryGeneration` para invalidar molduras após Undo/Redo, alterações ou troca de seleção. Durante giro/deslocamento/escala, o quadro, oito alças de escala, quatro de giro e centro são transformados juntos; dimensionamento por mouse passa a usar os eixos locais do objeto e transação `Editor::affineObjects` (Undo/Redo único, grupos preservados). A leitura X/Y/L/A também usa geometria local, sem crescimento visual artificial quando girado. A geometria `.alaser` não ganhou novo formato; orientação da moldura é estado **editorial transitório**.
- X/Y/L/A, cadeado de proporção, referência de nove pontos e menu `Posicionar` foram reagrupados em linha de rodapé após os indicadores operacionais. O inspetor direito continua disponível para ações específicas, sem duplicar campos ou abrir sozinho. O bloqueio do laser não muda.
- Ícones de ferramentas Qt renderizados em pixmap de resolução lógica 2x para nitidez; linhas de formas exibidas com pena cosmética, anti-alias e junções arredondadas. Não há alteração na geometria canônica nem nos modos Linha/Fill de fabricação; a edição padrão segue visualização de contorno.
- Regressão `--inline-transforms-19-8-test` ampliada para exigir quadro com larguras/alturas locais invariantes após rotação e restauração correta ao desfazer. O teste do rodapé verifica campos e botão de posicionamento.
- Os testes C++ portáteis e PowerShell passaram nas execuções consultadas, mas **a revisão Qt Windows está pendente**. Ocorreram erros transitórios de download MSYS2 nos runners e foi detectado/corrigido um conflito de nome de variável local (`names`), aguardando recompilação. **Não declarar homologação** até PASS Qt, inspeção do operador e testes com projeto de cópia em diferentes DPI.
- PR #13 permanece experimental; `main`, máquina/laser, emissão, G-code, bases antigas, restauração e arquivos pessoais não alterados.
 
## 01/11/2026 — Gerador de engrenagens vetoriais em mm (Inkscape como referência funcional)
- Usuário enviou captura Inkscape: `Extensões > Gerar > Engrenagem` e `Barra Dentada`; pediu implementação útil no Alencar Laser, especialmente para corte de MDF/mecanismos. A base gráfica e o código foram **desenvolvidos independentemente**, sem copiar a extensão GPL do Inkscape.
- Criados `src/gear_generator_19_9.hpp` e `tests/gear_generator_19_9_standalone.cpp`: engrenagem **externa** de dentes evolventes poligonais aproximados, com dentes 12–120, módulo em mm, ângulo de pressão 14–30°, furo central opcional. Cálculo de raio primitivo (m·z/2), base, externo e raiz, perfil fechado e furo com orientação inversa. Valida números finitos, núcleo mínimo e limite de 10 mil nós.
- Criado `qt/gear_dialog_19_9.hpp`: prévia visual offline com círculo primitivo, parâmetros, diâmetro calculado, posição do centro X/Y e alerta quando excede área configurada. Menu `Gerar > Engrenagem paramétrica (mm)...` em `qt/qt6_test6.cpp`.
- Geração chama `Editor::addContours(...,true,layerId,true)`, inserindo roda e furo em **uma transação Undo** e grupo. Preserva arte existente, camada ativa, sistema v5 e capacidade de selecionar/editar os contornos. O furo é contorno separado de corte. Cancelar diálogo não insere nada.
- Gates: `alencar_test19_9_gear_offline` nos workflows Linux/Windows e script local; modo Qt `--gear-generator-19-9-test` testa campos, prévia, menu, geração de dois contornos, preservação de arte preexistente e Undo.
- **Limitações até homologação:** involuta amostrada em segmentos retos (não curva matemática contínua), não inclui compensação de kerf/folga, adoçamento/fillet dos dentes, correção de undercut nem cálculo real de encaixe; não é certificado para engrenagens mecânicas sob carga ou tolerância de precisão. Cremalheira/engrenagem interna e engrenagens conjugadas permanecem pendentes.
- Status: **experimental** na PR #13. Testes finais Qt Windows e mouse/DPI com MDF 3 mm pendentes. `main`, hardware, G-code, emissão, bases homologadas e backups intocados.

## 01/11/2026 — Validação focada de engrenagens, pares, cremalheira e regressão Qt geral
- Captura VS Code do operador em `C:\ProjetoAlencarLaser`: script `-SomenteEngrenagens` compilou `alencar_test7_qt6.exe` e exibiu `GEAR_OFFLINE_19_9 PASS` e `ENGRENAGENS PASS nos testes sintéticos`; retorno ao prompt sem erro. Os warnings `QDragEnterEvent` obsoleto e `range-loop-construct` não impediram essa rodada.
- Código de módulo/passo circular sincronizados (`p = π m`), prévia opcional e referência Inkscape (19 dentes, 10 mm, 30°, furo 10 mm) implementado; o teste focado do operador confirma somente **ensaio sintético**, não experiência com mouse/arte real.
- Par externo nominal e cremalheira trapezoidal aproximada agora foram integrados ao menu `Gerar > Engrenagens`, com diálogos/prévias independentes, vetores editáveis e Undo transacional. Ainda não há cálculo confiável de fase angular, interferência, folga, undercut, kerf nem resistência mecânica.
- Foi examinada a execução do GitHub Actions Windows Qt run #201 (commit `70b2781`): build, regressão Bézier e teste portátil passaram, mas o **gate geral falhou em `layout-icons-19-8`**, com JSON `Painel nao foi mantido em selecao vazia`. Causa localizada: teste exigia texto antigo `Selecione`, enquanto a implementação atual mostra `Nenhuma forma selecionada...`.
- Correção pontual do teste na branch experimental, commit `04ea03b`: valida o texto efetivo, a visibilidade do painel e os comandos Agrupar/Desagrupar desativados. CI Qt Windows integral desse hotfix **ainda requer PASS conferido**, além do teste manual local da interface.
- Branch: `fix/curvas-tesoura-preflight-19-8-20261030`, PR #13 permanece **draft**. `main`, `APLICATIVO`, backups, GRBL, G-code, movimento e emissão física permanecem inalterados/bloqueados.
- Teste manual seguinte (somente com equipamento desconectado): conferir as três opções do menu, visualização de raios/dentes, deslocamento de objetos, Ctrl+Z, save/reopen `.alaser` em cópia, DPI e painel Seleção. Não homologar produção sem essas evidências.
