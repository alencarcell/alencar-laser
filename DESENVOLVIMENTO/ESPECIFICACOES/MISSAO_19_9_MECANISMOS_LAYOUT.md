# Missão 19.9 — Oficina de Mecanismos e layout de alta legibilidade

Data: 01/11/2026. Escopo exclusivo desta rodada experimental: simulação cinematica, projeto assistido, biblioteca didática e redesenho da interface. Referências de ergonomia (não de código nem de ícones): documentação LightBurn (Main / Creation / Numeric Edits / Laser / Status toolbars).

## Implantação na branch experimental PR #13

- Núcleo independente de Qt/hardware: `DESENVOLVIMENTO/FONTE/src/mechanism_studio_19_9.hpp`.
- Interface Qt/QPainter: `DESENVOLVIMENTO/FONTE/qt/mechanism_studio_dialog_19_9.hpp`, em **Gerar > Engrenagens > Oficina de Mecanismos — simulação e biblioteca (OFFLINE)**.
- Biblioteca com três conjuntos demonstrativos parametrizáveis:
  1. **Trem de estudo de relógio**, com três engrenagens, **não é um relógio completo**, sem escapamento, eixos/estrutura ou motor.
  2. **Transmissão de brinquedo didático**, duas rodas dentadas externas.
  3. **Pinhão/cremalheira escolar**, com translação idealizada por `s = r_p * theta`.
- Material: MDF, compensado e acrílico; espessura real em mm, dentes do motor e do movido, módulo, ângulo de pressão, furo, kerf **informado** e folga **informada**.
- Diagnósticos: parâmetros finitos e limites de domínio, undercut aproximado, folga menor do que `2 * kerf`, advertência de material, curso e espessura.
- **Importantíssimo**: NÃO há compensação geométrica de kerf, ajuste real de fase de contato, verificação de colisão, resistência de dentes, passo normatizado nem certificação de engrenamento. Os campos kerf/folga **não alteram** os contornos. Não confundir prévia de movimento com simulação de física ou CAM.
- Animação via `QTimer` apenas na janela local, com iniciar/pausar/reiniciar. As rodas externas giram em sentidos opostos, segundo a razão de dentes; trem de três rodas alterna sentido; a cremalheira translada por raio primitivo vezes ângulo. A câmera mantém escala fixa enquanto anima, sem oscilações.
- Inserir gera **contornos editoriais nominais** no documento, agrupados numa única transação Undo; cancelar não gera geometria.

## Identidade/ergonomia

- `DESENVOLVIMENTO/FONTE/src/tool_glyphs.hpp`: símbolos próprios renderizados por QPainter em 32 px lógicos / 64 px raster HiDPI, com contrastes em preto/laranja, estados normais e desativados explicitamente definidos.
- Família visual coerente: selecionar = ponteiro, desenhar curvas = linha+cúbica+âncoras, tesoura = tesoura, formas = contornos reconhecíveis, abrir/salvar = pasta/disquete, desfazer/refazer = setas, iniciar/pausar/parar = triângulo/barras/quadrado, centralizar/alinhamento = linhas de referência + formas.
- Nenhum glifo dependerá de fontes externas, não haverá ícone de ação desconhecida em branco.
- Barra lateral: botões 40×37 px, glifos 24×24, rolagem própria quando não houver espaço suficiente. Barra superior: glifos 23×23 e botões 34–35 px.
- Rodapé: status 25 px, faixa operacional 35 px, campos X/Y/L/A com largura mínima 70 e altura 26; proporção, posição, âncora e tempos continuam disponíveis. A identidade da máquina física permanece **LASER OFF** com tooltip completo. Não reduzir tipografia a ponto de cortar rótulos.
- Controladores físicos Iniciar/Pausar/Parar/Enquadrar preservados DESABILITADOS; a única animação ativa é a simulação OFFLINE dentro da oficina.

## Gates de aceitação

1. Compilar `alencar_test19_9_mechanisms_offline` e `alencar_test7_qt6`.
2. Executar `alencar_test19_9_mechanisms_offline` — três presets, materiais, cinemática e limites; sem acesso físico.
3. Executar `--gear-generator-19-9-test` — selecionadores/avisos, simulação e botões, inserção de modelos, preservação de desenho e um Undo.
4. Verificar ícones em Qt Windows em 100% e 200% DPI, campos completos em resolução 1366×768 e a ausência de corte nas barras.
5. Ensaiar com mouse e arquivo `.alaser` de cópia: abrir/fechar oficina, animação, cancelar, inserir, mover, Undo/Redo, salvar/fechar/abrir; comparar prévia nominal com desenho.
6. Validar matriz de regressão geral Windows Qt, C++ portátil e parser PowerShell; sem `PASS` completo, PR em rascunho.

Nenhuma etapa autoriza GRBL, Wi-Fi de controle, USB, G-code, movimento, potência ou emissão. Não mesclar `main`, não alterar `APLICATIVO`, bases estáveis, firmware ou backups. Não caracterizar modelos didáticos como sistemas mecânicos fabricáveis com precisão garantida.
