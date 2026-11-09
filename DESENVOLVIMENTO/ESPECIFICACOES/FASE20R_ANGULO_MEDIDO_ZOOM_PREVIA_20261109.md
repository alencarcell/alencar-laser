# Alencar Laser — Fase 20r: ângulo interno medido + zoom/pan na prévia

Data: 09/11/2026. Branch isolada: `feat/20r-measured-tooth-preview-20261109` (derivada de `feat/20q-angular-gear-20261109`).

## O que foi implementado nesta etapa

**1. Ângulo interno da ponta, explicitamente definido.** O gerador angular mantém dois métodos:
- **Por largura da base**: comportamento anterior preservado.
- **Por ângulo INTERNO na ponta**: usuário informa de 30 a 160 graus, com precisão de 0,01 grau. O programa resolve numericamente a posição radial da ponta mantendo a largura angular da base do dente, a quantidade de dentes e o módulo. O ângulo exibido é calculado entre os dois vetores que partem do vértice para as raízes, não apenas uma etiqueta sobre a forma.

Ao usar o segundo modo, a altura da ponta (logo o diâmetro máximo real) pode diminuir em comparação ao raio externo nominal do perfil involuta. Se não for possível atingir o ângulo dentro do envelope geométrico, ou se o dente ficar raso demais, o programa recusa os parâmetros e explica a restrição. O **círculo primitivo continua nominal**, não implica encaixe mecânico homologado.

> **Não confundir o ângulo interno na ponta com o ângulo de 90° desenhado na base do triângulo pelo operador.** O desenho original mostra uma face próxima da perpendicular à base. Ângulo de face em relação à tangente, ângulo interno na ponta e ângulos de bloqueio/impulso do escapamento são grandezas distintas. A geometria de contato da futura roda de escape e âncora Graham será uma função própria, dependente do projeto completo. Esta etapa não afirma reproduzir um escape Graham.

**2. Prévia interativa.**
- Roda do mouse amplia/reduz em torno da posição do cursor, escala entre 50% e 2.000% do enquadramento automático.
- Arrastar com botão esquerdo ou central desloca a prévia sem mudar os contornos vetoriais.
- Botões +, − e Ajustar restauram visualização; mostrador da escala.
- Controles geométricos ficam em painel rolável para não cortar a janela num monitor menor.
- Troca de perfil, ângulo e quantidade de dentes continuam atualizando a prévia.
- A prévia não exporta posições de pan/zoom nem altera documento/Undo.

**3. Compatibilidade e limites.**
- `ToothProfile::Involute` continua o padrão, e a rotina original de involuta não mudou.
- O modo de base (%) angular/serrilhado mantém o comportamento 20q anterior.
- A nova configuração se aplica **somente ao gerador individual**, não ao gerador nominal de pares.
- Nenhum recurso de ligação à máquina, laser, emissão, G-code, instalador, main ou APLICATIVO foi modificado.
- A roda de escape real, com âncora e pêndulo, ainda precisa de projeto e verificação própria.
- Cortes reais em MDF 3 mm exigem validação de tolerâncias, resistência das pontas e compensação do kerf.

## Testes adicionados (ainda não executados no Windows nesta etapa)

O alvo Qt-free `alencar_test_gear_measured_20r` verifica 12, 24, 60 e 120 dentes, perfis angular e serrilhado, ângulos internos 60°, 75°, 90°, 95°, 96°, 110°, 120° e 140°, erros inferiores a 10^-6 grau, altura de dente, limites, e inalterabilidade dos modos legados.

O modo de GUI `--gear-measured-20r-test` verifica o novo campo, prévia para 90°/120°, serrilhado 96°, zoom por roda do mouse, arraste para pan, botão Ajustar, inserção no desenho e Undo/Redo único.

O script executa **6 gates**: núcleo angular 20q, involuta 19.9, núcleo de ângulo medido 20r, GUI angular 20q, GUI legada 19.9, GUI ângulo + zoom 20r. Gera um JSON com revisão do Git e SHA-256 do executável. Uma falha bloqueia a abertura pelo iniciador protegido.

### Testar no Windows

Feche o editor, mantenha o laser fisicamente desconectado e execute:

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch feat/20r-measured-tooth-preview-20261109
if ($LASTEXITCODE -ne 0) { throw 'Troca de branch bloqueada; nao force ou limpe arquivos' }
git pull --ff-only origin feat/20r-measured-tooth-preview-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_ENGRENAGEM_ANGULO_ZOOM_20R_WINDOWS.ps1' -Abrir
```

No editor, abra **Gerar → Engrenagens → Engrenagem**, escolha **Ponta angular** ou **Serrilhado**, depois **Definição angular → Por ângulo INTERNO na ponta**. Teste 90° e 120° com 24 dentes, roda do mouse, arraste e Ajustar. Exporte apenas diagnóstico de sessão; nunca substitua projetos originais.

## Estado real

Código C++/Qt, regressões e scripts **publicados no GitHub, mas ainda não compilados nem executados no Windows do operador** após estes commits. Nenhum resultado de velocidade ou homologação de corte está sendo alegado. Próxima decisão depende do relatório de seis gates e da verificação visual.
