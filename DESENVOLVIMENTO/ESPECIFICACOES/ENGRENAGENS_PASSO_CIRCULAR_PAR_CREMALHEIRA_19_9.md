# Alencar Laser 19.9 — Gerador editorial de engrenagens (referência Inkscape)

## Escopo deste incremento

- Entrada **módulo** e **passo circular**, ambas em mm, sincronizadas: `p = π · m`.
- Formulário Qt: número de dentes, ângulo de pressão, furo, centro X/Y e checkbox de pré-visualização.
- Menu: `Gerar > Engrenagens > Engrenagem simples (mm)...`.
- Mantidas as transações de inserção como contornos vetoriais com um Undo e sem alterar outros objetos.
- Núcleo C++ independente de Qt: `generatePairNominal()` fornece razão e distância entre centros; `generateRackNominal()` fornece um perfil **teórico** de cremalheira. Essas duas rotinas ainda **não** estão conectadas à inserção ou à prévia na interface.

## Contrato geométrico

Engrenagem normalizada não deslocada: diâmetro primitivo `d = z · m`; diâmetro de ponta `da = (z+2) · m`; passo circular `p = π·m`.

Par externo nominal: `C = m (z1+z2)/2` e redução `i = z2/z1`. É apenas posição nominal: não resolvemos aqui fase angular, interferência, deslocamento de perfil ou tolerâncias de fabricação.

Cremalheira: largura `z·p`; dentes trapezoidais teóricos no ângulo de pressão informado, adendo `m`, dedendo `1,25m`, base positiva. **Não é um perfil de cremalheira de precisão validado por norma.**

## Cenário de aceitação (Inkscape, imagem do usuário)

- 19 dentes; passo circular 10,000000 mm; pressão 30°; furo 10 mm.
- Conversão esperada: módulo 3,183099 mm; diâmetro primitivo ~60,48 mm; diâmetro externo ~66,85 mm.
- Par ilustrativo: 19 + 38 dentes, mesmo módulo/ângulo -> C ~90,72 mm, redução 2:1.
- Cremalheira ilustrativa: 10 passos de 10 mm -> largura nominal 100 mm.

## Validação e restrições

- Regressão portátil: `DESENVOLVIMENTO/FONTE/tests/gear_generator_19_9_standalone.cpp`, coberta pelo target `alencar_test19_9_gear_offline`.
- Regressão GUI Qt: `--gear-generator-19-9-test`, que verifica campos sincronizados, prévia, contorno e Undo.
- A homologação necessita compilação Windows, testes Qt e inspeção manual das duas capturas de referência.
- O perfil de dente vigente é aproximado. Não comprova engrenamento, resistência estrutural, ausência de interferência ou precisão de montagem.
- **Kerf e folgas de encaixe ainda não são aplicados**. Quando implementados, deverão operar como compensação parametrizada não destrutiva, distinta da geometria nominal e sujeita a cupom de teste no material.
- Não acionar GRBL, Wi-Fi, USB, potência, movimento, G-code ou corte. Não mesclar `main` nem alterar a pasta `APLICATIVO` sem homologação.
