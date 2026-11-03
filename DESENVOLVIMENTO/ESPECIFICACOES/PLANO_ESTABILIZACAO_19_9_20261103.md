# ALENCAR LASER — PAUSA DE RECURSOS E ESTABILIZAÇÃO (03/11/2026)

## Decisão
O operador relatou piora importante nos travamentos, mesmo após várias tentativas de otimização. **Nenhuma nova otimização deve ser apresentada como sucesso sem evidência real**, e novos recursos ficam fora de prioridade até restaurar responsividade na mesma carga. PR #13 permanece draft, sem merge na `main`, sem publicar instalador, sem acionar hardware laser.

## Dados reais de 417 contornos / 122.390 nós
| Métrica | JSON 03/11 16:44 | JSON 03/11 18:11 |
|---|---:|---:|
| Paint P50 | 55.3875 ms | 14.0152 ms |
| Paint P95 | 1569.4323 ms | 1009.5903 ms |
| Paint máximo | 3119.7382 ms | 1304.2464 ms |
| Hit-test P95 | 180.9861 ms | 20.3628 ms |
| Input-to-paint P95 | 886.319 ms | 193.997 ms |
| Rebuild P95 | 32.1031 ms (n=10) | 823.4092 ms (n=8) |
| Rebuild máximo | 63.2177 ms | 1481.6027 ms |
| Style P95 | 3.0168 ms | 20.3233 ms |
| Arraste agrupado controle central | 0 | 1 (6 frames) |

**Atenção:** as sessões têm operações diferentes. O segundo JSON registra salvar, importar LightBurn, mover 417, undo e redo; não pode atribuir a mudança do tempo de rebuild apenas à otimização de arraste. O resultado individual do preview transform ficou em 0.8246 ms P95 (6 amostras). O gargalo de atualização integral da cena permanece relevante e os tempos de pintura continuam altos para fluidez.

## Inspeção técnica atual
- `QtEditorLab::finishSelectionTransform()` continua chamando `rebuild()` após mover objetos.
- `QtEditorLab::rebuild()` executa `scene_.clear()`, invalida ponteiros, reconstrói paths de todos os contornos com `scene_.addPath(pathOf(c))` e cria novamente itens nativos e raster; isso é uma fonte potencial de bloqueio da thread UI em operações de edição grandes.
- Já existem testes funcionais das operações, mas nenhum teste A/B controlado de latência a 122.390 nós.
- O arquivo LightBurn real `ampulheita do tempo.lbrn2` contém cerca de 2.133 Shapes Path e uma Shape Ellipse cujos atributos usam `Rx/Ry` (e `XForm`), enquanto o importador produziu `Dimensoes ausentes para Rect/Ellipse`. É uma incompatibilidade de importação **separada** da responsividade. Não modificar arquivo-fonte do usuário, não fazer importação parcial silenciosa.
- Crash interativo anterior `std::out_of_range map::at` continua sem correção comprovada. GDB e capturas precisam ser preservados. Não tratar como resolvido apenas porque outro lançamento abriu.

## Medida de contenção (sem alterar desenhos)
- `SCRIPTS/ABRIR_19_9_WINDOWS.ps1 -ModoCompatibilidade` desativa temporariamente agrupamento de arraste pela variável de processo `ALENCAR_DISABLE_GROUPED_DRAG=1`, sem instalar pacotes, modificar modelos, camadas, histórico, arquivos ou configurações permanentes.
- Isso apenas isola um experimento; **não garante fluidez nem evita o full rebuild**. Restaurar a variável de ambiente ao sair é obrigatório.
- Se o programa travar, interromper teste e preservar arquivos. Não sobrescrever projeto de origem.

## Próximas tarefas na ordem (sem atalhos)
1. Reproduzir regressão de forma isolada: mesma cópia do documento pesado, mesmos passos e zoom, 2 execuções com e sem `-ModoCompatibilidade`, cada uma com JSON. Se houver risco de perda de dados, parar.
2. Perfil detalhado de `rebuild()` em fases separadas: limpeza da QGraphicsScene, geração de QPainterPath, criação de QGraphicsItem, aplicação de estilos, raster/nativo e pós-processamento. Medir P50/P95 e congelamentos.
3. Só então implementar **atualização incremental de paths modificados** sob flag experimental e com regressão de fidelidade, Undo/Redo, imagens, camadas, seleção e hover. Evitar simplificar nós ou alterar a representação canônica.
4. Verificar inicialização e recuperação; corrigir `map::at` pelo stack trace em vez de mascarar exceção.
5. Tratar importação Rect/Ellipse `Rx/Ry` em bloco separado, com testes do arquivo do usuário e preservação integral de caminhos.
6. Homologação apenas após A/B mensurado e estabilidade em uso normal.

## Estado
Contenção implementada na branch no commit `6478944`; script revisado estruturalmente, **não executado no Windows**. Esta fase não reverte nem substitui a versão aprovada. O próximo foco técnico é mensurar o full rebuild e a pintura; nenhuma mudança física na máquina laser.


## 03/11/2026 — GDB capturou nova exceção ao abrir arquivo real
- Captura da janela Windows confirma `GDB exit code=0` da ferramenta, mas **"Excecao capturada"** na sessão de depuração: isso **não significa que o aplicativo terminou normalmente**. A pilha está no TXT local `RELATORIOS/DIAGNOSTICOS/FECHAMENTO_GDB_19_9_20261103_160816.txt`, ainda **não enviado**; não afirmar qual função lançou `std::out_of_range` até examinar o backtrace.
- Arquivo original inspecionado somente leitura: `ampulheita do tempo.lbrn2` (enviado também como `ampulheita do tempo(1).lbrn2`, SHA256 idêntico). É XML LightBurn 2.1.02, 2.134 shapes: 2.133 `Path` e 1 `Ellipse` com raio `Rx=1.7500045, Ry=1.7500045` e `XForm`. Nenhuma Shape tem W/H. O erro anterior `Dimensoes ausentes para Rect/Ellipse` estava relacionado ao parser que só aceita W/H.
- Arquivo tem 16.401 primitivas Bézier `B` e 1.275 segmentos de linha `L` além de 3 caminhos `LineClosed`; com tesselação fixa de 24 segmentos por Bézier, produz **aproximadamente 397 mil nós**. Isso excede o limite do importador, 250 mil; não é sinal de corrupção do arquivo.
- Patch experimental `lightburn_geometry_import.hpp`: interpretar Rx/Ry como raios, validar valores positivos e conflito com W/H; calcular limite inferior de nós após resolver as referências compartilhadas e **bloquear antes de tesselação massiva** qualquer arquivo que já exceda 250 mil, sem tocar no documento aberto; conferir total após cada raiz. **Não aumenta o limite, não simplifica curvas silenciosamente e não gera G-code**.
- Regressão offline Qt `alencar_test19_9_lbrn2_safe_import`: elipse Rx/Ry com transformação, W/H conflitantes e arquivo sintético acima do limite; execução específica pelo script `TESTAR_19_8_WINDOWS.ps1 -SomenteImportacaoSegura`, opcionalmente `-ArquivoLbrn2 <caminho local>`. **Ainda sem compilação Windows desta revisão.**
- Importação fiel do arquivo real exigirá trabalho separado no importador de curvas Bézier nativas (ou simplificação autorizada pelo usuário). Bloquear cedo com explicação é preferível a travar a UI ou importar incompletamente.
- **P0 seguinte:** usuário enviar `FECHAMENTO_GDB_19_9_20261103_160816.txt` para identificar exatamente o `map::at` da exceção; então reparar com teste reproduzível. Não pedir novos testes pesados antes de ler o TXT.
