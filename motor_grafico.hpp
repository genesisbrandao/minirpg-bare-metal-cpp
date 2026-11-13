#pragma once

#include <algorithm> // Necessario para std::fill
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/fb.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

class MotorGrafico {
private:
  int fb_fd;
  uint32_t *fb_ptr;
  std::vector<uint32_t> backBuffer;
  int largura;
  int altura;
  int stridePixels;
  size_t tamanhoTelaBytes;
  struct termios orig_termios;
  bool terminalModificado;

  // Fonte Bitmap 8x8 (59 caracteres: ASCII 32 a 90)
  static const uint8_t *getGlyph(char c) {
    if (c >= 'a' && c <= 'z')
      c -= 32; // Converte minusculas para maiusculas
    static const uint8_t font[59][8] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
        {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // '!' (33)
        {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '"' (34)
        {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // '#' (35)
        {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // '$' (36)
        {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // '%' (37)
        {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // '&' (38)
        {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''' (39)
        {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // '(' (40)
        {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // ')' (41)
        {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // '*' (42)
        {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // '+' (43)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // ',' (44)
        {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // '-' (45)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // '.' (46)
        {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // '/' (47)
        {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // '0' (48)
        {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // '1' (49)
        {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // '2' (50)
        {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // '3' (51)
        {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // '4' (52)
        {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // '5' (53)
        {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // '6' (54)
        {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // '7' (55)
        {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // '8' (56)
        {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // '9' (57)
        {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // ':' (58)
        {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // ';' (59)
        {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // '<' (60)
        {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // '=' (61)
        {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // '>' (62)
        {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // '?' (63)
        {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // '@' (64)
        {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // 'A' (65)
        {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // 'B' (66)
        {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // 'C' (67)
        {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // 'D' (68)
        {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // 'E' (69)
        {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // 'F' (70)
        {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // 'G' (71)
        {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // 'H' (72)
        {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'I' (73)
        {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // 'J' (74)
        {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // 'K' (75)
        {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // 'L' (76)
        {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // 'M' (77)
        {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // 'N' (78)
        {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // 'O' (79)
        {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // 'P' (80)
        {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // 'Q' (81)
        {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // 'R' (82)
        {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // 'S' (83)
        {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'T' (84)
        {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // 'U' (85)
        {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'V' (86)
        {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 'W' (87)
        {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // 'X' (88)
        {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // 'Y' (89)
        {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}  // 'Z' (90)
    };
    if (c < 32 || c > 90)
      return font[0]; // Retorna espaco se fora do intervalo
    return font[c - 32];
  }

public:
  MotorGrafico() : fb_fd(-1), fb_ptr(nullptr), terminalModificado(false) {
    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
      throw std::runtime_error(
          "Erro ao abrir /dev/fb0. Execute no TTY puro ou com sudo!");
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
        ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
      close(fb_fd);
      throw std::runtime_error("Erro ao ler informacoes do Framebuffer.");
    }

    largura = vinfo.xres;
    altura = vinfo.yres;
    stridePixels = finfo.line_length / 4; // 32 bits (4 bytes) por pixel
    tamanhoTelaBytes = finfo.line_length * altura;

    fb_ptr = (uint32_t *)mmap(0, tamanhoTelaBytes, PROT_READ | PROT_WRITE,
                              MAP_SHARED, fb_fd, 0);
    if (fb_ptr == MAP_FAILED) {
      close(fb_fd);
      throw std::runtime_error("Erro no mmap do Framebuffer.");
    }

    backBuffer.resize(stridePixels * altura, 0x00000000);
    ativarRawMode();
  }

  ~MotorGrafico() {
    restaurarTerminal();
    if (fb_ptr && fb_ptr != MAP_FAILED) {
      munmap(fb_ptr, tamanhoTelaBytes);
    }
    if (fb_fd >= 0) {
      close(fb_fd);
    }
  }

  void ativarRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    std::cout << "\033[?25l" << std::flush; // Esconde cursor do TTY
    terminalModificado = true;
  }

  void restaurarTerminal() {
    if (terminalModificado) {
      tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
      std::cout << "\033[?25h\033[2J\033[H"
                << std::flush; // Restaura cursor e limpa TTY
      terminalModificado = false;
    }
  }

  int getLargura() const { return largura; }
  int getAltura() const { return altura; }

  // Le teclas sem bloquear (retorna 'U', 'D', 'L', 'R' para setas, ou o
  // caractere)
  char lerTecla() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
      if (c == '\033') { // Sequencia de escape (setas direcionais)
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) == 1 &&
            read(STDIN_FILENO, &seq[1], 1) == 1) {
          if (seq[0] == '[') {
            if (seq[1] == 'A')
              return 'U'; // Cima
            if (seq[1] == 'B')
              return 'D'; // Baixo
            if (seq[1] == 'C')
              return 'R'; // Direita
            if (seq[1] == 'D')
              return 'L'; // Esquerda
          }
        }
        return 27; // ESC
      }
      return c;
    }
    return 0;
  }

  void limparTela(uint32_t cor) {
    std::fill(backBuffer.begin(), backBuffer.end(), cor);
  }

  void desenharPixel(int x, int y, uint32_t cor) {
    if (x >= 0 && x < largura && y >= 0 && y < altura) {
      backBuffer[y * stridePixels + x] = cor;
    }
  }

  void desenharRetangulo(int x, int y, int w, int h, uint32_t cor) {
    for (int py = y; py < y + h; ++py) {
      for (int px = x; px < x + w; ++px) {
        desenharPixel(px, py, cor);
      }
    }
  }

  void desenharCaixaBorda(int x, int y, int w, int h, uint32_t corFundo,
                          uint32_t corBorda, int espessura = 3) {
    desenharRetangulo(x, y, w, h, corBorda);
    desenharRetangulo(x + espessura, y + espessura, w - espessura * 2,
                      h - espessura * 2, corFundo);
  }

  void desenharBarraStatus(int x, int y, int w, int h, int atual, int maximo,
                           uint32_t corCheia, uint32_t corVazia) {
    desenharCaixaBorda(x - 2, y - 2, w + 4, h + 4, corVazia, 0x00FFFFFF, 2);
    if (maximo > 0 && atual > 0) {
      int larguraPreenchida = (atual * w) / maximo;
      desenharRetangulo(x, y, larguraPreenchida, h, corCheia);
    }
  }

  // Renderizador de Texto Bitmap 8x8 com escala
  void desenharTexto(int x, int y, const std::string &texto, uint32_t cor,
                     int escala = 2) {
    int cursorX = x;
    for (char c : texto) {
      const uint8_t *glyph = getGlyph(c);
      for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
          if (glyph[row] & (1 << col)) {
            desenharRetangulo(cursorX + col * escala, y + row * escala, escala,
                              escala, cor);
          }
        }
      }
      cursorX += 8 * escala;
    }
  }

  // Renderizador de Sprites Pixel-Art baseados em matrizes de strings e paleta
  void desenharSprite(int x, int y, const std::vector<std::string> &sprite,
                      const uint32_t paleta[], int escala = 8,
                      bool inverterX = false) {
    for (size_t row = 0; row < sprite.size(); ++row) {
      size_t larguraLinha = sprite[row].size();
      for (size_t col = 0; col < larguraLinha; ++col) {
        char ch = sprite[row][col];
        if (ch == '.' || ch == ' ')
          continue; // Pixel transparente

        int indiceCor = -1;
        if (ch >= '0' && ch <= '9')
          indiceCor = ch - '0';
        else if (ch >= 'A' && ch <= 'F')
          indiceCor = 10 + (ch - 'A');

        if (indiceCor >= 0) {
          int drawCol = inverterX ? (larguraLinha - 1 - col) : col;
          desenharRetangulo(x + drawCol * escala, y + row * escala, escala,
                            escala, paleta[indiceCor]);
        }
      }
    }
  }

  void atualizarTela() {
    std::memcpy(fb_ptr, backBuffer.data(), tamanhoTelaBytes);
  }
};
