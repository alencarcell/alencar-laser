# Alencar Laser — Módulo rotativo por rolos (planejamento)
Data: 01/11/2026
Status: ESPECIFICAÇÃO DE FUTURA ETAPA — NÃO IMPLEMENTADO / NÃO HOMOLOGADO

## Escopo e ordem de entrega
A etapa só começa após estabilização e homologação manual do editor TESTE_19.8: seleção/tesoura, transformações, persistência v5, prévia, segurança e instalador. Nenhuma data de entrega nem liberação física está definida.

### R1 — Configuração exclusivamente offline
- Nova aba ou diálogo `Ferramentas > Configurar rotativo` acessível sem conectar hardware.
- Tipo inicial: rolo motorizado (Roller); modelo de mandril (Chuck) somente em fase futura, com regras próprias.
- Perfil de rolo salvo separadamente da configuração da mesa plana: identificação do acessório, diâmetro do rolete motorizado em mm, diâmetro e circunferência do objeto, sentido de rotação/inversão visual, referência/origem da arte, comprimento útil e orientação.
- Campo da calibração: `mm por rotação` para controladoras GCode quando apropriado ao firmware; nunca supor passos ou valor numérico de um acessório não identificado. Eixo Y/substituição ou eixo dedicado A/Z só após confirmar a controladora, conexões, firmware e manual específicos.
- Validar números finitos e positivos, limites físicos, circunferência calculada, coerência de unidades, valores incongruentes e perfis incompatíveis.
- Gravação segura do perfil, com recuperação de estado plano padrão no próximo projeto e preservação de backup e arquivos `.alaser`. Sem alterar G-code, comandos, firmware ou parâmetros da máquina.

### R2 — Prévia e simulação offline
- Exibir área retangular desenrolada do cilindro; cálculo de circunferência C = pi × diâmetro do objeto; destacar onde arte excede o perímetro.
- A largura útil é o comprimento axial do objeto; a dimensão de rotação corresponde ao arco percorrido. Não confundir diâmetro do rolete com diâmetro do copo.
- Simular orientação, espelhamento, rotação e eventual repetição ao redor da circunferência, sem movimentar dispositivo; mostrar avisos para objetos cônicos/fora de centro e deslizamento.
- Comparação de uma arte de teste na mesa plana e na superfície desenrolada, mantendo originais intactos e Undo/Redo quando houver transformação editorial.

### R3 — Compatibilidade e calibração real: SOMENTE DEPOIS DA HOMOLOGAÇÃO SEPARADA
- Registrar marca/modelo do rolo do usuário, manual, diâmetro real do rolete, conector, controlador/firmware da AlgoLaser Alpha MK2 20W e modo de ligação. Os valores são dados de calibração, não defaults inventados.
- Teste de direção e rotação completa pode fazer parte de uma missão de autorização física futura; não inserir agora nenhum botão funcional que envie movimento.
- Confirmar intertravamentos, limites, recuperação do modo de mesa plana e retorno a configuração anterior; documentar procedimentos e testes seguros antes de ligar emissão.
- A operação física continua bloqueada no TESTE_19.8, inclusive se um perfil de rotativo for cadastrado.

## Matriz mínima de aceitação
- Perfil de rolo salvo/reaberto offline sem afetar modo plano nem outros projetos.
- Entradas inválidas e diâmetros zero rejeitados de forma atômica.
- Circunferência, posicionamento e desenrolamento testados com exemplos matemáticos conhecidos.
- Área insuficiente e arte maior que o perímetro geram alerta; sem corte silencioso.
- Regressão Windows Qt, persistência e prévia, com mouse físico, zoom/DPI e arquivo real de cópia.
- Nenhuma escrita USB/TCP, nenhum G-code, nenhuma emissão ou movimento antes de aprovação explícita.

## Fontes de referência para futuro desenvolvimento
- LightBurn 2.1 Rotary Mode (GCode): https://docs.lightburnsoftware.com/2.1/Reference/RotaryMode/RotaryModeGCode/
- A referência especifica Roller vs Chuck; para GCode, mm por rotação, diâmetro do rolete, diâmetro/circunferência do objeto e eixo de saída, que depende da controladora. Referência funcional, não copiar identidade/arte do LightBurn.

## Dependências e bloqueios
- O cronograma tem dependências, **não prazos datados**.
- Rolo rotativo incluído como FUTURO MÓDULO, não como função pronta em 19.8.
- `main`, `APLICATIVO`, backups, hardware e controle GRBL não alterados neste registro.
 
## Avanço R1 — Código experimental, 01/11/2026
- O planejamento anterior continua válido. R1 parcialmente implementado para operação **offline** em `src/rotary_geometry_19_9.hpp`, `qt/rotary_dialog_19_9.hpp` e teste C++ portátil, com seleção Rolo/Mandril e configurações de circunferência/diâmetro/rolete, eixo ainda não verificado, espelho apenas de prévia e salvamento QSettings. Este é um recurso experimental, ainda não homologado para uso final.
- A prévia na janela calcula circunferência e mostra proporção da altura de uma seleção em mm; **não é simulação raster/cinemática física**. A simulação cilíndrica completa R2 continua pendente.
- A opção de testar a rotação continua **desativada**; eixo, motor e ligação elétrica são hipóteses até obter manual e inspeção do Ortur real. Não aplicar ajustes do LightBurn ou do software à controladora.
