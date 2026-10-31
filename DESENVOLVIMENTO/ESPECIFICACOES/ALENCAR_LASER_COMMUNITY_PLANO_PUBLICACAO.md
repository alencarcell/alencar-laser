# ALENCAR LASER COMMUNITY — DIRETRIZ DE PUBLICAÇÃO (APROVADA EM 30/10/2026)

> **Documento de planejamento em repositório PRIVADO.**
> A aprovação da estratégia **não** autoriza publicar arquivos, alterar visibilidade,
> lançar instaladores ou ativar movimentos/emissão GRBL antes de auditoria e homologação.

## 1. Direção do produto

- **Nome do aplicativo:** Alencar Laser.
- **Visão:** editor e simulador gratuito para Windows, evoluindo para cliente compatível com controladores GRBL sob testes e revisão.
- **Modelo proposto:** software livre e código aberto; colaboração comunitária; apoio financeiro **voluntário**, sem bloquear o uso por pagamento.
- **Identidade:** manter a marca, a logomarca oficial e a autoria Alencar Laser no software e no site, com política de marca separada da licença de código.
- **Governança:** mudanças por PR, issues, changelog, tags, testes e critérios de segurança; versão estável só depois de homologação.
- **Não confundir o app com o firmware GRBL:** o software é um cliente independente compatível; referenciar e atribuir o firmware sem copiar nem reempacotar código/arquivos sem auditoria de licença.

## 2. Decisões atuais versus decisões pendentes

| Tema | Direção aprovada | Pendência antes de publicar |
|---|---|---|
| Gratuidade | Download sem cobrança ou ativação | Preparar binários reproduzíveis e assinaturas/hash |
| Código aberto | Receber contribuições públicas | Auditoria da origem de TODO o código, imagens, ícones e bibliotecas |
| Licença | **Candidata: GPL-3.0-or-later**, adequada à intenção de copyleft | Verificar compatibilidades das dependências, arquivos derivados e obrigações; formalizar texto e avisos |
| Marca | Nome/logotipo Alencar Laser mantidos | Regras claras para forks e distribuição da marca; avaliar proteção da marca |
| Repositório | Futuro repositório **público limpo**, com histórico sanitizado | Nunca tornar o repositório interno atual público automaticamente |
| Apoio voluntário | Botão “Apoie o desenvolvimento” no site | Destino de Pix comercial validado pelo proprietário e/ou conta Sponsors confirmada |
| Site | Página Alencar Laser no site oficial | Revisar cópias, acessibilidade, privacidade, links e instalador |
| Controle físico | Futuro módulo compatível com GRBL | Segurança, limites, firmware, alarmes, homing, frame, botão de emergência e máquina real testados |

**Importante sobre GPL:** o código pode ser redistribuído e até vendido por terceiros nos termos da licença. A marca comercial pode ser tratada separadamente; não usar a licença como promessa de exclusividade. Escolha final requer auditoria, não somente colocar um arquivo LICENSE.

## 3. Plano em fases e critérios de saída

### Fase A — Auditoria interna (sem público)
- [ ] Identificar todos os arquivos rastreados e dependências, suas licenças e direitos de redistribuição.
- [ ] Inspecionar *histórico completo* do Git antes de qualquer divulgação: segredos, tokens, certificados, chaves, caminhos pessoais, dados de clientes, logs e artefatos.
- [ ] Identificar recursos próprios versus externos: logomarca, ícones, fontes, fotografias, referências de interface de terceiros e firmware.
- [ ] Revisar árvore de origem do código por autoria/origem e necessidade de créditos.
- [ ] Decidir licença final e preparar avisos de copyright e dependências.
- [ ] Definir artefatos que não serão publicados: `DADOS`, `BACKUP`, `RELATORIOS` de clientes, `APLICATIVO` aprovado interno, configs locais e projetos `.alaser` particulares.

