# ALENCAR LASER — FASE 20g: DESEMPENHO REAL + GATES CONTROLADOS
Data: 06/11/2026

## Contexto real (fonte: JSON exportado pelo operador)
Diagnóstico `alencar-laser-diagnostics-4`, TESTE_19.8, sessão de 80.573 ms, Windows 11, Qt 6.12.0. O documento tinha 572 polilinhas com 13.404 nós e um Bézier. Foram registrados 10 movimentos de centenas de objetos, 10 transformações agrupadas com 658 atualizações de prévia, 5 seleções por área. Pintura: P50 23.0303 ms, P95 157.4255 ms, P99 192.4586 ms e máximo 608.0681 ms. Prévia de transformação P95 1.3691 ms; hit-test P95 16.5404 ms; input-to-paint P95 42.4615 ms; bbox da pintura P95 100%. Sem conexão física nem probes.

**Interpretação:** há latência elevada na pintura da sessão real. O custo da transformação isolada é baixo, mas NÃO prova causalidade. Sem temporizações vinculadas a cada evento individual, não se pode atribuir P95 ou máximo a arraste/zoom/seleção específica. O P95 da área 100% é sobre a caixa envolvente do paint event, não prova que todos os pixels foram efetivamente redesenhados. Métricas agregadas e percentis diferentes não devem ser somados.

## Implementação
`SCRIPTS/VALIDAR_PERFORMANCE_REAL_20G_WINDOWS.ps1` — branch `test/phase20g-real-gui-perf-20261106` — é um orquestrador de **somente diagnósticos e testes**. Reutiliza os comparadores de 40 e 575 contornos, sem ativar o cache DeviceCoordinateCache reprovado, sem modificar o editor Qt, sem instalar e sem comandos de laser.

- Procura o JSON mais recente `Alencar_Laser_Diagnostico_TESTE_19_7*.json` em `RELATORIOS` ou `Downloads`; aceita `-Diagnostico 'C:\...\arquivo.json'` para apontar exatamente uma sessão.
- Valida esquema, integridade das métricas, hardware desconectado e volume de amostras do diagnóstico.
- Executa o gate A/B com 40 objetos uma vez.
- Executa o teste controlado com 575 objetos duas vezes (configurável 1 a 3); cada uma tem seis processos OFF/ON/ON/OFF/OFF/ON, 144 eventos Qt, modo 3 com política de pintura equivalente OFF/ON.
- Interrompe no primeiro erro, captura stdout/stderr, gera JSON da sessão em `RELATORIOS/AUDITORIAS/GITHUB_19_8/FASE20G_GUI_REAL`. Os dados de projeto, paths e imagens nunca são publicados pelo script.
- `-SomenteDiagnostico` faz exclusivamente análise de JSON local, sem compilação.
- O status final `AUTOMATED_PASS_MANUAL_PENDING` não implica aceleração, fluidez real, homologação visual ou física.

## Executar no Windows
Feche a candidata de teste e mantenha o laser fisicamente desconectado.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha fetch' }
git switch test/phase20g-real-gui-perf-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha switch: nao use reset/clean/force' }
git pull --ff-only origin test/phase20g-real-gui-perf-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\VALIDAR_PERFORMANCE_REAL_20G_WINDOWS.ps1'
```

Se não encontrar o diagnóstico automaticamente, informe o caminho local com `-Diagnostico` ou copie o JSON para `RELATORIOS`. Não envie projeto bruto nem relatórios sensíveis para o GitHub.

## Critério para avançar para 20h
1. Receber PASS funcional e estatísticas Qt repetidas. Nenhum ganho causal é inferido da alternância OFF/ON3 para 575 contornos.
2. Desenvolver **telemetria associada a eventos** com timestamps individuais para separar arraste, zoom/pan, seleção e reposicionamento do Qt; apenas depois escolher uma otimização alvo.
3. Confirmar equivalência visual e vetorial, Undo/Redo, importação e exportação em trabalhos reais.
4. Validar instalador e reversão numa área de homologação.
5. Comunicação física permanece bloqueada para todos esses testes.

**Estado:** script publicado no GitHub, não compilado nem executado no Windows após este commit; main, APLICATIVO e instalador preservados.
