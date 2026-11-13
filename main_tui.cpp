#include "entidades.hpp"
#include "motor_tui.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using ListaTextos = std::vector<std::string>;

// ================== SPRITES EM ASCII ART ==================
const ListaTextos ARTE_GUERREIRO = {"   .-.   ", "  (o.o) /", "  /|#|\\/ ",
                                    " / | |   ", "  /   \\  "};

const ListaTextos ARTE_MAGO = {"   /^\\   *", "  (o.o)  |", "  /|~|\\--+",
                               "  /   \\  |", " /_____\\ |"};

const ListaTextos ARTE_ORC = {"  ,---.  ", " ( >_< ) ", "\\/|###|\\/",
                              "  |___|  ", "  d   b  "};

const ListaTextos ARTE_DRAGAO = {"   /\\   /\\   ", "  ((o) (o))  ",
                                 " /==\\ V /==\\ ", "((  '---'  ))",
                                 " \\\\_______// "};

enum EstadoTUI { TUI_INICIAL, TUI_BATALHA, TUI_FIM };

int executarModoTUI() {
  srand(time(0));

  MotorTUI tui;
  EstadoTUI estado = TUI_INICIAL;

  std::string nomeHeroi = "ARTHAS";
  int classeIndex = 0;      // 0 = Guerreiro, 1 = Mago
  int dificuldadeIndex = 0; // 0 = Orc, 1 = Dragao
  int campoConfig = 0;

  std::unique_ptr<Jogador> heroi = nullptr;
  std::unique_ptr<Inimigo> monstro = nullptr;
  bool ehMago = false;
  int rodada = 1;
  int opcaoBatalha = 0;

  std::string log1 = "";
  std::string log2 = "";
  int animHeroiX = 0;
  int animMonstroX = 0;
  int shakeFrames = 0;
  bool fogoAtivo = false;
  int fogoX = 0;
  bool rankingSalvo = false;

  bool rodando = true;
  while (rodando) {
    char tecla = tui.lerTecla();
    if (tecla == 27)
      rodando = false;

    tui.iniciarQuadro();
    int W = tui.getLargura();
    int H = tui.getAltura();

    // =====================================================================
    // 1. TELA INICIAL TUI
    // =====================================================================
    if (estado == TUI_INICIAL) {
      if (tecla == 'U')
        campoConfig = (campoConfig - 1 + 4) % 4;
      else if (tecla == 'D')
        campoConfig = (campoConfig + 1) % 4;
      else if (campoConfig == 0) {
        if ((tecla == 127 || tecla == 8) && !nomeHeroi.empty())
          nomeHeroi.pop_back();
        else if (tecla >= 32 && tecla <= 122 && nomeHeroi.size() < 14)
          nomeHeroi.push_back(tecla);
      } else if (campoConfig == 1 &&
                 (tecla == 'L' || tecla == 'R' || tecla == '\n')) {
        classeIndex = 1 - classeIndex;
      } else if (campoConfig == 2 &&
                 (tecla == 'L' || tecla == 'R' || tecla == '\n')) {
        dificuldadeIndex = 1 - dificuldadeIndex;
      } else if (campoConfig == 3 && (tecla == '\n' || tecla == '\r')) {
        if (nomeHeroi.empty())
          nomeHeroi = "HEROI";

        if (classeIndex == 1) {
          heroi = std::make_unique<Mago>(nomeHeroi, 90, 10);
          ehMago = true;
        } else {
          heroi = std::make_unique<Jogador>(nomeHeroi, 120, 22);
          ehMago = false;
        }

        if (dificuldadeIndex == 1) {
          monstro = std::make_unique<Chefao>("DRAGAO NEGRO", 110, 10);
        } else {
          monstro = std::make_unique<Inimigo>("ORC GUERREIRO", 80, 11);
        }

        rodada = 1;
        opcaoBatalha = 0;
        rankingSalvo = false;
        log1 = "UM " + monstro->getTipo() + " BLOQUEIA O CAMINHO!";
        log2 = "USE AS SETAS E PRESSIONE ENTER.";
        estado = TUI_BATALHA;
      }

      int boxW = 64;
      int boxH = 18;
      int boxX = (W - boxW) / 2;
      int boxY = (H - boxH) / 2;

      tui.desenharCaixa(boxX, boxY, boxW, boxH, 255, 215, 0);
      tui.definirCorRGB(255, 215, 0);
      tui.escrever(boxX + 14, boxY + 2, "=== MINI-RPG: TUI BARE-METAL ===");

      tui.definirCorRGB(150, 160, 180);
      tui.escrever(boxX + 10, boxY + 4,
                   "Setas: Navegar | Enter: Confirmar | ESC: Sair");

      if (campoConfig == 0)
        tui.definirCorRGB(255, 215, 0);
      else
        tui.definirCorRGB(255, 255, 255);
      tui.escrever(boxX + 6, boxY + 7,
                   (campoConfig == 0 ? "> Nome: " : "  Nome: ") + nomeHeroi +
                       (campoConfig == 0 ? "_" : ""));

      if (campoConfig == 1)
        tui.definirCorRGB(255, 215, 0);
      else
        tui.definirCorRGB(255, 255, 255);
      std::string strClasse = (classeIndex == 0)
                                  ? "< Guerreiro (HP 120 / ATK 22) >"
                                  : "< Mago (HP 90 / MP 50 / ATK 10) >";
      tui.escrever(boxX + 6, boxY + 9,
                   (campoConfig == 1 ? "> Classe: " : "  Classe: ") +
                       strClasse);

      if (campoConfig == 2)
        tui.definirCorRGB(255, 215, 0);
      else
        tui.definirCorRGB(255, 255, 255);
      std::string strDif = (dificuldadeIndex == 0)
                               ? "< Normal: Orc Guerreiro >"
                               : "< Dificil: Dragao Negro (BOSS) >";
      tui.escrever(boxX + 6, boxY + 11,
                   (campoConfig == 2 ? "> Inimigo: " : "  Inimigo: ") + strDif);

      if (campoConfig == 3)
        tui.definirCorRGB(46, 196, 182);
      else
        tui.definirCorRGB(180, 180, 180);
      tui.escrever(
          boxX + 20, boxY + 14,
          (campoConfig == 3 ? "[ INICIAR BATALHA ]" : "  INICIAR BATALHA  "));
      tui.resetarCor();

      // =====================================================================
      // 2. ARENA DE BATALHA TUI
      // =====================================================================
    } else if (estado == TUI_BATALHA) {
      if (tecla == 'q' || tecla == 'Q')
        rodando = false;

      ListaTextos menu;
      if (ehMago) {
        Mago *m = static_cast<Mago *>(heroi.get());
        menu = {"1. Atacar Cajado",
                "2. Bola de Fogo (20 MP)",
                "3. Curar (15 MP)",
                "4. Pocao Vida (" +
                    std::to_string(heroi->qtdeItem("Pocao de Vida")) + ")",
                "5. Elixir Mana (" +
                    std::to_string(m->qtdeItem("Elixir de Mana")) + ")",
                "6. Defender"};
      } else {
        menu = {"1. Atacar Espada",
                "2. Pocao Vida (" +
                    std::to_string(heroi->qtdeItem("Pocao de Vida")) + ")",
                "3. Defender"};
      }

      if (opcaoBatalha >= (int)menu.size())
        opcaoBatalha = 0;

      if (tecla == 'U' || tecla == 'w' || tecla == 'W') {
        opcaoBatalha = (opcaoBatalha - 1 + menu.size()) % menu.size();
      } else if (tecla == 'D' || tecla == 's' || tecla == 'S') {
        opcaoBatalha = (opcaoBatalha + 1) % menu.size();
      } else if (tecla == '\n' || tecla == '\r') {
        try {
          int acao = opcaoBatalha + 1;
          if (!ehMago) {
            if (acao == 2)
              acao = 4;
            else if (acao == 3)
              acao = 6;
          }

          int danoHeroi = 0;
          bool defendeu = false;

          if (acao == 1) {
            danoHeroi = heroi->atacar();
            monstro->setVida(monstro->getVida() - danoHeroi);
            log1 = heroi->getNome() + " atacou: -" + std::to_string(danoHeroi) +
                   " HP!";
            animHeroiX = 6;
          } else if (acao == 2 && ehMago) {
            std::string msgMagia;
            int manaAntes = static_cast<Mago *>(heroi.get())->getMana();
            danoHeroi =
                static_cast<Mago *>(heroi.get())->lancarBolaDeFogo(msgMagia);
            monstro->setVida(monstro->getVida() - danoHeroi);
            log1 = msgMagia + " (-" + std::to_string(danoHeroi) + " HP)";
            if (manaAntes >= 20) {
              fogoAtivo = true;
              fogoX = 22;
            }
          } else if (acao == 3 && ehMago) {
            log1 = static_cast<Mago *>(heroi.get())->curar();
          } else if (acao == 4) {
            log1 = heroi->usarPocao();
          } else if (acao == 5 && ehMago) {
            log1 = static_cast<Mago *>(heroi.get())->usarElixir();
          } else {
            log1 = heroi->getNome() + " assumiu postura defensiva!";
            defendeu = true;
          }

          if (monstro->getVida() <= 0) {
            log2 = monstro->getTipo() + " foi derrotado!";
            estado = TUI_FIM;
          } else {
            bool critico = false;
            int danoInimigo = monstro->atacar(critico);
            if (defendeu)
              danoInimigo /= 2;

            heroi->setVida(heroi->getVida() - danoInimigo);
            animMonstroX = -6;

            if (critico) {
              log2 = "CRITICO! " + monstro->getTipo() + " causou -" +
                     std::to_string(danoInimigo) + " HP!";
              shakeFrames = 8;
            } else {
              log2 = monstro->getTipo() + " contra-atacou: -" +
                     std::to_string(danoInimigo) + " HP!";
            }

            if (heroi->getVida() <= 0)
              estado = TUI_FIM;
            else
              rodada++;
          }
        } catch (const std::exception &e) {
          log1 = std::string("AVISO: ") + e.what();
          log2 = "Escolha outra acao.";
        }
      }

      if (animHeroiX > 0)
        animHeroiX--;
      if (animMonstroX < 0)
        animMonstroX++;

      int shakeX = 0;
      if (shakeFrames > 0) {
        shakeX = (shakeFrames % 2 == 0) ? 2 : -2;
        shakeFrames--;
      }

      int arenaW = 76;
      int startX = std::max(2, (W - arenaW) / 2) + shakeX;

      tui.definirCorRGB(255, 215, 0);
      tui.escrever(startX, 1,
                   "RODADA " + std::to_string(rodada) +
                       "  |  MINI-RPG TUI BARE-METAL (ESC = SAIR)");

      // Caixa HUD Inimigo
      tui.desenharCaixa(startX, 2, 36, 5, 230, 57, 70);
      tui.definirCorRGB(255, 100, 100);
      tui.escrever(startX + 2, 3, monstro->getTipo());
      tui.resetarCor();
      tui.escrever(startX + 2, 4,
                   "HP: " + std::to_string(monstro->getVida()) + "/" +
                       std::to_string(monstro->getVidaMax()));
      tui.desenharBarra(startX + 2, 5, 28, monstro->getVida(),
                        monstro->getVidaMax(), 230, 57, 70);

      // Arte ASCII do Inimigo
      if (dificuldadeIndex == 1) {
        tui.desenharAsciiArt(startX + 52 + animMonstroX, 3, ARTE_DRAGAO, 230,
                             57, 70);
      } else {
        tui.desenharAsciiArt(startX + 54 + animMonstroX, 3, ARTE_ORC, 46, 196,
                             100);
      }

      // Arte ASCII do Heroi
      if (ehMago) {
        tui.desenharAsciiArt(startX + 8 + animHeroiX, 9, ARTE_MAGO, 76, 201,
                             240);
      } else {
        tui.desenharAsciiArt(startX + 8 + animHeroiX, 9, ARTE_GUERREIRO, 200,
                             210, 225);
      }

      // Projetil Bola de Fogo ASCII
      if (fogoAtivo) {
        tui.definirCorRGB(255, 140, 0);
        tui.escrever(startX + fogoX, 8, "==(***)>");
        tui.resetarCor();
        fogoX += 4;
        if (fogoX >= 50)
          fogoAtivo = false;
      }
      // Caixa HUD Heroi
      int hudHeroiX = startX + 38;
      tui.desenharCaixa(hudHeroiX, 9, 38, 6, 76, 201, 240);
      tui.definirCorRGB(76, 201, 240);
      tui.escrever(hudHeroiX + 2, 10,
                   heroi->getNome() + (ehMago ? " (Mago)" : " (Guerreiro)"));
      tui.resetarCor();
      tui.escrever(hudHeroiX + 2, 11,
                   "HP: " + std::to_string(heroi->getVida()) + "/" +
                       std::to_string(heroi->getVidaMax()));
      tui.desenharBarra(hudHeroiX + 14, 11, 20, heroi->getVida(),
                        heroi->getVidaMax(), 46, 196, 182);
      if (ehMago) {
        Mago *m = static_cast<Mago *>(heroi.get());
        tui.escrever(hudHeroiX + 2, 12,
                     "MP: " + std::to_string(m->getMana()) + "/" +
                         std::to_string(m->getManaMax()));
        tui.desenharBarra(hudHeroiX + 14, 12, 20, m->getMana(), m->getManaMax(),
                          58, 134, 255);
      }
      // Painel Inferior: Menu + Log
      tui.desenharCaixa(startX, 16, 32, 8, 255, 255, 255);
      for (size_t i = 0; i < menu.size(); ++i) {
        if ((int)i == opcaoBatalha) {
          tui.definirCorRGB(255, 215, 0);
          tui.escrever(startX + 2, 17 + i, "> " + menu[i]);
        } else {
          tui.definirCorRGB(200, 210, 220);
          tui.escrever(startX + 2, 17 + i, "  " + menu[i]);
        }
      }
      tui.resetarCor();
      tui.desenharCaixa(startX + 33, 16, 43, 8, 255, 255, 255);
      tui.definirCorRGB(148, 163, 184);
      tui.escrever(startX + 35, 17, "LOG DE COMBATE:");
      tui.definirCorRGB(255, 255, 255);
      tui.escrever(startX + 35, 19, log1);
      tui.definirCorRGB(250, 204, 21);
      tui.escrever(startX + 35, 21, log2);
      tui.resetarCor();
      // =====================================================================
      // 3. TELA DE FIM DE JOGO
      // =====================================================================
    } else if (estado == TUI_FIM) {
      bool venceu = (heroi->getVida() > 0);
      if (!rankingSalvo) {
        std::ofstream arq("ranking.txt", std::ios::app);
        if (arq.is_open()) {
          arq << "Jogador: " << heroi->getNome()
              << " | Classe: " << (ehMago ? "Mago" : "Guerreiro")
              << " | Modo: TUI"
              << " | Resultado: " << (venceu ? "VITORIA" : "GAME OVER")
              << std::endl;
          arq.close();
        }
        rankingSalvo = true;
      }
      if (tecla == '\n' || tecla == '\r')
        estado = TUI_INICIAL;
      else if (tecla == 'q' || tecla == 'Q')
        rodando = false;
      int boxW = 58;
      int boxH = 12;
      int boxX = (W - boxW) / 2;
      int boxY = (H - boxH) / 2;
      if (venceu)
        tui.desenharCaixa(boxX, boxY, boxW, boxH, 46, 196, 182);
      else
        tui.desenharCaixa(boxX, boxY, boxW, boxH, 230, 57, 70);
      if (venceu)
        tui.definirCorRGB(46, 196, 182);
      else
        tui.definirCorRGB(230, 57, 70);
      tui.escrever(boxX + 18, boxY + 2,
                   venceu ? "VITORIA GLORIOSA!" : "GAME OVER...");
      tui.definirCorRGB(255, 255, 255);
      tui.escrever(boxX + 4, boxY + 5, log1);
      tui.definirCorRGB(250, 204, 21);
      tui.escrever(boxX + 4, boxY + 7, log2);
      tui.definirCorRGB(150, 160, 180);
      tui.escrever(boxX + 8, boxY + 10,
                   "[ENTER] Jogar Novamente  |  [ESC] Sair");
      tui.resetarCor();
    }
    tui.renderizar();
    usleep(33333); // ~30 FPS
  }
  return 0;
}
