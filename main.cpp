#include "entidades.hpp"
#include "motor_grafico.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using ListaTextos = std::vector<std::string>;

// ================== PALETA DE CORES (0x00RRGGBB) ==================
const uint32_t PALETA_RPG[] = {
    0x00000000, // 0: Preto (contorno)
    0x00FFFFFF, // 1: Branco (brilho / olhos)
    0x00F1C27D, // 2: Pele
    0x0094A3B8, // 3: Aco claro (espada / armadura)
    0x00475569, // 4: Aco escuro
    0x002563EB, // 5: Azul real (manto mago / detalhes)
    0x0060A5FA, // 6: Azul claro (energia / magia)
    0x00D97706, // 7: Dourado / Madeira (cajado / elmo)
    0x00DC2626, // 8: Vermelho (capa / dragao / sangue)
    0x00991B1B, // 9: Vermelho escuro (escamas / sombra)
    0x0016A34A, // A: Verde Orc / Cura
    0x0014532D, // B: Verde escuro Orc
    0x0078350F, // C: Marrom couro / chifres
    0x00FACC15, // D: Amarelo fogo / olhos boss
    0x001E293B, // E: Cinza carvao (Dragao Negro)
    0x000F172A  // F: Preto azulado profundo
};

// ================== SPRITES PIXEL-ART 16x16 ==================
const std::vector<std::string> SPRITE_GUERREIRO = {
    "......0000......", ".....077770.....", "....03333330....",
    "....03100130..03", ".....022220..031", "....03333330.031",
    "...0833773380310", "..0803333330770.", "..080333333020..",
    "..08044444400...", ".....0440440....", ".....0440440....",
    ".....0330330....", "....0CC0.0CC0...", "................",
    "................"};

const std::vector<std::string> SPRITE_MAGO = {
    ".......00.......", "......0550......", ".....055550.....",
    "....05577550..06", "...0555555550061", ".....022220..070",
    ".....021210..070", "....05555550.070", "...0556666550070",
    "..05055555502070", "..0505555550.070", "....05555550.070",
    "....05555550.070", "....0CC00CC0.070", "................",
    "................"};

const std::vector<std::string> SPRITE_ORC = {
    ".....000000.....", "....0AAAAAA0....", "...0AA8AA8AA0...",
    "...0AAAAAAAA0...", "...0A10AA01A0...", "....0AAAAAA0....",
    "..000BBBBBB000..", ".0CC0BBBBBB0CC0.", "0CC00CCCCCC00CC0",
    "0A0.0CCCCCC0.0A0", "020.04444440.030", "....0BB00BB0.031",
    "....0BB00BB0.031", "...0CC0..0CC0030", "................",
    "................"};

const std::vector<std::string> SPRITE_DRAGAO = {
    "..00........00..", ".0CC0..00..0CC0.", ".0CC0.0EE0.0CC0.",
    "0880.0EEEE0.0880", "08980EDEEEDE0898", "08980EEEEEE00898",
    ".080.0E88E0..080", "....0EEEEEE0....", "...0EE9999EE0...",
    "..0EEE9DD9EEE0..", ".0EEEE9999EEEE0.", ".0EE00EEEE00EE0.",
    "0EE0.0EEEE0.0EE0", "....0EE00EE0....", "...0CC0..0CC0...",
    "................"};

// Sprite 8x8 para Bola de Fogo
const std::vector<std::string> SPRITE_BOLA_FOGO = {
    "..8888..", ".8DDDD8.", "8DD11DD8", "8D1111D8",
    "8D1111D8", "8DD11DD8", ".8DDDD8.", "..8888.."};

struct Particula {
  int x, y;
  int vy;
  int vida;
  uint32_t cor;
};

enum EstadoJogo { TELA_INICIAL, TELA_RANKING, LEVEL_UP, BATALHA, FIM_DE_JOGO };

std::vector<std::string> carregarRanking(int maxLinhas = 10) {
  std::vector<std::string> linhas;
  std::ifstream arq("ranking.txt");
  if (!arq.is_open())
    return linhas;

  std::string linha;
  while (std::getline(arq, linha)) {
    if (!linha.empty())
      linhas.push_back(linha);
  }
  if ((int)linhas.size() > maxLinhas) {
    linhas.erase(linhas.begin(), linhas.end() - maxLinhas);
  }
  std::reverse(linhas.begin(), linhas.end()); // Mais recentes primeiro
  return linhas;
}

