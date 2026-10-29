# Alencar Laser 19.8 — Auditoria de consolidação, simulação offline

**Data:** 28/10/2026  
**Branch exclusiva:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Escopo:** consolidar núcleo de prévia, rotas, camadas, Fill/bitmap e testes sem alterar `main`, versões homologadas, `C:\ProjetoAlencarLaser\APLICATIVO` ou qualquer conexão física de laser.

## 1. Evidência de entrada

O operador confirmou anteriormente em seu Windows:
- atualização fast-forward do GitHub;
- CMake gerou projeto;
- teste C++20 de movimento com 1.000 trajetórias: PASS;
- compilação Qt e teste **sintético de movimento**: PASS.

**Não inferir desses resultados que Fill, imagem, S/V/A, M3/M4, novas rotas, aninhamento ou UI de duplo clique passaram.** Esses códigos foram adicionados depois. O usuário optou por testar a candidata mais completa ao voltar ao computador.

## 2. Revisão dos módulos (inspeção de fonte na branch)

- `src/editor_tools.hpp`: metadados por camada têm velocidade, potência nominal, passes, M3/M4, Output/Show/Air e opt-in para otimização. T1/T2 nunca integram saída; as configurações da janela são editoriais.
- `src/preview_sim_19_8.hpp`: plano em memória de segmentos ativos e viagens, repetição por camada e passes globais com limite de movimentos. Não gera G-code.
- `src/preview_motion_19_8.hpp`: tempo nominal/cinemático simplificado, limite de velocidade/aceleração X/Y. Não imita os perfis de GRBL.
- `src/preview_raster_19_8.hpp`: raster virtual para Fill por paridade e máscaras binárias, overscan sempre sem emissão; há limites de densidade e dimensões.
- `src/preview_route_19_8.hpp`: vizinho mais próximo e precedência `innerFirst` apenas na mesma camada; respeita opt-out por camada e ignora T1/T2.
- `src/preview_nesting_19_8.hpp`: geometria estrita de contenção, rejeita tangência, cruzamentos e caminhos complexos/ambíguos. **Correção desta rodada**: validação de simplicidade do polígono em cache para não repetir trabalho quadrático e rejeição de contornos internos fechados autointersectantes antes de impor precedência.
- `qt/preview_dialog_19_8.hpp` e `qt/layer_settings_19_8.hpp`: dados e opções são usados apenas em prévia/salvamento local; a interface ainda precisa de validação visual Qt Windows.

## 3. Novos gates de regressão

1. **`tests/preview_route_19_8_standalone.cpp` (C++20 sem Qt):** 
   - três retângulos aninhados cortados de dentro para fora;
   - opção global `innerFirst` desativada;
   - opt-out `Layer.optimizeTravel=false`;
   - linha aberta inteiramente interna;
   - mesma geometria em outra camada sem precedência cruzada;
   - contorno tocando borda sem falsa precedência;
   - contorno interno autointersectante rejeitado da regra de precedência;
   - guias T1/T2 ignoradas;
   - passagens por camada e globais com comprimentos esperados;
   - **invariante de continuidade**: todo deslocamento tem origem no fim do movimento anterior, e não há salto silencioso com trecho ativo.
2. **`SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteSimulacao`**: compila o novo teste C++20 portátil junto aos testes de movimento e raster e depois executa os gates Qt já preparados.
3. **`.github/workflows/preview-19-8-offline.yml`**: pipeline de teste C++20 em GitHub Actions apenas na branch experimental, `permissions: contents: read`, sem segredos, sem Qt e sem conexão com máquina. Tenta instalar Boost.JSON e compilar os três testes portáteis. A presença do YAML no repositório **não comprova que Actions executou ou passou**.

## 4. Resultados: distinção entre verificação e execução

- **Verificado nesta rodada**: conteúdo e integração por inspeção do código e presença dos novos testes/script/workflow nos arquivos do GitHub.
- **Não executado neste ambiente**: compilação C++20 atual, CI remoto com resultado verificável, compilação Qt6 e inspeção visual no Windows. A consulta do conector GitHub não retornou runs/status para os commits consultados; isso não é um PASS nem prova de que o workflow falhou.
- **Não homologado**: controle físico, G-code, CAM, motor, assistência de ar, potência M3/M4 e ligação com o equipamento.
- **Riscos residuais**: erros de compilação pela integração do Qt, layout e clicabilidade dos checkboxes, desempenho da prévia com projetos extensos, ordenação real multi-camada, reproduzir trajetórias com limite de 300 mil movimentos e divergência de tempos frente ao LightBurn.

## 5. Critério de saída para candidata integrada

No Windows, com hardware desconectado:
```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Atualizacao interrompida; preservar arquivos locais." }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```
Se `-SomenteSimulacao` passar, executar `-SomenteInteracao` e depois `-Abrir`, sem conexão à máquina. Conferir prints e vídeo de: ícones laterais, S/V/A compactos, duplo clique, Undo/Redo, modo M3/M4 de referência, Fill/imagem, viagens, passes e roteamento interno.

**NÃO executar** `git reset --hard`, `git clean`, sobrescrever `APLICATIVO`, mesclar em `main` ou prometer certificação de corte/gravação antes de evidência física supervisionada e revisão de segurança.

## 6. Próxima prioridade

Deixar a base 19.8 compilar e passar a suíte Windows completa, corrigir erros de integração que apareçam nos logs, depois avaliar o mesmo arquivo no LightBurn para medir tempos/distâncias e tratar a ordenação por grupo/prioridade em futura etapa. Para auditoria de vários vídeos/projetos/logs em conjunto, o **ChatGPT Work** pode ser mais adequado.
