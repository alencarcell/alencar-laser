# Alencar Laser 19.8 — Prévia virtual Fill e imagem P&B

Data: 28/10/2026. Branch experimental: `feat/refino-curvas-rastreio-diagnosticos-19-8`.

**Status:** código novo publicado para testes; Qt6 Windows e testes com projetos reais **ainda não executados** nesta etapa. Nenhum movimento físico, G-code, acionamento de laser ou ar implementado. Não mesclar com `main` nem alterar `APLICATIVO` até validação.

## Referências oficiais (não é uma cópia do simulador LightBurn)

- LightBurn 2.1 — Fill Mode: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/FillMode/
  - Apenas contornos fechados. Varredura bidirecional, intervalo entre linhas e ângulo de varredura. Formas sobrepostas ou aninhadas **na mesma camada** usam interação paritária, com vazados; camadas separadas podem regravar a mesma região.
- LightBurn 2.1 — Image Mode: https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/ImageMode/
  - Modos como Threshold, Ordered dithering, Atkinson, Jarvis e Grayscale. Importar bitmap não equivale a traçar vetor.
- LightBurn — Overscanning: https://docs.lightburnsoftware.com/2.0/Explainers/Overscanning/
  - Extensões de deslocamento sem emissão antes/depois da faixa para permitir aceleração em equipamento físico; este projeto mostra apenas geometria estimada.
- LightBurn — Preview: https://docs.lightburnsoftware.com/2.1/GetStarted/PreviewBeginner/
  - Exibe trabalho e deslocamentos em cores diferentes, ordem e tempo simulado.

## Implementado

1. **`src/preview_raster_19_8.hpp`**, independente do Qt:
   - `appendFill` separa formas fechadas em camadas `mode=Fill`, `Output=true`; exclui sempre guias T1/T2. Aceita também fechamento repetido da primeira coordenada. Faz interseções de cada linha horizontal com todas as arestas da mesma camada, ordena X e emparelha por regra **even–odd**, formando ilhas e vazados. Não move nós do documento.
   - `appendMask` recebe máscara binária em memória e dimensões físicas. Agrupa pixels pretos consecutivos em segmentos ativos virtuais. Reconstrói linha por linha com espaçamento físico configurável, sem escrever bitmap ou vetores originais.
   - `emitRow` descreve deslocamentos sem emissão ao redor das faixas (overscan geométrico) e opção bidirecional. `Move.laser=true` é apenas metadado **da visualização**, nunca um comando.
2. **`qt/preview_dialog_19_8.hpp`**:
   - Controles para habilitar Fill e imagem, espaçamento de linha (0,05–5 mm), overscan virtual (0–20 mm), alternância de direção.
   - Imagem ativa da mesa entra por cópia somente-leitura, reduzida a até 320×320 pixels para prévia. Limiar fixo ajustável (1–254) ou **Ordered Bayer 4×4**. Transparência é combinada sobre branco; nenhum valor de potência é simulado.
   - A interface continua mostrando trajeto ativo claro, viagens vermelhas, reprodução e dois tempos (nominal e perfil simplificado de aceleração). Para planos densos, os desenhos na UI são amostrados visualmente (até ~25 mil movimentos exibidos por repaint); o cálculo usa todos os segmentos do plano.
   - Relatório JSON local opcional registra configurações do Fill/imagem e estimativas, sem coordenadas ou pixels.
3. **`qt/qt6_test6.cpp`** recebe a imagem atual do editor na abertura da prévia, sem alterar `rasterSource_`, documentos ou controle físico; novo modo `--simulation-raster-19-8-test` escreve `RESULTADO_PREVIA_RASTER_19_8.json`.
4. **`tests/preview_raster_19_8_standalone.cpp`** compila com C++20 sem Qt, testando retângulo 10×10, furo interno 4×4, 2 passagens, máscara 4×2, overscan apenas virtual, flags Output, exclusão T1, unidirecional e limites.
5. **`SCRIPTS/TESTAR_19_8_WINDOWS.ps1`** inclui teste C++ portátil do raster e teste Qt no parâmetro `-SomenteSimulacao`, bem como na suíte `-Abrir` (teste Qt).

## Limitações e riscos de interpretação

- **Não** simula potência, intensidade grayscale, dithering de difusão de erro, material, velocidade real por linha, buffer GRBL, aceleração real nos cantos ou tempo de hardware. O `Bayer 4×4` é somente uma aproximação visual de dithering e não representa o processamento exato do LightBurn.
- **Não** há agrupamento otimizador de trajetórias nem priorização de camadas, ângulos alternativos de 90°, modos Fill+Line combinados ou hachura cruzada.
- Fill e máscara assumem linhas **horizontais**, com espaçamento centrado; ordenação pela camada numérica. Contornos abertos são ignorados para Fill. Linhas/objetos fora dos limites da mesa não são corrigidos automaticamente.
- A primeira renderização pode ser interrompida em projetos muito densos; limites intencionais: até 1.500 linhas por camada Fill ou imagem, máscara até 1.000×1.000, e até 250.000 segmentos na montagem raster. A soma de passes respeita 300.000 no plano final. Ao exceder, o diálogo mostra falha limitada em vez de emitir comando ou truncar o cálculo silenciosamente.
- Overscan virtual pode ultrapassar a borda de desenho; **não usar esta prévia para autorizar movimento da máquina**, especialmente sem validação do espaço físico e origem.
- A camada T1/T2 continua sem saída mesmo quando exibida na mesa.

## Testes e validação pendente

O teste anterior do núcleo de **movimento** foi aprovado no Windows pelo usuário (com 1.000 trajetórias e Qt), mas **o Fill/bitmap novo ainda não foi testado no Windows**. Os testes foram adicionados e verificados estaticamente no GitHub, mas essa etapa não recebeu compilação Qt6 real neste turno.

Quando o usuário estiver no Windows, com máquina desconectada:

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Git nao atualizou; preservar alteracoes locais" }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

Se `PASS`, executar `-Abrir` para conferir com mouse real: (1) Fill retângulo 10×10 e vazado; (2) camada Output off; (3) T1/T2; (4) importar PNG arrastando e comparar Threshold/Ordered Bayer; (5) alternar bidirecional e espaçamento; (6) overscan vermelho; (7) JSON só de métricas; (8) confirmar projeto não alterado ao fechar.

**Atenção à integridade:** não fazer `git reset --hard`, `git clean`, apagar a pasta `.vscode`, substituir a base testada ou mesclar branch sem auditoria. Não chamar funções de transmissão da controladora.
