# Trabalho 2 (PL4 + PP4) — Computação Gráfica — UEMS

**Aluno:** Victor Rech Vendruscolo (nº 12 da lista).
**Programa:** `brinquedoteca.cpp` (versão 3), em C++ com OpenGL de pipeline fixo, GLU e GLUT (freeglut).
**Slides:** `PL4_PP4_Victor_Rech_Vendruscolo_v3.pptx` (e o `.pdf` com o mesmo nome).

Este documento explica **o que foi feito** e **como o programa funciona**, rotina por rotina.
No fim, a tabela de rastreabilidade mostra de onde veio cada recurso.

---

## 1. O que o enunciado pedia

A PL4 pede uma proposta com cinco itens, e a PP4 pede a implementação dessa proposta:

| Item | Onde está no programa |
|---|---|
| 1. Cenário com modelos 3D | sala, barraca, carrinho, amarelinha e brinquedos: objetos da GLUT, quádricas da GLU e malhas próprias |
| 2. Animação | ronda do carrinho num trajeto de curvas de Bézier, com faróis e sirene; cabeça do ursinho |
| 3. Z-buffer | `GLUT_DEPTH`, `glEnable(GL_DEPTH_TEST)` e limpeza do buffer de profundidade a cada quadro |
| 4. Iluminação | 6 luzes (sala, janela, 2 faróis, 2 da sirene), materiais, brilho especular e emissão |
| 5. Textura (pesquisa) | madeira, listras e bolinhas, geradas no código (Guha cap. 12) |

A professora pediu uma **brinquedoteca** e autorizou a ideia sem ajustes.

---

## 2. Como usar (interação)

| Tecla | O que faz |
|---|---|
| Espaço | pausa ou continua a ronda; quando ela termina, começa outra |
| ← → | giram a câmera em volta da sala |
| ↑ ↓ | sobem ou descem a câmera |
| 1 | liga ou desliga a luz da sala (`GL_LIGHT0`) |
| 2 | liga ou desliga a luz da janela (`GL_LIGHT1`) |
| 3 | liga ou desliga os faróis (`GL_LIGHT2` e `GL_LIGHT3`) |
| 4 | liga ou desliga a sirene (`GL_LIGHT4` e `GL_LIGHT5`) |
| z | liga ou desliga o z-buffer |
| t | liga ou desliga as texturas |
| Esc | sai |

Roteiro de demonstração sugerido: deixar a ronda andar uma vez com tudo ligado; apertar `1` para
mostrar o sol da janela; apertar `2` para ficar de noite (faróis, sirene e olhos do ursinho);
apertar `z` e `t` para mostrar o erro sem z-buffer e a cena sem textura.

---

## 3. A cena

- **Sala:** piso de 12 × 12 (x e z de −6 a 6), parede do fundo (z = −6) e parede da esquerda (x = −6), com 5 de altura.
  A câmera fica no lado aberto (x e z positivos) e olha para a quina.
- **Janela redonda** na parede da esquerda, centro (−5,97; 2,9; 3,0) e raio 0,85.
- **Luminária de teto** no centro, com a lâmpada em y = 4,38.
- **Barraca** no canto do fundo: centro (−3,4; 0; −3,2), porta virada para +x.
- **Amarelinha** começando em (2,4; 0; 2,4) e girada 45° para a quina.
- **Brinquedos:** blocos (4,4; 0; −4,9), torre (0,4; 0; −5,0), patinho (5,2; 0; 0,2),
  ursinho (−4,7; 0; 0,5) e bola (−4,6; 0,6; 4,4).
- **Vaga do carrinho:** (3,6; 0; 3,6), apontando para o fundo.

---

## 4. Como funciona por dentro, rotina por rotina

### 4.1 Estrutura (igual aos códigos da aula)

- `main`: cria a janela com `GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH`, chama `setup` e registra as callbacks
  (`drawScene`, `resize`, `keyInput`, `specialKeyInput` e o timer `animate`). Depois entra no `glutMainLoop`.
