# Alencar Laser — Release Readiness 20f

**Data:** 06/11/2026
**Status:** runner escrito e publicado em branch isolada; **NÃO executado ainda no Windows**.
**Branch:** `test/release-readiness-20f-20261106` (derivada de `exp/phase20d-item-cache-20261106`).
**Escopo:** editor **offline** C++/Qt 6, sem comunicação com o laser.

## Por que esta rodada aproxima a conclusão
Substitui a sequência fragmentada de correções pontuais por um gate de regressão consolidado. O runner usa scripts existentes já inspecionados; não inventa novos comandos físicos e não ativa cache Qt recusado na Fase 20e.

| Gate | Cobertura | Resultado exigido |
|---|---|---|
| G01 | Suíte integral C++ e Qt: seleção, desenho, Tesoura, Bézier, texto, camadas, simulação, importação, codec v5, recuperação, histórico | exit 0 |
| G02 | Importação nativa Bézier GUI, metadados de camada, reabertura de .alaser v5 | exit 0 |
| G03 | Importação LightBurn com rejeição segura | exit 0 |
| G04 | Tesoura C++ e hover Qt, recorte, Undo/Redo | exit 0 |
| G05 | Arraste agrupado e histórico | exit 0 |
| G06 | Política repaint OFF/ON com 40 contornos (sem prometer ganho) | exit 0 |
| G07 (opcional) | 575 contornos, 6 rodadas OFF/ON3, 144 movimentos; política equivalente, sem alegar speedup | exit 0 e gate do script denso |

O script `SCRIPTS/VALIDAR_PRE_LANCAMENTO_20F_WINDOWS.ps1` invoca cada gate em um PowerShell filho, captura stdout/stderr e encerra no **primeiro erro**, deixando `RESULTADO_RELEASE_READINESS_20F.json` em pasta local exclusiva da sessão. `-Rapido` executa G01 somente; sem `-Rapido` executa G01–G06; `-IncluirPerformance` acrescenta G07; `-ArquivoLbrn2` passa um arquivo real à suíte de integração (opcional, pode conter dados do usuário, fica local).

### Comando completo de validação

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch test/release-readiness-20f-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha ao mudar de branch; nao use -f/reset/clean' }
git pull --ff-only origin test/release-readiness-20f-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\VALIDAR_PRE_LANCAMENTO_20F_WINDOWS.ps1' -IncluirPerformance
```

Recomenda-se fechar os executáveis de desenvolvimento e manter o laser fisicamente desconectado. Não usa `-Abrir`, nem cria instalador, nem faz commit/push. Os relatórios existentes dos testes podem ser atualizados pelos próprios scripts legados na pasta RELATORIOS: as saídas do runner ficam em sessão isolada.

## Testes manuais obrigatórios depois do PASS automático

1. Abrir candidata em cópia de teste, com o laser desconectado; avaliar mouse físico, cursor da Tesoura, prévia de trecho, clique, desfazer/refazer, arrastar 40/575 contornos e zoom/pan sem rastros.
2. Reabrir arquivos `.alaser` v5 originais com cópias de segurança e comparar contornos, camadas, imagens, texto e posição após salvar em outro nome.
3. Importar arquivos `.lbrn2` reais cobrindo linhas, curvas Bézier, grupos, elementos rejeitados e entradas inválidas; nunca prometer suporte completo para estruturas não cobertas.
4. Forçar fechamento de sessão de teste e validar recuperação de projeto sem sobrescrita do original.
5. Testar DPI 100%/125%/150%, alternância de monitor, temas e mouse; verificar que ícones e alças preservam posicionamento.
6. Conferir memória e fluidez em trabalhos reais (pelo menos leve, médio e denso), distinguindo desempenho do benchmark sintético.
7. Instalar em diretório de homologação separado, abrir e desinstalar sem tocar nos projetos do usuário; verificar atualização e reversão **antes** de usar `APLICATIVO`.
8. Wi-Fi, GRBL, movimentos físicos, potência, corte e gravação continuam fora da homologação offline. Exigem protocolo de segurança separado.

## Critério de saída

Somente com todos os gates automáticos aprovados, testes manuais documentados e instalador/reversão verificados é adequado declarar uma **candidata offline**. Aprovação física do laser exige procedimento próprio. Nenhuma estimativa de porcentagem global é inferida deste teste.

## Situação do cache 20d/20e

O DeviceCoordinateCache experimental é mantido apenas como histórico investigativo. Em 575 contornos, no cenário pan (40,20), zoom 1,0, foram observadas 818 diferenças de pixel por rodada, inclusive com cache fresco; a mediana global não mostrou aceleração robusta. Não integrar esse cache ao editor principal.
