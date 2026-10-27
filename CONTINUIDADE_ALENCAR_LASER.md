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
