
#pragma once

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

class MotorTUI {
private:
  struct termios orig_termios;
  bool terminalModificado;
  std::string bufferQuadro;
  int largura;
  int altura;

public:
  MotorTUI() : terminalModificado(false), largura(80), altura(24) {
    atualizarDimensoes();
    ativarRawMode();
  }

  ~MotorTUI() {
    restaurarTerminal();
  }

  void atualizarDimensoes() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0 && w.ws_row > 0) {
      largura = w.ws_col;
      altura = w.ws_row;
    }
  }

  void ativarRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    // Ativa Alternate Screen Buffer (?1049h) e esconde cursor (?25l)
    std::string initSeq = "\033[?1049h\033[?25l\033[2J";
    write(STDOUT_FILENO, initSeq.c_str(), initSeq.size());
    terminalModificado = true;
  }

  void restaurarTerminal() {
    if (terminalModificado) {
      // Restaura cursor (?25h) e volta para a tela normal do terminal (?1049l)
      std::string endSeq = "\033[0m\033[?25h\033[?1049l";
      write(STDOUT_FILENO, endSeq.c_str(), endSeq.size());

      tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
      terminalModificado = false;
    }
  }

  int getLargura() const { return largura; }
  int getAltura() const { return altura; }

  char lerTecla() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
      if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
          if (seq[0] == '[') {
            if (seq[1] == 'A') return 'U'; // Cima
            if (seq[1] == 'B') return 'D'; // Baixo
            if (seq[1] == 'C') return 'R'; // Direita
            if (seq[1] == 'D') return 'L'; // Esquerda
          }
        }
        return 27; // ESC
      }
      return c;
    }
    return 0;
  }

  // Inicia um novo frame limpando o buffer na RAM
  void iniciarQuadro() {
    atualizarDimensoes();
    bufferQuadro.clear();
    bufferQuadro += "\033[2J\033[H"; // Limpa tela e move cursor para (1,1)
  }

  // Define cor RGB TrueColor (24-bit) para o texto
  void definirCorRGB(int r, int g, int b) {
    bufferQuadro += "\033[38;2;" + std::to_string(r) + ";" +
                    std::to_string(g) + ";" + std::to_string(b) + "m";
  }

  void resetarCor() {
    bufferQuadro += "\033[0m";
  }

  // Move o cursor (base 1) e escreve texto no buffer
  void escrever(int x, int y, const std::string& texto) {
    if (x < 1 || y < 1 || y > altura) return;
    bufferQuadro += "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H" + texto;
  }

  // Desenha caixa com caracteres ASCII/Box-Drawing
  void desenharCaixa(int x, int y, int w, int h, int r, int g, int b) {
    definirCorRGB(r, g, b);
    std::string topo = "+" + std::string(w - 2, '-') + "+";
    std::string meio = "|" + std::string(w - 2, ' ') + "|";

    escrever(x, y, topo);
    for (int i = 1; i < h - 1; ++i) {
      escrever(x, y + i, meio);
    }
    escrever(x, y + h - 1, topo);
    resetarCor();
  }

  // Desenha barra de HP/Mana em caracteres
  void desenharBarra(int x, int y, int larguraBarra, int atual, int maximo, int r, int g, int b) {
    int preenchido = 0;
    if (maximo > 0 && atual > 0) {
      preenchido = (atual * larguraBarra) / maximo;
    }
    int vazio = larguraBarra - preenchido;

    escrever(x, y, "[");
    definirCorRGB(r, g, b);
    bufferQuadro += std::string(preenchido, '#');
    definirCorRGB(80, 80, 90);
    bufferQuadro += std::string(vazio, '.');
    resetarCor();
    bufferQuadro += "]";
  }

  // Desenha arte ASCII linha por linha
  void desenharAsciiArt(int x, int y, const std::vector<std::string>& arte, int r, int g, int b) {
    definirCorRGB(r, g, b);
    for (size_t i = 0; i < arte.size(); ++i) {
      escrever(x, y + i, arte[i]);
    }
    resetarCor();
  }

  // Despeja o quadro inteiro no terminal em uma unica chamada de sistema (Double Buffering)
  void renderizar() {
    write(STDOUT_FILENO, bufferQuadro.c_str(), bufferQuadro.size());
  }
};