- `setup`: roda uma vez. Liga o z-buffer e a iluminação, configura as 6 luzes, o `GL_COLOR_MATERIAL`,
  o `GL_NORMALIZE`, cria a quádrica e as texturas.
- `resize`: `glViewport` e `gluPerspective(60, w/h, 1, 100)`.
- `animate`: o timer. A cada 20 ms chama `atualizarRonda`, gira a sirene, conta os quadros
  (pisca da sirene), mexe a cabeça do ursinho e se agenda de novo com `glutTimerFunc`.

### 4.2 `drawScene` — um quadro

1. Limpa a cor e o z-buffer (`glClear`).
2. Posiciona a câmera com `gluLookAt`. O olho gira em volta do eixo y:
   `olhoX = distCamera · sin(ângulo)`, `olhoZ = distCamera · cos(ângulo)`.
3. Decide as luzes do carrinho: os faróis apagam dentro da barraca (fase 1) e a sirene só funciona
   durante a ronda (fases 0 e 2).
4. Envia as posições das luzes **depois** do `gluLookAt`. A posição passa pela matriz modelview do
   momento: assim a luz da sala e a da janela ficam presas à sala, e não à câmera.
5. Chama `posicionarLuzesDoCarrinho` **antes** de desenhar, para os faróis já iluminarem este quadro.
6. Desenha a sala, a amarelinha e a barraca sem brilho; depois a luminária e os brinquedos com brilho.
7. `glutSwapBuffers`: buffer duplo, mostra o quadro pronto.

### 4.3 Malha, piso e paredes

- `desenharMalha(largura, altura, n, repS, repT)`: plano no xy com n × n quadradinhos, em faixas de
  triângulos (`GL_TRIANGLE_STRIP`, mesmo padrão do `hemisphere.cpp`). Uma só normal (0, 0, 1),
  porque o plano é reto. Cada vértice recebe uma coordenada de textura (`glTexCoord2f`).
- **Por que uma malha fina?** A luz é calculada nos vértices e interpolada (`GL_SMOOTH`). Num piso de
  4 vértices, o círculo de um spot não apareceria.
- `desenharPiso`: a malha 12 × 12 com 48 divisões, deitada com `glRotatef(-90, 1, 0, 0)`.
  A madeira se repete 4 vezes.
- `desenharParedes`: a mesma malha (12 × 5) duas vezes. A da esquerda gira 90° em y.
  As bolinhas se repetem 6 vezes na largura e 2,5 na altura, para ficarem redondas.

### 4.4 Janela e luminária

- `desenharJanela`: o vidro é um `gluDisk` com cor preta e **emissão** (azul-claro de dia, azul-escuro
  quando a luz da janela está desligada). A moldura é um `glutSolidTorus`.
  Os dois nascem virados para +z; `glRotatef(90, 0, 1, 0)` os vira para +x.
- `desenharLuminaria`: fio (linha sem iluminação), cúpula (`glutSolidCone` com a ponta para cima) e
  lâmpada (`glutSolidSphere` emissiva só com a luz da sala ligada).

### 4.5 Barraca

- `desenharBarraca`: a parede é `gluCylinder(q, 1.6, 1.6, 1.5, 32, 6)` em pé; o teto é
  `gluCylinder(q, 1.85, 0.0, 1.4, 32, 4)` (raio de cima 0 = cone). Os dois usam a textura de listras,
  com as coordenadas geradas por `gluQuadricTexture`.
- **Porta sempre aberta:** um retângulo escuro de 0,9 × 0,9 com um meio disco em cima
  (`gluPartialDisk` de −90° a 90°). Ela fica um pouco à frente da parede (raio 1,64).
- **Como o carrinho "entra":** a porta escura está na frente da parte do carrinho que já passou dela.
  O z-buffer esconde essa parte. Quando o carrinho está todo dentro, a parede e o teto o escondem.

### 4.6 Amarelinha

- `casaAmarelinha`: um quadrado de 0,7 (malha 4 × 4) deitado no chão, um pouco acima do piso (y = 0,01)
  para não brigar no z-buffer. Borda com `GL_LINE_LOOP` e número com `glutStrokeCharacter`
  (fonte de traço, como no `Fonts.cpp`). A escala 0,003 deixa o número com 0,3 de altura.
