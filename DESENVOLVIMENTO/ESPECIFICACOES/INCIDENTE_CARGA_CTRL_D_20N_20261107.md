# Alencar Laser — incidente de carga extrema Ctrl+D (07/11/2026)

## Evidências da sessão do operador
A candidata offline 20n passou **G01 a G14** no Windows e abriu a interface. Durante duplicações repetidas de conjuntos muito grandes, a imagem registrou **18.400 contornos, 429.280 nós e 1.150 selecionados**, com a janela mostrando "Não está respondendo". O usuário encerrou a janela, sem intenção de utilizar lotes desse tamanho na prática.

O JSON anexado (`alencar-laser-diagnostics-4`, máquina desconectada) corresponde a 428,448 segundos de sessão; não descreve cada estado intermediário da tela. Mostra:
- Pintura: P50=68,0276 ms; P95=601,9574 ms; P99=5699,2082 ms; máximo=9832,5966 ms; 551 amostras.
- Reconstrução da cena: P95=554,8765 ms; máximo=4300,6714 ms; 72 reconstruções.
- `input_to_paint_p95_ms=199,5617`; `foreground_p95_ms=4,024`; `background_p95_ms=31,9776`; atualização de viewport `minimal`.
- 2 eventos `command_error`; diagnóstico não registra a causa individual.
- 24 operações Undo, 16 remoções contínuas via Tesoura, 1 ausência de alvo da Tesoura, 7 movimentações; 0 eventos perdidos.
- Estado final após as operações: 561 contornos e 12.026 nós; não confundir com o pico da captura.
- `machine_connected=false`, `diagnostic_probes_sent=0`. Sem indício de acionamento físico.

## Diagnóstico
O núcleo e o atalho Ctrl+D **passaram** 575 objetos e Undo/Redo. O problema agora é custo explosivo de múltiplas duplicações sucessivas: operações com milhares de itens multiplicam custos de `Editor::change` (cópia de estado inteiro), validação, histórico e reconstrução da cena Qt. Não há evidência de falha geométrica ou perda definitiva de dados; a janela Windows foi marcada sem resposta durante a carga extrema.

## Proteção implementada (branch isolada)
`fix/offline-20n-ctrl-d-load-guard-20261107`, não integrada à candidata:
- Pré-validação antes da cópia do estado para recusar duplicação que excederia **6.000 contornos** ou **150.000 nós**, mantendo a operação em um único Undo quando permitida.
- Segunda validação após expandir grupos/textos, antes de criar clones/IDs.
- Mensagem de erro orienta reduzir seleção; rejeição não altera geometria nem histórico.
- Esses limites se aplicam **somente à duplicação**. Não bloqueiam abrir ou editar projetos existentes. São limites conservadores sujeitos a revisão mediante benchmark.
- Teste C++ estendido: 575, 1.150, 2.300 e 4.300 contornos permitidos, depois tentativa projetada de 6.100 rejeitada com estado/histórico inalterados. Teste Qt de Ctrl+D com 575 segue presente.
- `SCRIPTS/TESTAR_GUARDA_CTRL_D_20N_WINDOWS.ps1` compila/testa a proteção num diretório Windows separado, sem instalar, promover ou conectar à máquina.

## Executar
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha fetch' }
git switch fix/offline-20n-ctrl-d-load-guard-20261107
if ($LASTEXITCODE -ne 0) { throw 'Nao force troca de branch' }
git pull --ff-only origin fix/offline-20n-ctrl-d-load-guard-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_GUARDA_CTRL_D_20N_WINDOWS.ps1'
```

## Critério para integrar
Teste Windows C++/Qt verde, duplicação manual normal (1, 20, 575), recusa explicada para carga acima do limite e ausência de perda de documento/Undo. Depois reexecutar a regressão geral e criar candidata atualizada. Não promover a atual 20n enquanto falha de carga extrema não tiver proteção validada. Nenhuma alteração na versão instalada e nenhuma homologação GRBL/laser físico.
