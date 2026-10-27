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
