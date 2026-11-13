#include <iostream>
#include <string>

// Declaracao das funcoes que estao nos outros arquivos
int executarModoFB();
int executarModoTUI();

int main(int argc, char *argv[]) {
  // Permite escolher direto pelo terminal: ./minirpg --fb ou ./minirpg --tui
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "--fb")
      return executarModoFB();
    if (arg == "--tui")
      return executarModoTUI();
  }

  std::cout << "==========================================\n";
  std::cout << "      MINI-RPG BARE-METAL (LAUNCHER)      \n";
  std::cout << "==========================================\n";
  std::cout << "1. Modo TUI Bare-Metal (Roda em qualquer terminal)\n";
  std::cout << "2. Modo Framebuffer /dev/fb0 (TTY Puro - 60 FPS)\n";
  std::cout << "0. Sair\n";
  std::cout << "Escolha o motor grafico: ";

  int escolha;
  if (!(std::cin >> escolha))
    return 0;

  if (escolha == 1) {
    return executarModoTUI();
  } else if (escolha == 2) {
    return executarModoFB();
  }

  return 0;
}
