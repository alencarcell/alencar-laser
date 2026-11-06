# Fase 20e — cache sob zoom e pan sintéticos (06/11/2026)

Branch experimental `exp/phase20d-item-cache-20261106`. Adicionados `cache_dynamic_20e.cpp`, alvo CMake `alencar_phase20e_dynamic` e script Windows `SCRIPTS/TESTAR_CACHE_FASE20E_WINDOWS.ps1`.

Executa 6 rodadas por tamanho (40, 128 e 575), alternando a ordem de pintura das cenas com e sem cache. Cada rodada passa por 8 etapas de zoom/pan e mede medianas, diferenças exatas de pixels e RSS do processo. Cena sintética, não uma interação real do usuário. Os controles de 40 e 128 itens não possuem contornos elegíveis para cache; somente o conjunto de 575 inclui 14 contornos com 160 nós.

Não promove o recurso, não altera a GUI principal, documentos, `.alaser`, versão instalada, instalador, main ou comandos de laser. `QGraphicsScene::render` offscreen pode se comportar de forma distinta de `QGraphicsView` interativo; RSS é do processo todo, não apenas do cache. Diferenças de pixels precisam ser analisadas, mas igualdade também não prova equivalência em outras condições. Comparações de tempo são heurísticas, sem teste estatístico.

## Executar no Windows

```powershell
cd C:\ProjetoAlencarLaser
git pull --ff-only origin exp/phase20d-item-cache-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_CACHE_FASE20E_WINDOWS.ps1'
```

Pendente: compilação Windows, leitura das seis rodadas por caso e validação posterior com mouse real/zoom em `QGraphicsView`; não alegar ganho causal ou homologação antes disso.
