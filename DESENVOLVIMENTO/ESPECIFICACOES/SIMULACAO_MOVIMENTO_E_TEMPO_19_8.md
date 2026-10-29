# Alencar Laser — Prévia virtual com tempo cinemático (TESTE_19.8)

**Data:** 28/10/2026  
**Branch:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Estado:** implementação experimental no GitHub; compilação/GUI Qt6 Windows e testes com mouse ainda pendentes. `main` e `APLICATIVO` não são alterados.

## 1. Objetivo e limites

Refinar a estimativa de tempo da prévia sem habilitar transmissão, movimentos, potência, ar assistido ou G-code. O motor é inteiramente local: recebe uma cópia de leitura do estado do Editor e gera uma sequência virtual `Move{from,to,laser,layer}`. O booleano `laser` **é apenas uma etiqueta visual** (não é comando).

**Coberto nesta rodada:** contornos `Line`; deslocamentos entre contornos; limite de velocidade X/Y e acelerações distintas para linhas e viagens; repetição do plano completo em 1–20 passagens; cronograma cumulativo; barra de progresso por tempo; reprodução virtual até 40×; métricas numéricas e exportação JSON local sem coordenadas.

**Não coberto:** `Fill`/imagem raster, ordenação por camada do LightBurn, compensações de kerf, tabs, Z/foco, overscan, velocidade de canto GRBL real, junções/junction deviation, otimização, origem real da controladora, micro-pausas, buffer, tempo de comunicação, tempo de ignição, atraso de ar e dinâmica real do equipamento. Acelerações e limites têm valores padrão *hipotéticos*, sem leitura ou gravação na máquina.

## 2. Modelo geométrico e de tempo

1. `sim198::build(state)` filtra T1/T2 e `Output=false`, ignora `Fill` e adiciona os deslocamentos sem laser a partir de (0,0). A geometria original não muda.
2. `sim198::repeated(plan, passes)` duplica o plano inteiro e modela uma viagem sem laser de retorno à origem virtual antes da passagem seguinte. **Não equivale aos passes por camada/forma do LightBurn.** Limites: 1–20 passes e até 300 mil movimentos.
3. Para um movimento com comprimento `d` e direção unitária `u=(ux,uy)`, a velocidade escalar máxima (mm/s) respeita a velocidade solicitada e os tetos projetados `Vx/|ux|` e `Vy/|uy|`. A aceleração efetiva (mm/s²) usa o mínimo de `Ax/|ux|` e `Ay/|uy|`.
4. Movimentos contíguos do mesmo tipo/camada com mudança angular de até **12° entre segmentos adjacentes** são agrupados como uma corrida. Em cada corrida aplica-se um perfil *repouso–repouso* (triangular ou trapezoidal): se `d <= v²/a`, `t=2sqrt(d/a)`; caso contrário `t=2v/a+(d-v²/a)/v`. A regra simplifica as transições de velocidade e pode superestimar trechos com curvas suaves ou subestimar outros efeitos físicos.
5. Para o slider temporal, a duração da corrida é dividida entre os segmentos **proporcionalmente ao comprimento**. O instante e posição parcial dentro do segmento são aproximações visuais, não amostras reais da cinemática GRBL. A interface mostra também o tempo puramente nominal (distância/velocidade solicitada) para comparação.

A referência usada é a documentação do LightBurn, que descreve limites X/Y, aceleração e ajustes de estimativas em **Additional Settings**. Não copiamos nem alegamos equivalência com o simulador interno do LightBurn:
- https://docs.lightburnsoftware.com/latest/Reference/DeviceSettings/AdditionalSettings/
- https://docs.lightburnsoftware.com/latest/Troubleshooting/PreviewWindow/InaccurateTimeEstimates/
- https://docs.lightburnsoftware.com/2.1/Reference/Preview/

## 3. Arquivos modificados

- `DESENVOLVIMENTO/FONTE/src/preview_motion_19_8.hpp` — novo núcleo cinemático C++20 sem Qt.
- `DESENVOLVIMENTO/FONTE/src/preview_sim_19_8.hpp` — passes completos, viagens de retorno e limites.
- `DESENVOLVIMENTO/FONTE/qt/preview_dialog_19_8.hpp` — controles de velocidade, aceleração, X/Y, passes, slider por tempo, reprodução e exportação local de estatísticas.
- `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp` — teste `--simulation-motion-19-8-test` com JSON e regressão também em `--inline-transforms-19-8-test`.
- `DESENVOLVIMENTO/FONTE/tests/preview_motion_19_8_standalone.cpp` — teste portátil determinístico + 1.000 trajetórias pseudoaleatórias.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1` — opção `-SomenteSimulacao`; a suíte completa também chama o teste específico.

## 4. Evidências e critérios de validação

**Já executado fora do Windows:** o header cinemático publicado no GitHub foi conferido contra o arquivo C++20 testado localmente (mesmo SHA Git blob `7e4e8ab10daa4bd2627d0987c9359afd2afc034c`). Compilação `g++ -std=c++20 -Wall -Wextra -Werror -pedantic -O2` passou. Harness com perfis triangular/trapezoidal, X/Y/diagonal, acelerações e limites, tempo cumulativo, parâmetros inválidos e 1.000 trajetórias com semente fixa passou. Não equivale a compilação do executável Qt6.

**Próxima validação Windows:**
```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
# Pare se o Git acusar conflito; nunca descarte alterações locais.
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

O teste focado compila e executa harness C++ portátil e executável Qt, escrevendo `RELATORIOS/AUDITORIAS/GITHUB_19_8/simulacao/RESULTADO_PREVIA_MOVIMENTO_19_8.json`. Somente após `PASS` cabe executar `-SomenteInteracao` e `-Abrir` para avaliação visual com máquina desconectada.

**Checklist visual posterior:** abrir preview, verificar linha branca e deslocamento vermelho; mover slider para início/meio/fim; reproduzir/pausar; alterar velocidades X/Y e acelerações; usar duas passagens; desativar `Output` de camada e conferir exclusão; criar T1 e T2 e conferir exclusão; exportar JSON e conferir que não contém coordenadas nem desenho; fechar janela e assegurar que o projeto não mudou. Repetir com arquivo .lbrn2 real, comparando com dados do LightBurn, sem contato físico com a máquina.

## 5. Segurança, dados e critérios de evolução

`QSaveFile` grava relatório JSON **somente após escolha explícita do usuário** no seletor local. O relatório contém tempos, distâncias totais, valores hipotéticos, contagens e advertências, mas não contém nós, vértices, arquivos originais, dados GRBL nem identificadores da máquina. Não há conexões, serial, TCP nem API de transmissão nos novos módulos.

Para calibrar estimativas futuras, coletar *tempo real de execução*, tempo estimado LightBurn, parâmetros de corte, velocidade máxima e aceleração X/Y, número de passes e tipo de camada. Tratar todos os dados recebidos como observacionais até serem associados ao mesmo trabalho. **Não habilitar movimento/emissão** nesta etapa.

## 6. Pendente

- Compilação e testes Qt6 reais no Windows.
- Revisão de comportamento para desenhos grandes (slider/reprodução com muitas arestas).
- Raster `Fill` e `Image`, overscan e ordem de camadas.
- Leitura passiva e opcional dos parâmetros da máquina; calibração com relatórios reais.
- Homologação separada dos controles físicos, após controles de segurança, permissões e testes locais.
