# Alencar Laser — Experimento isolado Fase 20d (06/11/2026)

## Implementado
Branch: `exp/phase20d-item-cache-20261106`, derivada de `fix/curvas-tesoura-preflight-19-8-20261030`. O renderizador Qt principal NÃO foi editado.

Arquivos novos:
- `DESENVOLVIMENTO/FONTE/experiments/phase20d/cache_probe.cpp`
- `DESENVOLVIMENTO/FONTE/experiments/phase20d/CMakeLists.txt`
- `SCRIPTS/TESTAR_CACHE_FASE20D_WINDOWS.ps1`

Lab Qt independente cria desenhos sintéticos determinísticos e compara `NoCache` com `DeviceCoordinateCache` apenas em paths com pelo menos 129 nós. Cenários de 40, 128 e 575 contornos, zoom 1, 1.2 e 2; 12 frames temporizados após aquecimento. Conta TODOS os pixels divergentes e diferença máxima por canal (sem considerar equivalência automática). JSON fica apenas em RELATORIOS locais.

**Limites de validade:** comparação em QGraphicsScene render offscreen, não acompanha interações reais do mouse, seleção, zoom dinâmico, rotação, alterações geométricas, múltiplas telas nem uso de memória. O modo DeviceCoordinateCache não é equivalente a uma cache de geometria; alteração do zoom pode invalidá-la. Resultados em 40/128 têm 0 contornos 129+, portanto são controles negativos úteis. Medianas isoladas não garantem aceleração do aplicativo. Mesmo se pixels iguais, isso não homologa para produção.

## Comando Windows

```powershell
cd C:\ProjetoAlencarLaser
git switch exp/phase20d-item-cache-20261106
git pull --ff-only origin exp/phase20d-item-cache-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha no git pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_CACHE_FASE20D_WINDOWS.ps1'
```

Caso `git switch` seja bloqueado por mudanças locais, NÃO usar force, reset ou clean. Preservar alterações e diagnosticar primeiro. Script não executa o aplicativo principal nem envia comandos ao laser; versão instalada e `main` intactas.

## Gates posteriores antes de qualquer integração
1. Compilar e executar no Windows; obter 9 medições e verificar relatório.
2. Inspecionar raster de diferença com zoom/pan reais (pixel divergence) e visibilidade de traços finos.
3. Repetir na cena real com seleção e prévia de arraste; avaliar RSS e uso de CPU.
4. Demonstrar preservação de geometria, Undo/Redo, exportação e camadas.
5. Só então avaliar opt-in no programa principal, com OFF padrão e reversão imediata.

Status: **código publicado, sem build Windows e sem ganho causal comprovado**.