int executarModoFB() {
  srand(time(0));

  try {
    MotorGrafico gfx;
    int W = gfx.getLargura();
    int H = gfx.getAltura();

    EstadoJogo estado = TELA_INICIAL;

    // Variaveis da Tela Inicial
    std::string nomeHeroi = "ARTHAS";
    int classeIndex = 0; // 0 = Guerreiro, 1 = Mago
    int modoJogoIndex =
        0; // 0 = Campanha (Orc -> Dragao), 1 = Rapida: Orc, 2 = Rapida: Dragao
    int campoConfig =
        0; // 0 = Nome, 1 = Classe, 2 = Modo, 3 = Iniciar, 4 = Ranking

    // Entidades e Estado da Batalha
    std::unique_ptr<Jogador> heroi = nullptr;
    std::unique_ptr<Inimigo> monstro = nullptr;
    bool ehMago = false;
    bool isBossAtual = false;
    int nivelHeroi = 1;
    int rodada = 1;
    int opcaoBatalha = 0;

    // Logs e Efeitos Visuais
    std::string logLinha1 = "";
    std::string logLinha2 = "";
    int animHeroiOffsetX = 0;
    int animMonstroOffsetX = 0;
    int flashHeroiFrames = 0;
    int flashMonstroFrames = 0;
    int screenShakeFrames = 0;

    // Projetil Bola de Fogo
    bool fogoAtivo = false;
    int fogoX = 0, fogoY = 0, fogoAlvoX = 0;

    // Particulas de Cura/Mana
    std::vector<Particula> particulas;
    std::vector<std::string> historicoRanking;
    bool rankingSalvo = false;

    auto spawnParticulasCura = [&](int baseX, int baseY, uint32_t cor) {
      for (int i = 0; i < 18; ++i) {
        Particula p;
        p.x = baseX + (rand() % 110);
        p.y = baseY + (rand() % 110);
        p.vy = -2 - (rand() % 3);
        p.vida = 25 + (rand() % 15);
        p.cor = cor;
        particulas.push_back(p);
      }
    };

    bool rodando = true;
    while (rodando) {
      char tecla = gfx.lerTecla();
      if (tecla == 27)
        rodando = false; // ESC sai a qualquer momento

      // =====================================================================
      // 1. TELA INICIAL
      // =====================================================================
      if (estado == TELA_INICIAL) {
        if (tecla == 'U') {
          campoConfig = (campoConfig - 1 + 5) % 5;
        } else if (tecla == 'D') {
          campoConfig = (campoConfig + 1) % 5;
        } else if (campoConfig == 0) {
          if ((tecla == 127 || tecla == 8) && !nomeHeroi.empty()) {
            nomeHeroi.pop_back();
          } else if (tecla >= 32 && tecla <= 122 && nomeHeroi.size() < 14) {
            nomeHeroi.push_back(tecla);
          }
        } else if (campoConfig == 1 &&
                   (tecla == 'L' || tecla == 'R' || tecla == '\n')) {
          classeIndex = 1 - classeIndex;
        } else if (campoConfig == 2 &&
                   (tecla == 'L' || tecla == 'R' || tecla == '\n')) {
          modoJogoIndex = (modoJogoIndex + 1) % 3;
        } else if (campoConfig == 3 && (tecla == '\n' || tecla == '\r')) {
          if (nomeHeroi.empty())
            nomeHeroi = "HEROI";
          nivelHeroi = 1;

          if (classeIndex == 1) {
            heroi = std::make_unique<Mago>(nomeHeroi, 90, 10);
            ehMago = true;
          } else {
            heroi = std::make_unique<Jogador>(nomeHeroi, 120, 22);
            ehMago = false;
          }

          if (modoJogoIndex == 2) {
            monstro = std::make_unique<Chefao>("DRAGAO NEGRO", 110, 10);
            isBossAtual = true;
          } else {
            monstro = std::make_unique<Inimigo>("ORC GUERREIRO", 80, 11);
            isBossAtual = false;
          }

          rodada = 1;
          opcaoBatalha = 0;
          rankingSalvo = false;
          particulas.clear();
          fogoAtivo = false;
          logLinha1 = "RODADA 1: " + monstro->getTipo() + " SURGIU!";
          logLinha2 = "ESCOLHA SUA ACAO NO MENU.";
          estado = BATALHA;
        } else if (campoConfig == 4 && (tecla == '\n' || tecla == '\r')) {
          historicoRanking = carregarRanking(10);
          estado = TELA_RANKING;
        }

        // Renderizacao da Tela Inicial
        gfx.limparTela(0x000F172A);
        int boxW = 700;
        int boxH = 490;
        int boxX = (W - boxW) / 2;
        int boxY = (H - boxH) / 2;

        gfx.desenharCaixaBorda(boxX, boxY, boxW, boxH, 0x001E293B, 0x00FFD700,
                               4);
        gfx.desenharTexto(boxX + 55, boxY + 28,
                          "MINI-RPG BARE-METAL FRAMEBUFFER", 0x00FFD700, 2);
        gfx.desenharTexto(boxX + 55, boxY + 55,
                          "SETAS: NAVEGAR | ENTER: SELECIONAR | ESC: SAIR",
                          0x0094A3B8, 1);

        // Campo 0: Nome
        uint32_t cor0 = (campoConfig == 0) ? 0x00FFD700 : 0x00FFFFFF;
        gfx.desenharTexto(boxX + 45, boxY + 95,
                          (campoConfig == 0 ? "> NOME: " : "  NOME: ") +
                              nomeHeroi + (campoConfig == 0 ? "_" : ""),
                          cor0, 2);

        // Campo 1: Classe
        uint32_t cor1 = (campoConfig == 1) ? 0x00FFD700 : 0x00FFFFFF;
        std::string txtClasse = (classeIndex == 0)
                                    ? "< GUERREIRO (HP 120 / ATK 22) >"
                                    : "< MAGO (HP 90 / MP 50 / ATK 10) >";
        gfx.desenharTexto(boxX + 45, boxY + 145,
                          (campoConfig == 1 ? "> CLASSE: " : "  CLASSE: "),
                          cor1, 2);
        gfx.desenharTexto(boxX + 65, boxY + 172, txtClasse, 0x004CC9F0, 2);

        gfx.desenharSprite(boxX + boxW - 130, boxY + 120,
                           (classeIndex == 0 ? SPRITE_GUERREIRO : SPRITE_MAGO),
                           PALETA_RPG, 6);

        // Campo 2: Modo de Jogo
        uint32_t cor2 = (campoConfig == 2) ? 0x00FFD700 : 0x00FFFFFF;
        std::string txtModo = "< CAMPANHA: ORC + DRAGAO >";
        if (modoJogoIndex == 1)
          txtModo = "< BATALHA RAPIDA: ORC GUERREIRO >";
        else if (modoJogoIndex == 2)
          txtModo = "< DESAFIO BOSS: DRAGAO NEGRO >";

        gfx.desenharTexto(boxX + 45, boxY + 225,
                          (campoConfig == 2 ? "> MODO: " : "  MODO: "), cor2,
                          2);
        gfx.desenharTexto(boxX + 65, boxY + 252, txtModo, 0x00FF5555, 2);

        // Campo 3: Botao Iniciar
        uint32_t cor3 = (campoConfig == 3) ? 0x0016A34A : 0x00334155;
        gfx.desenharCaixaBorda(boxX + 180, boxY + 315, 340, 52, cor3,
                               0x00FFFFFF, 2);
        gfx.desenharTexto(boxX + 225, boxY + 332, "INICIAR BATALHA", 0x00FFFFFF,
                          2);

        // Campo 4: Botao Ver Ranking
        uint32_t cor4 = (campoConfig == 4) ? 0x002563EB : 0x00334155;
        gfx.desenharCaixaBorda(boxX + 180, boxY + 385, 340, 52, cor4,
                               0x00FFFFFF, 2);
        gfx.desenharTexto(boxX + 245, boxY + 402, "VER RANKING", 0x00FFFFFF, 2);

        // =====================================================================
        // 2. TELA DE RANKING (LEITURA DO RANKING.TXT)
        // =====================================================================
      } else if (estado == TELA_RANKING) {

        if (tecla == '\n' || tecla == '\r' || tecla == 'q' || tecla == 'Q') {
          estado = TELA_INICIAL;
        }
        gfx.limparTela(0x000F172A);
        int boxW = 860;
        int boxH = 480;
        int boxX = (W - boxW) / 2;
        int boxY = (H - boxH) / 2;
        gfx.desenharCaixaBorda(boxX, boxY, boxW, boxH, 0x001E293B, 0x003A86FF,
                               4);
        gfx.desenharTexto(boxX + 240, boxY + 25, "HISTORICO DE BATALHAS",
                          0x00FFD700, 2);
        if (historicoRanking.empty()) {
          gfx.desenharTexto(boxX + 210, boxY + 200,
                            "NENHUMA BATALHA REGISTRADA AINDA.", 0x0094A3B8, 2);
        } else {
          for (size_t i = 0; i < historicoRanking.size(); ++i) {
            uint32_t corLinha =
                (historicoRanking[i].find("VITORIA") != std::string::npos)
                    ? 0x002EC4B6
                    : 0x00FF5555;
            gfx.desenharTexto(boxX + 35, boxY + 75 + (i * 32),
                              historicoRanking[i], corLinha, 1);
          }
        }
        gfx.desenharTexto(boxX + 250, boxY + boxH - 45,
                          "[ENTER] VOLTAR AO MENU", 0x00FFD700, 2);
        // =====================================================================
        // 3. TELA DE LEVEL UP (MODO CAMPANHA)
        // =====================================================================
      } else if (estado == LEVEL_UP) {
        if (tecla == '\n' || tecla == '\r') {
          // Evolui o heroi recriando com atributos maiores mantendo o nome e
          // classe
          nivelHeroi = 2;
          if (ehMago) {
            heroi = std::make_unique<Mago>(nomeHeroi, 115, 15);
          } else {
            heroi = std::make_unique<Jogador>(nomeHeroi, 150, 28);
          }
          monstro = std::make_unique<Chefao>("DRAGAO NEGRO", 110, 10);
          isBossAtual = true;
          rodada = 1;
          opcaoBatalha = 0;
          particulas.clear();
          fogoAtivo = false;
          logLinha1 = "CHEFAO FINAL: DRAGAO NEGRO DESCEU DOS CEUS!";
          logLinha2 = "PREPARE-SE PARA O COMBATE DECISIVO!";
          estado = BATALHA;
        }
        gfx.limparTela(0x000F172A);
        int boxW = 680;
        int boxH = 360;
        int boxX = (W - boxW) / 2;
        int boxY = (H - boxH) / 2;
        gfx.desenharCaixaBorda(boxX, boxY, boxW, boxH, 0x001E293B, 0x00FFD700,
                               4);
        gfx.desenharTexto(boxX + 180, boxY + 35, "LEVEL UP! NIVEL 2",
                          0x00FFD700, 3);
        gfx.desenharTexto(boxX + 55, boxY + 100,
                          "ORC GUERREIRO DERROTADO COM SUCESSO!", 0x002EC4B6,
                          2);
        gfx.desenharTexto(boxX + 55, boxY + 145,
                          "+ VIDA MAXIMA AUMENTADA E RESTAURADA", 0x00FFFFFF,
                          2);
        gfx.desenharTexto(boxX + 55, boxY + 180,
                          "+ FORCA DE ATAQUE AUMENTADA (+5 ATK)", 0x00FFFFFF,
                          2);
        gfx.desenharTexto(boxX + 55, boxY + 215,
                          "+ MOCHILA REABASTECIDA PARA O BOSS!", 0x004CC9F0, 2);
        gfx.desenharTexto(boxX + 110, boxY + 290,
                          "[ENTER] ENFRENTAR O DRAGAO NEGRO", 0x00FFD700, 2);
        // =====================================================================
        // 4. BATALHA POR TURNOS COM ANIMACOES E PARTICULAS
        // =====================================================================
      } else if (estado == BATALHA) {
        if (tecla == 'q' || tecla == 'Q')
          rodando = false;
        ListaTextos opcoesMenu;
        if (ehMago) {
          Mago *m = static_cast<Mago *>(heroi.get());
          opcoesMenu = {"1. ATACAR CAJADO",
                        "2. BOLA DE FOGO (20 MP)",
                        "3. CURAR (15 MP)",
                        "4. POCAO VIDA (" +
                            std::to_string(heroi->qtdeItem("Pocao de Vida")) +
                            ")",
                        "5. ELIXIR MANA (" +
                            std::to_string(m->qtdeItem("Elixir de Mana")) + ")",
                        "6. DEFENDER"};
        } else {
          opcoesMenu = {"1. ATACAR ESPADA",
                        "2. POCAO VIDA (" +
                            std::to_string(heroi->qtdeItem("Pocao de Vida")) +
                            ")",
                        "3. DEFENDER"};
        }
        if (opcaoBatalha >= (int)opcoesMenu.size())
          opcaoBatalha = 0;
        if (tecla == 'U' || tecla == 'w' || tecla == 'W') {
          opcaoBatalha =
              (opcaoBatalha - 1 + opcoesMenu.size()) % opcoesMenu.size();
        } else if (tecla == 'D' || tecla == 's' || tecla == 'S') {
          opcaoBatalha = (opcaoBatalha + 1) % opcoesMenu.size();
        } else if (tecla == '\n' || tecla == '\r') {
          try {
            int acaoMapeada = opcaoBatalha + 1;
            if (!ehMago) {
              if (acaoMapeada == 2)
                acaoMapeada = 4;
              else if (acaoMapeada == 3)
                acaoMapeada = 6;
            }
            int danoHeroi = 0;
            bool defendeu = false;
            if (acaoMapeada == 1) {
              danoHeroi = heroi->atacar();
              monstro->setVida(monstro->getVida() - danoHeroi);
              logLinha1 = heroi->getNome() + " ATACOU: -" +
                          std::to_string(danoHeroi) + " HP!";
              animHeroiOffsetX = 50;
              flashMonstroFrames = 8;
            } else if (acaoMapeada == 2 && ehMago) {
              std::string msgMagia;
              int manaAntes = static_cast<Mago *>(heroi.get())->getMana();
              danoHeroi =
                  static_cast<Mago *>(heroi.get())->lancarBolaDeFogo(msgMagia);
              monstro->setVida(monstro->getVida() - danoHeroi);
              logLinha1 = msgMagia + " (-" + std::to_string(danoHeroi) + " HP)";
              animHeroiOffsetX = 40;
              flashMonstroFrames = 12;
              // Dispara projetil visual se tinha mana para Bola de Fogo
              if (manaAntes >= 20) {
                fogoAtivo = true;
                fogoX = 240;
                fogoY = H - 350;
                fogoAlvoX = W - 260;
              }
            } else if (acaoMapeada == 3 && ehMago) {
              logLinha1 = static_cast<Mago *>(heroi.get())->curar();
              spawnParticulasCura(110, H - 400, 0x002EC4B6);
            } else if (acaoMapeada == 4) {
              logLinha1 = heroi->usarPocao();
              spawnParticulasCura(110, H - 400, 0x0016A34A);
            } else if (acaoMapeada == 5 && ehMago) {
              logLinha1 = static_cast<Mago *>(heroi.get())->usarElixir();
              spawnParticulasCura(110, H - 400, 0x003A86FF);
            } else {
              logLinha1 = heroi->getNome() + " ASSUMIU POSTURA DEFENSIVA!";
              defendeu = true;
            }
            // Verifica derrota do inimigo
            if (monstro->getVida() <= 0) {
              logLinha2 = monstro->getTipo() + " FOI DERROTADO!";
              if (modoJogoIndex == 0 && !isBossAtual) {
                estado = LEVEL_UP; // Vai para o Chefao na Campanha!
              } else {
                estado = FIM_DE_JOGO;
              }
            } else {
              // Contra-ataque do Inimigo
              bool critico = false;
              int danoInimigo = monstro->atacar(critico);
              if (defendeu)
                danoInimigo /= 2;
              heroi->setVida(heroi->getVida() - danoInimigo);
              animMonstroOffsetX = -50;
              flashHeroiFrames = 8;
              if (critico) {
                logLinha2 = "CRITICO! " + monstro->getTipo() + " CAUSOU -" +
                            std::to_string(danoInimigo) + " HP!";
                screenShakeFrames = 14; // Tremor de tela!
              } else {
                logLinha2 = monstro->getTipo() + " ATACOU: -" +
                            std::to_string(danoInimigo) + " HP!";
              }
              if (heroi->getVida() <= 0) {
                estado = FIM_DE_JOGO;
              } else {
                rodada++;
              }
            }
          } catch (const std::exception &e) {
            logLinha1 = std::string("ERRO: ") + e.what();
            logLinha2 = "ESCOLHA OUTRA ACAO.";
          }
        }
        // Atualiza animacoes a 60 FPS
        if (animHeroiOffsetX > 0)
          animHeroiOffsetX -= 5;
        if (animMonstroOffsetX < 0)
          animMonstroOffsetX += 5;
        if (flashHeroiFrames > 0)
          flashHeroiFrames--;
        if (flashMonstroFrames > 0)
          flashMonstroFrames--;
        int shakeX = 0, shakeY = 0;
        if (screenShakeFrames > 0) {
          shakeX = (rand() % 17) - 8;
          shakeY = (rand() % 17) - 8;
          screenShakeFrames--;
        }
        if (fogoAtivo) {
          fogoX += 38;
          fogoY -= 8;
          if (fogoX >= fogoAlvoX)
            fogoAtivo = false;
        }
        for (size_t i = 0; i < particulas.size();) {
          particulas[i].y += particulas[i].vy;
          particulas[i].vida--;
          if (particulas[i].vida <= 0)
            particulas.erase(particulas.begin() + i);
          else
            ++i;
        }
        // Renderizacao da Arena (com offset de Screen Shake)
        gfx.limparTela(0x00141824);
        gfx.desenharRetangulo(0 + shakeX, 170 + shakeY, W, H - 410, 0x001E293B);
        gfx.desenharTexto(30, 20,
                          "RODADA " + std::to_string(rodada) + "  |  NV." +
                              std::to_string(nivelHeroi) + "  |  ESC = SAIR",
                          0x00FFD700, 2);
        // HUD do Inimigo
        gfx.desenharCaixaBorda(30 + shakeX, 55 + shakeY, 420, 100, 0x001E2436,
                               0x00AAAAAA);
        gfx.desenharTexto(45 + shakeX, 70 + shakeY, monstro->getTipo(),
                          0x00FF5555, 2);
        gfx.desenharTexto(45 + shakeX, 97 + shakeY,
                          "HP: " + std::to_string(monstro->getVida()) + "/" +
                              std::to_string(monstro->getVidaMax()),
                          0x00FFFFFF, 2);
        gfx.desenharBarraStatus(45 + shakeX, 123 + shakeY, 380, 18,
                                monstro->getVida(), monstro->getVidaMax(),
                                0x00E63946, 0x00331111);
        // Sprite do Inimigo
        if (flashMonstroFrames % 2 == 0) {
          const auto &spriteMonstro = isBossAtual ? SPRITE_DRAGAO : SPRITE_ORC;
          gfx.desenharSprite(W - 280 + animMonstroOffsetX + shakeX, 90 + shakeY,
                             spriteMonstro, PALETA_RPG, 10, true);
        }
        // Sprite do Heroi
        if (flashHeroiFrames % 2 == 0) {
          const auto &spriteHeroi = ehMago ? SPRITE_MAGO : SPRITE_GUERREIRO;
          gfx.desenharSprite(110 + animHeroiOffsetX + shakeX, H - 400 + shakeY,
                             spriteHeroi, PALETA_RPG, 9, false);
        }
        // Renderiza Bola de Fogo e Particulas
        if (fogoAtivo) {
          gfx.desenharSprite(fogoX, fogoY, SPRITE_BOLA_FOGO, PALETA_RPG, 6,
                             false);
        }
        for (const auto &p : particulas) {
          gfx.desenharRetangulo(p.x, p.y, 6, 6, p.cor);
        }
        // HUD do Heroi
        int hudHeroiX = W - 460 + shakeX;
        int hudHeroiY = H - 390 + shakeY;
        gfx.desenharCaixaBorda(hudHeroiX, hudHeroiY, 430, 130, 0x001E2436,
                               0x00AAAAAA);
        gfx.desenharTexto(hudHeroiX + 15, hudHeroiY + 15,
                          heroi->getNome() +
                              (ehMago ? " (MAGO)" : " (GUERREIRO)"),
                          0x004CC9F0, 2);
        gfx.desenharTexto(hudHeroiX + 15, hudHeroiY + 45,
                          "HP: " + std::to_string(heroi->getVida()) + "/" +
                              std::to_string(heroi->getVidaMax()),
                          0x00FFFFFF, 2);
        gfx.desenharBarraStatus(hudHeroiX + 180, hudHeroiY + 45, 230, 16,
                                heroi->getVida(), heroi->getVidaMax(),
                                0x002EC4B6, 0x00113322);
        if (ehMago) {
          Mago *m = static_cast<Mago *>(heroi.get());
          gfx.desenharTexto(hudHeroiX + 15, hudHeroiY + 80,
                            "MP: " + std::to_string(m->getMana()) + "/" +
                                std::to_string(m->getManaMax()),
                            0x00FFFFFF, 2);
          gfx.desenharBarraStatus(hudHeroiX + 180, hudHeroiY + 80, 230, 16,
                                  m->getMana(), m->getManaMax(), 0x003A86FF,
                                  0x00112244);
        }
        // Painel Inferior: Menu + Log
        int painelY = H - 240;
        int larguraMenu = 440;
        gfx.desenharCaixaBorda(30, painelY, larguraMenu, 210, 0x000F172A,
                               0x00FFFFFF);
        for (size_t i = 0; i < opcoesMenu.size(); ++i) {
          int itemY = painelY + 18 + (i * 30);
          uint32_t corItem = ((int)i == opcaoBatalha) ? 0x00FFD700 : 0x00CBD5E1;
          std::string prefixo = ((int)i == opcaoBatalha) ? "> " : "  ";
          gfx.desenharTexto(45, itemY, prefixo + opcoesMenu[i], corItem, 2);
        }
        int logX = 30 + larguraMenu + 20;
        int logW = W - logX - 30;
        gfx.desenharCaixaBorda(logX, painelY, logW, 210, 0x000F172A,
                               0x00FFFFFF);
        gfx.desenharTexto(logX + 20, painelY + 20,
                          "LOG DE COMBATE:", 0x0094A3B8, 2);
        gfx.desenharTexto(logX + 20, painelY + 60, logLinha1, 0x00FFFFFF, 2);
        gfx.desenharTexto(logX + 20, painelY + 95, logLinha2, 0x00FACC15, 2);
        gfx.desenharTexto(logX + 20, painelY + 165, "SETAS CIMA/BAIXO + ENTER",
                          0x0064748B, 2);
        // =====================================================================
        // 5. TELA DE FIM DE JOGO
        // =====================================================================
      } else if (estado == FIM_DE_JOGO) {
        bool venceu = (heroi->getVida() > 0);
        if (!rankingSalvo) {
          std::ofstream arquivo_ranking("ranking.txt", std::ios::app);
          if (arquivo_ranking.is_open()) {
            std::string modoTxt = (modoJogoIndex == 0) ? "Campanha" : "Rapida";
            arquivo_ranking << "Jogador: " << heroi->getNome()
                            << " | Classe: " << (ehMago ? "Mago" : "Guerreiro")
                            << " | Modo: " << modoTxt << " | Resultado: "
                            << (venceu ? "VITORIA" : "GAME OVER") << std::endl;
            arquivo_ranking.close();
          }
          rankingSalvo = true;
        }
        if (tecla == '\n' || tecla == '\r') {
          estado = TELA_INICIAL;
        } else if (tecla == 'q' || tecla == 'Q') {
          rodando = false;
        }
        gfx.limparTela(0x000F172A);
        int boxW = 640;
        int boxH = 320;
        int boxX = (W - boxW) / 2;
        int boxY = (H - boxH) / 2;
        uint32_t corTema = venceu ? 0x002EC4B6 : 0x00E63946;
        gfx.desenharCaixaBorda(boxX, boxY, boxW, boxH, 0x001E293B, corTema, 4);
        gfx.desenharTexto(boxX + 160, boxY + 40,
                          venceu ? "VITORIA GLORIOSA!" : "GAME OVER...",
                          corTema, 3);
        gfx.desenharTexto(boxX + 50, boxY + 115, logLinha1, 0x00FFFFFF, 2);
        gfx.desenharTexto(boxX + 50, boxY + 150, logLinha2, 0x00FACC15, 2);
        gfx.desenharTexto(boxX + 50, boxY + 200,
                          "RESULTADO SALVO EM RANKING.TXT", 0x0094A3B8, 2);
        gfx.desenharTexto(boxX + 50, boxY + 255,
                          "[ENTER] MENU INICIAL   |   [ESC] SAIR", 0x00FFD700,
                          2);
      }
      gfx.atualizarTela();
      usleep(16666); // ~60 FPS
    }
  } catch (const std::exception &e) {
    std::cerr << "Erro fatal: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
