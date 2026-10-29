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
