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