- `desenharAmarelinha`: translação até o começo, rotação de 45° e 8 casas: 1, 2, 3, 4|5, 6, 7|8.
  O céu é um `gluPartialDisk` de meio círculo.

### 4.7 A ronda do carrinho (a parte mais nova)

**Trajeto:** duas tabelas, `ida[4][8]` e `volta[4][8]`. Cada linha é um trecho de **Bézier cúbica**
com 4 pontos de controle (x, z). Nas emendas, o último ponto de um trecho é o primeiro do próximo,
e os pontos vizinhos ficam alinhados: a curva não faz quina.

**`bezier(c, t, ...)`** calcula o ponto e a tangente:

- Ponto: `P(t) = (1−t)³B₀ + 3t(1−t)²B₁ + 3t²(1−t)B₂ + t³B₃` (pesos de Bernstein, que somam 1).
- Tangente (derivada): `P'(t) = 3[(1−t)²(B₁−B₀) + 2t(1−t)(B₂−B₁) + t²(B₃−B₂)]`.

**`pontoDoTrajeto(u, ...)`**: `u` vai de 0 a 4. A parte inteira escolhe o trecho; a parte decimal é o `t`.
Na fase 0 usa a tabela `ida`; na fase 2, a tabela `volta`.

**`atualizarRonda()`** (chamada pelo timer):

| Fase | O que acontece |
|---|---|
| 0 — ida | anda da vaga até o centro da barraca, passando pelo patinho, blocos e torre |
| 1 — na barraca | espera 100 quadros (2 s); os faróis e a sirene ficam apagados; o ursinho vira a cabeça |
| 2 — volta | sai da barraca e volta para a vaga, passando pelo ursinho e pela bola |
| 3 — fim | parado na vaga; o espaço começa outra ronda |

- **Velocidade constante:** o tamanho da tangente diz quanto a curva anda quando `u` anda 1.
  Por isso `u += 0.05 / |P'(t)|`: o carrinho anda 0,05 por quadro em qualquer trecho.
- **Direção:** `carroAng = atan2(-dz, dx) · 180/π`. O `glRotatef(carroAng, 0, 1, 0)` leva o +x do carrinho
  (a frente) para a direção (dx, dz). O sinal de `dz` é negativo porque girar +x em volta de y por um
  ângulo a dá (cos a, 0, −sen a).
- **Meia-volta escondida:** a ida termina no centro da barraca virada para −x; a volta começa no mesmo
  ponto virada para +x. A troca acontece dentro da barraca, sem ninguém ver.
- **Rodas:** `distAndada` soma a distância de cada quadro. Rolar sem deslizar:
  giro da roda (graus) = −(distAndada / raio da roda) · 180/π.

### 4.8 Carrinho, faróis e sirene

- `posicionarCarrinho`: `glTranslatef(carroX, 0, carroZ)` e `glRotatef(carroAng, 0, 1, 0)`.
- `desenharCarrinho`: sobe o raio da roda; carroceria e cabine são cubos escalados (`caixa`);
  faróis e cúpula da sirene são esferas com emissão; 4 rodas, cada uma no seu `glPushMatrix`.
- `desenharRoda`: toro + 5 parafusos a cada 72° (o mesmo laço do `Carro2.0.cpp`).
- `posicionarLuzesDoCarrinho`: aplica `posicionarCarrinho` e então chama `glLightfv` para os faróis.
  Depois sobe até a cabine, gira `anguloSirene` em y e define as duas luzes da sirene.
  **Como `glLightfv` usa a modelview do momento, as luzes seguem o carrinho** (ideia do `movelight.c`
  e da lanterna na mão do robô da lista L42).

### 4.9 Brinquedos

- **Padrão de todos:** cada peça entre `glPushMatrix` e `glPopMatrix`, com `glTranslatef`, `glRotatef` e
  `glScalef`, e um objeto Solid da GLUT. Peças simétricas usam `for (lado = -1; lado <= 1; lado += 2)`.
