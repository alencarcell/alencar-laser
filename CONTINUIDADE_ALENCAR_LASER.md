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
