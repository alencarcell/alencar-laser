# Alencar Laser — candidata offline integrada 20n (07/11/2026)

## Objetivo
Consolidar **Ctrl+D múltiplo** e o modo de atualização de viewport **MinimalViewportUpdate** numa candidata utilizável no laboratório, sem modificar o instalado. A aprovação automática não substitui avaliação manual, backup e teste do instalador.

## Base consolidada
Branch `candidate/offline-20n-20261107` derivada de `fix/phase20m-bulk-duplicate-20261107` (inclui o experimento 20l):
- Ctrl+D agora usa `Editor::duplicateObjects()`: duplicação em uma transação, mantém contornos, grupos, camadas, texto editável, novos IDs de nós e Undo/Redo.
- `ALENCAR_MINIMAL_VIEWPORT_20L=1` somente no processo de laboratório: não altera default do editor instalado.
- Cache por item e invalidações experimentais 20h/20k continuam OFF.
- O Qt original e a versão instalada seguem inalterados.

## Evidências já fornecidas pelo operador
- **Fase 20m**: núcleo C++ com 575 objetos e GUI Ctrl+D com 575 objetos: PASS de geometria, histórico, grupos, camadas bloqueadas e texto.
- **Fase 20l sintética**: P95 OFF/ON (ms): 28,58/28,27; 49,66/28,83; 38,19/30,86; hashes dos frames iguais.
- Sessões reais de viewport OFF/ON **não estritamente pareadas**: OFF terminou com 141 contornos, 37.365 nós; ON terminou com 573 contornos, 13.409 nós. Zoom P95 OFF ~172,29 ms e ON ~36,15 ms, mas **não é ganho causal homologado**.
- O operador associou erros de comando repetidos a Ctrl+D com seleção múltipla. O código GUI antigo exigia um único objeto. Correção automatizada já passou, uso manual pendente.

## Novos arquivos da candidata
- `SCRIPTS/HOMOLOGAR_CANDIDATA_OFFLINE_20N_WINDOWS.ps1`: configura e compila `DESENVOLVIMENTO/FONTE/build_WINDOWS_CANDIDATE_20N`, verifica C++ 575 e 13 modos Qt de laboratório. Falha no primeiro gate, não faz merge, push, instalação ou conexão; registra resumo JSON e hash SHA-256 do executável.
- `SCRIPTS/ABRIR_CANDIDATA_OFFLINE_20N_WINDOWS.ps1`: só abre o executável da mesma revisão HEAD se houver gate verde e hash coincidente. Usa fixture local, MinimalViewportUpdate somente no processo e não abre conexões físicas.
- `-Abrir` opcional no primeiro script abre a sessão manual imediatamente após passar os gates.

### Comando único para validar e abrir após aprovação

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch candidate/offline-20n-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha na troca de branch: nao use force/reset/clean' }
git pull --ff-only origin candidate/offline-20n-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\HOMOLOGAR_CANDIDATA_OFFLINE_20N_WINDOWS.ps1' -Abrir
```

Para abrir novamente sem recompilar (após PASS), executar:
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\ABRIR_CANDIDATA_OFFLINE_20N_WINDOWS.ps1'
```

## Critérios ainda pendentes
1. Passar os 14 gates na branch integrada; **ainda não executados no Windows após estes commits**.
2. Testar de verdade Ctrl+D com 1, 2, 20 e ~575 objetos; um Undo deve desfazer apenas o lote; Redo restaurar; conferir grupos, camadas, texto vetorial e seleção visual.
3. Testar seleção, Tesoura, zoom/pan, importação e abertura/salvamento de **cópias** de `.alaser` e `.lbrn2`. Não sobrescrever originais.
4. Comparar fluidez no mesmo arquivo, número de nós/objetos, DPI e sequência de eventos. Medidas reais despareadas não são homologação.
5. Backup, recuperação, nova instalação em **diretório de homologação** e reversão testada antes de promover instalador.
6. GRBL e laser físico permanecem excluídos.

**Estado:** fonte e automação publicadas; não há novo executável compilado no Windows nesta etapa. Nenhuma mudança em `main`, `APLICATIVO` ou instalador de produção.
