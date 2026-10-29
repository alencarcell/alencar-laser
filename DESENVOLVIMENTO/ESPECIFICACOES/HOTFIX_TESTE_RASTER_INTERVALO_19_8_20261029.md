# Alencar Laser TESTE_19.8 — Hotfix de fixtures raster (29/10/2026)

**Branch de trabalho:** `feat/refino-curvas-rastreio-diagnosticos-19-8`.  
**Gatilho:** captura de tela do usuário em VS Code, após `TESTAR_19_8_WINDOWS.ps1 -SomenteSimulacao`.

## Evidência observada

- `PASS: profiles, axis caps/accel, continuity, 1000 random trajectories` — o teste de movimento C++20 passou.
- `terminate called after throwing an instance of 'std::runtime_error'`
- `what(): Intervalo ou overscan virtual fora dos limites`
- `Teste C++ de Fill e bitmap falhou`, antes da compilação Qt6 nesta execução.

## Causa raiz confirmada na fonte

`src/preview_raster_19_8.hpp::validate(Settings)` permite `0.05 <= intervalMm <= 5.0` e `0 <= overscanMm <= 20`. Esse limite coincide com o controle Qt de intervalo (0,05–5 mm).

Os testes recém-adicionados tinham `cfg.intervalMm=10.0` ou `scan.intervalMm=10.0`, **fora da faixa do motor**, e chamavam `appendMask` sem esperar a exceção. Trata-se de fixture de teste inválida, não evidência de defeito no hardware.

## Correções publicadas

- `tests/preview_raster_19_8_standalone.cpp`: casos de bitmap agora usam **5 mm**, altura **10 mm** para duas linhas de 4 pixels (20 mm + 10 mm ativos = 30 mm). Cenário de passo constante usa altura **12,5 mm** com linhas nos centros `y=2,5` e `y=7,5`. Imagem menor que meio intervalo usa altura **2 mm** e uma linha em `y=1`. Teste de overscan >20 mm mantém intervalo **válido** de 5 mm para isolar a causa correta. Incluído teste explícito de fronteira que aceita 5 mm e rejeita 10 mm.
- `qt/qt6_test6.cpp::previewRaster198Test()`: mesma correção de intervalo e geometria esperada, evitando o mesmo erro no gate Qt6.
- `src/preview_raster_19_8.hpp`: **sem alteração nos limites**; o motor e a interface permanecem coerentes.

## Estado da validação

- Código corrigido e conferido no GitHub experimental.
- Compilação e execução do hotfix no Windows **ainda pendentes**; não declarar PASS após apenas publicar os arquivos.
- Nenhum teste físico foi autorizado. Nenhuma mudança em `main`, `APLICATIVO`, controladora, máquina, G-code, CAM, potência ou M3/M4.

## Repetição segura no computador

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Git não atualizou; preservar arquivos locais." }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

A pasta local `.vscode/` não deve ser apagada. Se outro teste falhar, usar a **primeira mensagem real de erro** e `RELATORIOS/COMPILACAO/BUILD_GITHUB_19_8.txt`, em vez de assumir que a correção do raster resolve todas as possíveis regressões.

## Teste rápido adicional (sem Qt6)

Foi acrescentado `-SomenteRaster` em `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`. Esse parâmetro **não executa CMake nem compila a interface Qt6**: compila somente o teste portátil C++ de Fill/bitmap, e retorna falha se o executável não passar. Após `git pull --ff-only`, executar:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteRaster
```

Se aparecer `RASTER C++ PASS`, só então usar `-SomenteSimulacao` para os testes integrados. O comando rápido **não certifica** a prévia Qt6 nem a candidata completa.