- **Ursinho:** esferas escaladas. A cabeça tem um `glRotatef(anguloCabeca, 0, 1, 0)` próprio, dentro da
  hierarquia do corpo. Os olhos ganham emissão azul-clara quando a luz da sala está apagada.
  Pelúcia não brilha: `brilho(0)` antes e `brilho(1)` depois.
- **Patinho:** esferas escaladas e bico de cone.
- **Torre de argolas:** base e haste com `gluCylinder` e `gluDisk`; 5 toros deitados, cada um menor.
- **Blocos:** 4 cubos.
- **Bola:** `gluSphere` com a textura de listras (vira gomos na esfera).

### 4.10 Iluminação

| Luz | Tipo | Valores principais |
|---|---|---|
| `GL_LIGHT0` sala | pontual (w = 1), sem cone | posição (0; 4,3; 0); difusa (0,85; 0,82; 0,75); ambiente própria 0,25 |
| `GL_LIGHT1` janela | spot longe | posição (−13,64; 11,02; 13,03); direção (1,6; −1,7; −2,1); cone 3,3°; expoente 0 |
| `GL_LIGHT2` e `3` faróis | spots presos ao carrinho | cone 25°; expoente 8; atenuação linear 0,1 e quadrática 0,1 |
| `GL_LIGHT4` e `5` sirene | spots vermelho e azul girando | cone 30°; expoente 5; atenuação linear 0,1 e quadrática 0,06 |

- **Luz ambiente global:** 0,15 (`glLightModelfv`). Com todas as luzes apagadas, sobra só ela e a emissão.
- **Por que a janela fica longe?** Com a fonte a 15 unidades e um cone de 3,3°, o cone tem na janela o
  raio do vidro (tan 3,3° · 15 ≈ 0,85). Os raios chegam quase paralelos, como o sol, e a mancha no chão
  tem o formato da janela (esticada pela inclinação).
- **Atenuação:** fator = 1 / (kc + kl·d + kq·d²). Os faróis iluminam bem o que está perto e pouco a parede longe.
- **Materiais:** `GL_COLOR_MATERIAL` faz o `glColor3f` virar cor ambiente e difusa. `brilho(1)` põe
  especular 0,8 e shininess 50 (plástico); `brilho(0)` tira o especular (piso, paredes, barraca, pelúcia).
- **Emissão:** lâmpada, vidro da janela, faróis, cúpula da sirene e olhos do ursinho. A emissão não
  ilumina os outros objetos: só faz o próprio objeto parecer aceso.
- **`GL_NORMALIZE`:** `glScalef` muda o tamanho das normais; o OpenGL volta elas para tamanho 1.
- **Limitação:** o pipeline fixo não calcula sombras. A luz atravessa os objetos: o farol que aponta
  para a barraca acende um pouco a parede atrás dela.

### 4.11 Textura (pesquisa própria)

Passos (de `loadTextures.cpp`, Guha cap. 12):

1. Gerar a imagem num vetor `unsigned char [64][64][3]`.
2. `glGenTextures(3, textura)` cria os ids; `glBindTexture` ativa um deles.
3. `glTexParameteri`: `GL_REPEAT` em s e t; filtro `GL_NEAREST` (madeira e listras) ou `GL_LINEAR` (bolinhas).
4. `glTexImage2D` envia a imagem.
5. Aplicar: `glTexCoord2f` nas malhas; `gluQuadricTexture(q, GL_TRUE)` nas quádricas.
   `glTexEnvf(..., GL_MODULATE)` multiplica a textura pela cor iluminada.

| Imagem | Fórmula | Onde |
|---|---|---|
| Madeira | `tabua = i / 16`; linha escura se `i % 16 == 0`; emenda se `(j + 24·tabua) % 64 == 0` | piso |
| Listras | `c = (j / 8) % 4` (8 faixas em 4 cores) | barraca e bola |
| Bolinhas | `di = i % 32 − 16`, `dj = j % 32 − 16`; bolinha se `di² + dj² ≤ 100` | paredes |

