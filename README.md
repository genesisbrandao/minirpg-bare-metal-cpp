
# Mini-RPG Bare-Metal em C++ (Framebuffer & TUI)

Um RPG de combate por turnos desenvolvido em **C++17 puro** para Linux, projetado sob a filosofia *bare-metal* — **zero bibliotecas externas** (sem SDL, Raylib, OpenGL ou ncurses). 

O projeto conta com um **Launcher Unificado** que permite ao jogador escolher entre dois motores gráficos independentes compartilhando o mesmo núcleo lógico:
1. **Modo Framebuffer (`/dev/fb0`):** Renderização pixel-art a ~60 FPS escrevendo diretamente na memória de vídeo do kernel Linux em um terminal TTY puro.
2. **Modo TUI (Text User Interface):** Interface colorida em ASCII Art utilizando *ANSI Escape Codes* e *Alternate Screen Buffer*, compatível com qualquer emulador de terminal moderno.

---

## Arquitetura e Engenharia de Software

O projeto foi estruturado aplicando o **Princípio da Responsabilidade Única** e **Desacoplamento** entre regras de negócio e camada de apresentação:

* **`entidades.hpp` (Core Logic):** Toda a lógica do RPG utilizando Programação Orientada a Objetos (herança, polimorfismo com métodos virtuais, *Smart Pointers* `std::unique_ptr`, gerenciamento de inventário via `std::map` e tratamento de exceções). Totalmente independente de `std::cout` ou chamadas gráficas.
* **`motor_grafico.hpp` (Driver Framebuffer):**
  * Mapeamento direto da VRAM via chamadas de sistema `open()`, `ioctl()` e `mmap()`.
  * **Double Buffering:** Composição de quadros em um `std::vector<uint32_t>` na RAM e transferência única via `std::memcpy` para eliminar *flickering* e *tearing*.
  * **Fonte Bitmap 8x8 embutida:** Renderizador de texto feito do zero com operações *bitwise* sobre matrizes de bytes, sem depender de fontes do sistema.
  * **Renderizador de Sprites:** Decodificação de matrizes de caracteres para paletas hexadecimais (`0x00RRGGBB`) com suporte a escala e espelhamento.
* **`motor_tui.hpp` (Driver Terminal ANSI):**
  * Controle de cursor, cores *TrueColor* (24-bit RGB) e *Alternate Screen Buffer* (`\033[?1049h`) via sequências de escape ANSI.
  * **Double Buffering em Texto:** Acúmulo de todo o quadro em um único buffer de string antes de invocar `write()`, garantindo animações fluidas sem cintilação.
* **Input sem bloqueio (*Raw Mode*):** Configuração de baixo nível via `<termios.h>` (desativando `ECHO` e `ICANON`) e `<fcntl.h>` (`O_NONBLOCK`) em ambos os motores, capturando setas direcionais e teclas instantaneamente com restauração automática do terminal ao sair.

---


## 📸 Capturas de Tela

| Modo Gráfico (`/dev/fb0` no TTY) | Modo TUI Bare-Metal (ANSI) |
| :---: | :---: |
| ![Menu Framebuffer](assets/fb_menu.png) | ![Menu TUI](assets/tui_menu.png) |
| ![Batalha Framebuffer](assets/fb_batalha.png) | ![Batalha TUI](assets/tui_batalha.png) |

## Recursos do Jogo

* **Classes Jogáveis:**
  * **Guerreiro:** Alto HP (120) e ataque físico consistente (22 ATK).
  * **Mago:** HP reduzido (90), reserva de Mana (50 MP) e acesso às magias *Bola de Fogo* (projétil animado) e *Curar* (partículas visuais).
* **Modos de Jogo:**
  * **Campanha:** Enfrente o *Orc Guerreiro*, evolua seu personagem (**Level Up** com aumento de atributos e reabastecimento de mochila) e desafie o chefe final *Dragão Negro*.
  * **Batalha Rápida / Desafio Boss:** Combate direto contra o inimigo escolhido.
* **Efeitos Visuais:** Animações de investida, projéteis de magia, sistema de partículas de cura/mana e *Screen Shake* (tremor de tela) em golpes críticos.
* **Persistência:** Registro automático do histórico de vitórias e derrotas em `ranking.txt` com visualização integrada no menu inicial.

---

## Como Compilar e Executar

### Pré-requisitos
* Sistema operacional Linux
* Compilador `g++` (com suporte a C++17) e `make`

### Compilação
Na pasta raiz do projeto, execute:
```bash
make
```
Isso compilará os módulos separadamente (`.o`) e gerará o executável unificado **`minirpg`**.

### Execução
Você pode iniciar o menu interativo de seleção de motor:
```bash
./minirpg
```
Ou iniciar diretamente no modo desejado via argumentos:
```bash
./minirpg --tui   # Inicia direto no modo Terminal (ANSI)
./minirpg --fb    # Inicia direto no modo Gráfico (/dev/fb0)
```

> **Nota sobre o Modo Framebuffer (`/dev/fb0`):** Para rodar o modo gráfico sem interferência do servidor gráfico (X11/Wayland), alterne para um terminal TTY puro pressionando **`Ctrl + Alt + F3`** (use `Ctrl + Alt + F1` ou `F2` para voltar ao ambiente gráfico). Caso receba erro de permissão ao abrir `/dev/fb0`, execute com `sudo ./minirpg --fb` ou adicione seu usuário ao grupo `video` (`sudo usermod -aG video $USER`).

---

## Estrutura de Arquivos

```text
├── entidades.hpp       # Classes do RPG (Jogador, Mago, Inimigo, Chefao)
├── motor_grafico.hpp   # Engine 2D para /dev/fb0 e fonte Bitmap 8x8
├── motor_tui.hpp       # Engine TUI baseada em ANSI Escape Codes
├── main.cpp            # Loop principal e sprites do Modo Framebuffer
├── main_tui.cpp        # Loop principal e artes ASCII do Modo TUI
├── launcher.cpp        # Ponto de entrada e seletor de motor gráfico
├── Makefile            # Automação de compilação separada e linkagem
└── ranking.txt         # Histórico persistente de partidas (gerado ao jogar)
```