### Fase B — Community (offline, sem GRBL operacional)
- [ ] Corrigir Tesoura Bézier: corte, Undo/Redo, renderização e captura de telemetria com mouse físico.
- [ ] Aprovar layout, alinhamentos, seleção, ferramentas, gerenciamento de camadas, raster/Fill, intervalo entre linhas e importação.
- [ ] Abrir/reabrir cópias de projetos `.alaser` v5 sem perda; implementar backup/rollback e testes.
- [ ] Passar suíte automatizada C++/Qt no Windows, CI e inspeção gráfica em várias escalas/DPI.
- [ ] Conferir instalador isolado, hashes SHA-256, `CHANGELOG.md` e `RELEASE_NOTES.md`.
- [ ] Marcar versão candidata **Community 1.0-beta** (sem chamar estável antecipadamente).

### Fase C — Estrutura pública de colaboração
- [ ] Criar **novo repositório público com snapshot limpo** após auditoria e revisão humana (não expor histórico privado).
- [ ] Incluir `README.md` com capturas, instalação, limites e link oficial.
- [ ] Incluir `LICENSE` definido após auditoria; `THIRD_PARTY_NOTICES`/SBOM conforme dependências.
- [ ] Adicionar `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md`, templates para issue/PR, política de versões e de marca.
- [ ] Proteger branch principal, exigir revisão e CI para merges; publicar artefatos exclusivamente em Releases.
- [ ] Publicar código e instalador de beta **offline**, identificando o controle físico como indisponível.

### Fase D — Divulgação e apoio
- [ ] Criar página `/alencar-laser` no site oficial Alencar Cell (rota exata a confirmar).
- [ ] Exibir: logo oficial, finalidade, screenshots, requisitos, download, changelog, aviso de segurança, GitHub, documentação e comunidade.
- [ ] Incluir botão **Apoie o desenvolvimento**; link de pagamento definido apenas com autorização e dados comerciais corretos.
- [ ] Explicar que apoio é opcional e não compra tratamento prioritário nem funcionalidade.
- [ ] Manter downloads versionados e hash SHA-256 visível, com aviso de beta e procedência.
- [ ] Evitar alegação de certificação, aprovação de fabricantes ou equivalência com LightBurn.

### Fase E — GRBL operacional (trilha separada)
- [ ] Definir máquinas/firmwares aceitos e parâmetros e testar inicialmente **somente leitura**.
- [ ] Avaliar comandos (M3/M4, feed, potência, pausa/cancelamento, alarmes, homing, intertravamentos e recuperação) com segurança por padrão.
- [ ] Testar clipping da mesa, origem, sobrecursos, pré-visualização versus G-code, desconexão e retomada com máquina isolada e operador.
- [ ] Criar bloqueios, confirmação explícita, simulação e trilha de auditoria; **não liberar disparo por uma PR de interface**.
- [ ] Só etiquetar suporte GRBL “operacional” após protocolo de aceitação específico, com riscos conhecidos.

## 4. Fluxo de contribuições desejado

1. Usuário abre issue (bug, melhoria, compatibilidade ou documentação).
2. Mantenedor faz triagem, confirma reprodução, prioridade e escopo.
3. Contribuidor cria fork/branch e PR com teste e descrição, sem projetos privados.
4. CI + revisão + inspeção de segurança antes do merge.
5. Release notes, versão e link no site quando houver versão homologada.

## 5. Padrão de segurança e privacidade

- O aplicativo **nunca** deve coletar ou transmitir dados sem consentimento explícito.
- Relatórios para issues devem omitir dados pessoais, projetos de clientes e credenciais.
- O código aberto não implica controle físico automaticamente seguro; avisos e proteções são requisitos.
- Não distribuir firmware GRBL ou materiais de terceiros em pacote sem licença e atribuições verificadas.

## 6. Estado em 30/10/2026

- Repositório atual `alencarcell/alencar-laser`: **privado**, branch principal `main`.
- Projeto Qt6/C++ 19.8 ainda experimental; PRs técnicas independentes e validação manual pendente.
- Nesta decisão apenas **planejamento privado** foi autorizado e preparado.
- **Proibido nesta etapa:** alterar visibilidade do repositório, liberar binário público, mexer em `main`, modificar `APLICATIVO` ou habilitar operações físicas.
