# ALENCAR LASER — CONTINUIDADE E TRANSFERÊNCIA PARA O GROK
**Registro:** 16/11/2026  
**Repositório:** `alencarcell/alencar-laser`  
**Branch de referência mais recente identificada no GitHub:** `feat/memoria-pastas-exportacao-svg-20zc-20261111` (PR #32, draft)  
**Branch experimental histórica:** `fix/curvas-tesoura-preflight-19-8-20261030`

> Este documento é um mapa de continuidade, não um certificado de homologação. Diferencie sempre evidência automatizada, validação manual e hipótese.

## 1. Missão para o Grok

Assumir a continuidade do projeto existente e conduzir uma auditoria consolidada para chegar a uma versão estável. Não reiniciar o projeto, não reescrever módulos aprovados sem necessidade e não iniciar uma rodada ampla de novas funcionalidades.

Primeira entrega esperada: inventário do estado real do repositório, arquitetura, funcionalidades, testes, branches/PRs, riscos e pendências priorizadas. Só depois propor correções mínimas e verificáveis.

## 2. Estrutura e ambiente conhecidos

- Projeto local do operador: `C:\ProjetoAlencarLaser`
- Código-fonte Qt: `DESENVOLVIMENTO\FONTE`
- Arquivo Qt central historicamente utilizado: `DESENVOLVIMENTO\FONTE\qt\qt6_test6.cpp`
- Tecnologias: C++, Qt 6, CMake/Ninja e scripts PowerShell para build/testes Windows.
- Branch histórica de desenvolvimento: `fix/curvas-tesoura-preflight-19-8-20261030`.
- O GitHub também contém PRs mais recentes da série 20z; verificar o HEAD e a branch exatos antes de executar qualquer comando.

## 3. Evidência de build/testes recebida em 16/11/2026

O log compartilhado pelo operador informa:
- build concluído;
- regressões automatizadas concluídas com PASS;
- testes citados: exclusão de âncora Bézier; observador GRBL read-only; regras de zoom/Wi-Fi; tesoura por camadas; planejamento de intervalos; corte Bézier nativo; política de recuperação; rotativo offline; engrenagens; mecanismos; histórico/memória; importação LBRN2 segura e nativa; codec .alaser v5;
- editor aberto em modo de desenvolvimento;
- mensagem final: “BUILD E REGRESSOES AUTOMATIZADAS PASSARAM. AVALIACAO VISUAL AINDA NECESSARIA.”

Avisos registrados no mesmo log:
- `QFont::setPointSize: Point size <= 0 (-1)`;
- `Mouse event "MouseRelease" not accepted by receiving widget`;
- avisos C++ `-Wmisleading-indentation` em `src/bezier_interaction.hpp`.

**Limite da evidência:** o trecho de log compartilhado não registra o SHA do commit nem o nome da branch que foi compilada. Portanto, não afirmar que o HEAD atual do PR #32 foi compilado até confirmar o SHA local/CI correspondente.

## 4. Tesoura K e eventos de mouse — pendência de validação

O código Qt inspecionado no histórico mostra o fluxo de clique usando o `ScissorSpan` armazenado no hover, limpando a prévia, chamando `editor.removeScissorSpan(cutting, joinAllowed)`, reconstruindo e registrando `scissor_continuous_span_removed`.

O log automatizado confirma os testes de corte Bézier nativo e planejamento de intervalos. Entretanto, a evidência compartilhada não comprova, por si só, que o movimento natural do mouse físico foi recebido sem fallback sintético.

A auditoria deve:
1. localizar no código e nos testes a rota natural e qualquer fallback de `QMouseEvent`/ `QApplication::sendEvent`;
2. fazer o teste reportar explicitamente qual rota entregou o hover;
3. manter testes sintéticos como testes de processamento, sem apresentá-los como validação física;
4. validar manualmente no Windows, com laser fisicamente desconectado: prévia ciano, desaparecimento ao afastar, corte, Ctrl+Z e Ctrl+Y;
5. registrar o resultado manual separadamente.

Não remover o fallback automaticamente: primeiro entender sua finalidade e preservar cobertura de teste.

## 5. PRs e issues relevantes identificados no GitHub

- PR #32 — 20zc/20zd: exportação SVG aprovada no LightBurn; correção da memória de pastas. A descrição informa que a memória de pastas ainda não foi homologada após fechar/reabrir e usar Salvar como. Draft.
- PR #31 — 20zb: repinturas locais e diagnóstico de desempenho Qt. A descrição informa que a revisão ainda não tinha compilação Qt/Windows.
- PR #30 — 20za: nós visíveis e destaque dos extremos na ferramenta J. A descrição informa que ainda não foi compilado no Windows.
- PR #29 — 20z: remover aresta entre nós e painel Objetos contextual. A descrição informa que ainda não foi compilado/testado no Windows.
- PRs #28 a #23: evolução experimental da Tesoura, incluindo cortes sucessivos, prévia ciano, consistência hover/clique, união conservadora e classificação de recusas. Verificar cada PR e sua situação atual antes de concluir que está incorporado ou validado.
- Issue #18: desempenho da GUI Bézier e metadados de camada incompletos. Contém evidências e pendências específicas; não declarar que todas foram resolvidas.
- Issue #19: suporte fiel à hierarquia de grupos aninhados no importador LBRN2. O comportamento descrito é recusar grupos não suportados para evitar flatten/perda de hierarquia; suporte completo ainda não estava implementado no registro consultado.
- Issue #20: central de auditoria filtrada e continuidade GitHub. Contém salvaguardas de privacidade e fluxo de publicação; conferir o estado atual antes de reutilizar scripts.

Links:
- Repositório: https://github.com/alencarcell/alencar-laser
- PR #32: https://github.com/alencarcell/alencar-laser/pull/32
- PR #31: https://github.com/alencarcell/alencar-laser/pull/31
- PR #30: https://github.com/alencarcell/alencar-laser/pull/30
- Issue #18: https://github.com/alencarcell/alencar-laser/issues/18
- Issue #19: https://github.com/alencarcell/alencar-laser/issues/19
- Issue #20: https://github.com/alencarcell/alencar-laser/issues/20

## 6. Regras de segurança e preservação

- Antes de qualquer operação local: conferir `git status --short`, branch atual e HEAD.
- Não sobrescrever alterações locais nem executar reset/clean/force push.
- Não mesclar PRs nem promover código para `main`, `APLICATIVO` ou instalador sem autorização explícita.
- Não alterar arquivos de clientes ou originais de projeto durante testes; usar cópias.
- Não conectar nem emitir comandos para o laser durante auditoria de software.
- Não habilitar GRBL, G-code ou fabricação como consequência desta transferência.
- Não reduzir precisão Bézier, âncoras, alças ou qualidade de zoom para aparentar ganho de desempenho.
- Não converter silenciosamente grupos LBRN2 em curvas independentes.
- Manter parâmetros de fabricação não verificados explicitamente marcados como não verificados.
- Cada correção deve ser pequena, rastreável e acompanhada de teste de regressão.

## 7. Plano acelerado de continuidade

**Fase A — Reconhecimento (sem modificar código)**
1. Confirmar branch, HEAD, PRs abertos, árvore do repositório e estado do CI.
2. Ler README, documentação de arquitetura, especificações, changelog e issues relacionadas.
3. Mapear funcionalidades implementadas para arquivos, testes e status de validação.
4. Identificar divergências entre a branch local testada em 16/11 e o HEAD atual do GitHub.

**Fase B — Fechamento técnico**
1. Resolver apenas bloqueios comprovados para estabilidade.
2. Separar warnings cosméticos de erros funcionais.
3. Validar os fluxos Qt de mouse e Tesoura K.
4. Reexecutar os testes na branch/commit que será candidato à entrega.
5. Fazer homologação visual/manual com o laser desconectado.
6. Documentar recursos que permanecem incompletos (por exemplo, grupos LBRN2 e metadados de camada) sem escondê-los.

**Fase C — Entrega**
1. Apresentar matriz PASS / FAIL / NOT TESTED por funcionalidade.
2. Identificar exatamente o commit candidato.
3. Preparar changelog e instruções de build/instalação.
4. Não publicar nem substituir a versão de produção até autorização do operador.

## 8. Formato da primeira resposta do Grok

Entregar uma tabela com:
- módulo/funcionalidade;
- arquivo(s) principal(is);
- teste(s) relacionado(s);
- branch/PR;
- evidência mais recente;
- status: PASS automatizado / PASS manual / FAIL / NOT TESTED;
- risco;
- ação mínima recomendada.

Ao final, indicar os cinco bloqueios mais importantes para uma versão estável. Não fazer alterações de código antes de apresentar esse diagnóstico.

---
**Nota de atualização:** este documento registra o estado conhecido até 16/11/2026 com base nos logs e registros do GitHub consultados. O Grok deve conferir o estado remoto atual e atualizar este documento à medida que novas evidências forem produzidas.
