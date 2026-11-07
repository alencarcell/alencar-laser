# Alencar Laser — candidata offline 20o (07/11/2026)

## Estado e objetivo
Integrar numa base de testes isolada a proteção de duplicação extrema do Ctrl+D, o Ctrl+D múltiplo com Undo/Redo atômico e a estratégia experimental MinimalViewportUpdate. Nenhum instalador ou versão em produção é substituído.

## Evidência da máquina Windows enviada pelo operador
A branch de proteção anterior passou:
- `PASS BULK_DUPLICATE_20M`: teste de 575 objetos, geometria, grupos, texto, camadas bloqueadas, Undo/Redo e carga extrema bloqueada com segurança.
- `PASS CTRL+D Guard 20n`: Qt com 575 objetos e Undo/Redo.
O relatório da candidata 20n anterior havia mostrado G01–G14 PASS, mas um teste manual com duplicações repetidas chegou a 18.400 contornos e 429.280 nós, tornando a janela temporariamente sem resposta. A proteção criada depois desse teste ainda não tinha sido submetida a **todos** os gates integrados.

## Nova candidata
Branch `candidate/offline-20o-20261107` parte de `fix/offline-20n-ctrl-d-load-guard-20261107`.

- `SCRIPTS/HOMOLOGAR_CANDIDATA_OFFLINE_20O_WINDOWS.ps1` compila com CMake numa pasta isolada `build_WINDOWS_CANDIDATE_20O`, testa o núcleo C++ + 13 cenários Qt (14 gates ao todo), com flag de viewport minimal ON somente no processo de laboratório, e escreve SHA-256 do executável num JSON local.
- `SCRIPTS/ABRIR_CANDIDATA_OFFLINE_20O_WINDOWS.ps1` permite reabrir apenas se o SHA-256 e a revisão HEAD coincidirem com um relatório de gate verde.
- O executável é de laboratório, sem código de emissão física validado. Laser fisicamente desconectado.
- O limite de duplicação 6000 contornos / 150000 nós incide **sobre projeção de nova duplicação**, não sobre abertura de trabalhos existentes.
- Cache gráfico 20e e experiências reprovadas 20h/20k desligadas. O modo minimal 20l exige validação real comparável antes de integrar à produção.

## Comando para validar e abrir depois dos gates
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch candidate/offline-20o-20261107
if ($LASTEXITCODE -ne 0) { throw 'Nao force troca de branch' }
git pull --ff-only origin candidate/offline-20o-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\HOMOLOGAR_CANDIDATA_OFFLINE_20O_WINDOWS.ps1' -Abrir
```
Para reabrir depois de PASS na mesma revisão:
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\ABRIR_CANDIDATA_OFFLINE_20O_WINDOWS.ps1'
```

## Pendências antes de qualquer instalador
1. Os 14 gates da nova candidata ainda não foram executados no Windows após estes commits.
2. Validar Ctrl+D com 1, 20 e 575 objetos na GUI real, confirmar seleção e Undo/Redo, e uma tentativa de duplicação excessiva com mensagem sem congelamento.
3. Abrir **cópias** de projetos reais .alaser e arquivos .lbrn2. Verificar Tesoura, zoom, recuperação e histórico.
4. A/B de fluidez com condições comparáveis: mesma geometria e sequência de operações, não apenas P95 global de sessões distintas.
5. Criar instalador de homologação em diretório separado com backup/restauração/reversão. Não liberar máquina física.

**Status:** fonte e roteiros de build publicados no GitHub; sem novo binário compilado nesta etapa. `main`, `APLICATIVO`, instalador e máquina permanecem intactos.
