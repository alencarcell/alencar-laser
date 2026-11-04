# Central de Auditoria — Alencar Laser 19.9 (experimental)

**Entrada de continuidade entre chats:** este README, `INDICE.json`, [Issue #20](https://github.com/alencarcell/alencar-laser/issues/20) (central), [Issue #18](https://github.com/alencarcell/alencar-laser/issues/18) (performance) e [PR #13](https://github.com/alencarcell/alencar-laser/pull/13).

## Objetivo

Manter rastreabilidade de testes reais do Windows sem depender de capturas de tela. A pasta local `RELATORIOS/` continua **ignorada pelo Git** e abriga os JSON completos. Somente os registros **filtrados** em `AUDITORIA_GITHUB/REGISTROS/` e o `INDICE.json` podem ser publicados.

## Como usar no Windows PowerShell 5.1

Com o projeto em `C:\ProjetoAlencarLaser`, na branch `fix/curvas-tesoura-preflight-19-8-20261030`:

```powershell
git status --short
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
# Passo 0: autoteste de seguranca totalmente offline (sem dados reais):
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_CENTRAL_AUDITORIA_19_9.ps1"
# Passo 1: prepara as evidencias filtradas localmente, sem rede:
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\REGISTRAR_AUDITORIA_GITHUB_19_9.ps1"
# Passo 2: inspecione AUDITORIA_GITHUB\INDICE.json e REGISTROS.
# Passo 3: somente apos inspecao, publicar na branch experimental:
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\REGISTRAR_AUDITORIA_GITHUB_19_9.ps1" -Publicar
```

A validação opcional em GitHub Actions está em `.github/workflows/central-auditoria-19-9.yml`; verificar os resultados antes de considerar o coletor validado.

O script coleta **o ultimo** JSON `AUDITORIA_CACHE_ZOOM_19_9_*.json` e o ultimo `COMPARACAO_CACHE_19_9_*.json`. Opcoes: `-Todos` importa historico local reconhecido, `-ArquivoJson "caminho.json"` seleciona um arquivo especifico. O modo normal **nao faz commit nem push**. O modo `-Publicar` requer acao explicita, verifica branch e remote, recusa staging anterior e inclui somente registros filtrados e o indice. Nunca usa `git add .`, `--force` nem grava na `main`.

## Campos de cada registro

- `schema`, `event_id`, `recorded_utc`, `type`, `status`, `version`;
- SHA-256 **do proprio relatorio bruto** para verificar correspondencia local, mais **prefixo de 12 caracteres** do hash de origem (sem nome/caminho do projeto);
- metrics numericas permitidas: contornos, P95 com/sem cache, percentual, memoria estimada, zoom visivel, quantidade de itens em cache, pixels divergentes acima de 8 e quantidade de checks falhos;
- estado de homologacao sempre **falso**, ate processo separado e validacao manual.

**Nao exportar:** JSON original, caminho de usuario, CPF, contato, nome de clientes, imagens, .lbrn2, .alaser, logs de comunicacao, fotos, scripts .env, tokens ou segredos. Nao se permite copiar campos de texto livres do relatorio. O codigo admite apenas dois esquemas tecnicos conhecidos; outro JSON e rejeitado.

## Interpretacao correta

`PASS_AUTOMATED_MANUAL_PENDING` significa **sucesso do gate automatizado** sobre o JSON local existente, e **nao** homologacao do aplicativo, da precisao de corte nem de conexao fisica. Sempre citar teste de Windows (evidencia automatica), contexto e versao da branch. Toda seguranca de hardware continua bloqueada. Cache permanece **desligado por padrao** no editor.

## Seguranca e limites

- Primeiro upload de registros depende de execucao local; **a criacao desta infraestrutura no GitHub nao importa retroativamente os JSON do computador**.
- A hora `recorded_utc` deriva da modificacao do arquivo local, nao e carimbo criptografico do teste.
- O hash SHA-256 prova comparacao com o arquivo local, mas nao atesta sozinho autenticidade do evento.
- Para rastrear origem exata do experimento, consulte os JSON completos no computador e a Issue #18.
- O envio usa `git push` com a autenticacao Git existente; falhas de rede deixam o commit local preservado para reconciliar manualmente.
- O projeto tem outras ferramentas experimentais e nao deve ter sua branch `main`, instalador ou dispositivos fisicos alterados por esta central.

## Proximos testes

Mouse real, selecao e deselecao, zoom/pan continuo, Tesoura com curvas, desfazer/refazer, RSS e pelo menos mais duas rodadas A/B com origem identica. Se houver diferencas visuais, registrar FAIL/INCONCLUSIVE em vez de chamar o cache de homologado.