- Madeira adaptada de `createChessboard` (Exp. 12.1); listras adaptadas de `createStripes` (Exp. 12.16).
- Nas quádricas, a coordenada s dá a volta em torno do eixo: no cilindro as listras ficam em pé; na esfera viram gomos.
- `ligarTextura` liga `GL_TEXTURE_2D` só para os objetos texturizados; depois o código desliga.

---

## 5. Como compilar e executar

```bash
cd Computacao_Grafica/Trabalho2_CG
g++ -g brinquedoteca.cpp -o output/brinquedoteca -lGL -lGLU -lglut
./output/brinquedoteca
```

No VS Code, a tarefa do `tasks.json` faz o mesmo (F6).

---

## 6. Tabela de rastreabilidade

| Recurso | Trecho do código | Onde apareceu na aula |
|---|---|---|
| Estrutura `setup`/`drawScene`/`resize`/`keyInput`/timer | `main`, `animate` | `rotatingHelix3.cpp`, `Clown.cpp` (Guha cap. 4), `spots.c` |
| Buffer duplo | `GLUT_DOUBLE`, `glutSwapBuffers` | slides de animação (`CG-animacao.pdf`) |
| Câmera | `gluLookAt` em `drawScene`; `gluPerspective` em `resize` | slides de animação e câmera; códigos de iluminação |
| Objetos Solid da GLUT | `caixa`, `desenharRoda`, brinquedos | `glutObjects.cpp` (Guha cap. 10), `spots.c` |
| Quádricas da GLU | `desenharBarraca`, `desenharTorreArgolas`, `desenharBola`, janela | `gluQuadrics.cpp`, `torpedo.cpp` (Guha cap. 10) |
| `gluPartialDisk` | porta da barraca, céu da amarelinha | `torpedo.cpp` (barbatanas) |
| Malha em faixas de triângulos | `desenharMalha` | `hemisphere.cpp` (cap. 3), `checkeredFloor.cpp` (cap. 11) |
| Hierarquia (`glPushMatrix`/`glPopMatrix`) | carrinho, rodas, ursinho | `Carro2.0.cpp`, `Planet.cpp`, `Robot.cpp` (exercícios da aula) |
| Curvas de Bézier (Bernstein) | `bezier` | aula de modelagem, `superficies.cpp` |
| Tangente da Bézier | `bezier` (dx, dz) | `bezierCurveTangent.cpp` (Guha cap. 10) |
| `atan2` | `atualizarRonda` | trigonometria (Guha cap. 4), já usada na PP2 |
| Fonte de traço | `casaAmarelinha` | `Fonts.cpp` (Guha cap. 3) |
| Z-buffer | `GLUT_DEPTH`, `GL_DEPTH_TEST`, `glClear` | `triangles.c`, `planes.c`, slides de visibilidade |
| Luz pontual × direcional (w) | posições das luzes | `plane.c` |
| Spots (cutoff, direção, expoente) | `setup`, `posicionarLuzesDoCarrinho` | `spots.c`, lista L42 |
| Luz presa a um objeto | `posicionarLuzesDoCarrinho` | `movelight.c`, lista L42 (lanterna do robô) |
| Atenuação | faróis e sirene em `setup` | lista L42; experimentos de atenuação (Guha cap. 11) |
| Materiais, especular e emissão | `brilho`, janela, lâmpada, faróis, olhos | `light.c`, `material.c` |
| `GL_COLOR_MATERIAL` | `setup` | `colormat.c`, `spots.c` |
| `GL_NORMALIZE` | `setup` | `lightlab.c`, `teapots.c`, Exp. 11.25 |
| Aramado (só para os slides) | `glPolygonMode(GL_LINE)` | slides de OpenGL (modo de desenho) |
| **Textura** | `criarMadeira`, `criarListras`, `criarBolinhas`, `enviarTextura`, `ligarTextura` | **pesquisa própria:** Guha cap. 12, Experimenter p. 87–94 (`loadTextures.cpp`, `texturedTorpedo.cpp`, `fieldAndSkyLit.cpp`) |
