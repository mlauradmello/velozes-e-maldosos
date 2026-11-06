# Velozes e Maldosos

Site interativo do protótipo **Velozes e Maldosos** — um jogo de corrida de carrinhos
para dois jogadores, feito com Arduino: quem clicar mais rápido chega primeiro.

O site traz uma representação 3D do protótipo que dá para girar e explorar, com pontos
clicáveis explicando cada parte, a história de onde veio a ideia, a ficha técnica e
fotos do protótipo real.

## O protótipo

| | |
|---|---|
| Comprimento | 55 cm |
| Altura total | 29 cm (com o painel do cenário) |
| Pista | 45 cm, duas raias |
| Controlador | Arduino |
| Driver | Módulo L9110 (ponte H dupla) |
| Motores | 2 × TT (os amarelos) |
| Controles | 2 botões arcade com microswitch + botão de restart |
| Sinalização | LED verde, LED vermelho e buzzer |
| Manutenção | Abertura na traseira com cortina de feltro vermelho |

## O site

- Modelo 3D montado com [three.js](https://threejs.org), a partir das fotos e das medidas do protótipo
- Mini-corrida jogável: teclas **A** e **L**, ou os botões na tela
- Contagem regressiva e música de vitória com as mesmas notas do código do Arduino
- Funciona em celular e computador

## Como rodar

É uma página estática, sem build. Basta servir a pasta:

```bash
python3 -m http.server 8000
```

E abrir <http://localhost:8000>.

Precisa de internet na primeira carga: a biblioteca 3D e as fontes vêm de CDN.

## Estrutura

```
index.html    a página inteira (HTML, CSS, JS e o modelo 3D)
img/          a logo e as fotos do protótipo
```
