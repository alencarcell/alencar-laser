# Alencar Laser — Fase 20q: dentes em ponta angular / serrilhados
Data: 09/11/2026. Branch: `feat/20q-angular-gear-20261109`.

## Referência e intenção
Desenho manual enviado pelo operador: roda circular com dentes inclinados e pontas afiadas, visualmente próxima de uma lâmina serrilhada, diferente da engrenagem de involuta exibida no gerador anterior. Objetivo é obter **contorno vetorial editável para corte laser em MDF**, não comprovar transmissão de força nem geometria de uma serra operacional.

## Implementação (já publicada)
- `DESENVOLVIMENTO/FONTE/src/gear_generator_19_9.hpp`:
  - `ToothProfile::Involute` continua **padrão** e usa o cálculo anterior, sem alterações no ramo evolvente.
  - `ToothProfile::Angular`: triângulos com vértice radial central, duas laterais retas simétricas.
  - `ToothProfile::Sawtooth`: um vértice deslocado no sentido angular, criando a silhueta serrilhada inclinada do rabisco.
  - `angularBasePercent` controla o ângulo da base do dente (45–90% de um passo); `tipPositionPercent` controla a posição do vértice serrilhado (15–85%, 50% central).
  - Quatro vértices por dente, sem redundância no ponto de fechamento; 24 dentes = 96 vértices no contorno externo. Não são gerados milhares de nós desnecessários.
  - Contorno externo anti-horário, furo central horário. Raio de ponta `r_a = m(z/2+1)`, raio de raiz `r_f = m(z/2-1,25)`. Validações de raio/furo e entrada preservadas.
  - A forma serrilhada não é uma involuta nem um mecanismo de catraca testado.
- `DESENVOLVIMENTO/FONTE/qt/gear_dialog_19_9.hpp`: seletor **Perfil do dente** com 3 opções; controle da base, posição de ponta e prévia ao vivo; parâmetros de ângulo de pressão ficam desativados para perfis não evolventes. Alerta visível de caráter ilustrativo.
- `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp`: a opção escolhida é enviada ao editor, gerando contornos fechados no desenho em um Undo. Status diferencia perfil angular decorativo.

## Gates novos
1. `alencar_test_gear_angular_20q`: núcleo independente Qt, verifica 12/24/60/120 dentes, raio de ponta/raiz, quatro nós por dente, orientação de contornos, furo, deslocamento do vértice e entradas inválidas.
2. `alencar_test19_9_gear_offline`: regressão antiga da involuta, pares e cremalheira.
3. `--gear-angular-20q-test`: Qt, verifica controles, mudança visual na prévia, criação de dois contornos no editor, Undo/Redo atômico e ausência de comandos ao laser.
4. `--gear-generator-19-9-test`: regressão Qt anterior: gerador antigo, par nominal, cremalheira e oficina de mecanismos.
5. `SCRIPTS/TESTAR_ENGRENAGEM_ANGULAR_20Q_WINDOWS.ps1`: compila executáveis isolados e executa esses quatro gates, gravando relatório local.

### Executar no Windows
Feche o Alencar Laser de homologação e deixe a máquina fisicamente desconectada.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch feat/20q-angular-gear-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha na branch; nao use force/reset/clean' }
git pull --ff-only origin feat/20q-angular-gear-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_ENGRENAGEM_ANGULAR_20Q_WINDOWS.ps1'
```

## Requisitos de aceitação e segurança
- Os quatro gates passam no Windows com Qt 6.12 e C++20 (ainda pendentes neste commit).
- Visualmente, o perfil angular exibe pontas centralizadas e o serrilhado exibe pontas inclinadas; sem auto-interseções, sem duplicações de nós, sem furo deslocado.
- Gerador involuta original, pares e cremalheira continuam sem mudança de saída.
- Criar a roda no editor e fazer Undo/Redo; exportar apenas após verificar visualmente, sempre em projeto de teste.
- NÃO misturar o perfil serrilhado com a Oficina de Mecanismos nem homologá-lo como roda dentada de transmissão.
- Nenhum código de máquina, G-code, controle físico, laser, `main`, `APLICATIVO` ou instalador de produção foi alterado.
- O ZIP portátil 20p anterior **não contém** esta função. A integração exige uma futura candidata específica depois dos gates.

**Estado:** código, interface e testes publicados no GitHub. Não compilado nem validado no Windows do operador nesta fase.
