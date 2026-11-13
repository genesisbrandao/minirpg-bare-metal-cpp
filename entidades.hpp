#pragma once

#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>

// ================== CLASSE DE JOGADOR ==================
class Jogador {
protected:
  std::string nome;
  int vida;
  int vidaMax;
  int forca;
  std::map<std::string, int> mochila;

public:
  Jogador(std::string nomeInicial, int vidaInicial, int forcaInicial)
      : nome(nomeInicial), vida(vidaInicial), vidaMax(vidaInicial),
        forca(forcaInicial) {
    mochila["Pocao de Vida"] = 2;
  }
  virtual ~Jogador() {}

  void setVida(int novaVida) {
    if (novaVida < 0)
      vida = 0;
    else if (novaVida > vidaMax)
      vida = vidaMax;
    else
      vida = novaVida;
  }

  int getVida() const { return vida; }
  int getVidaMax() const { return vidaMax; }
  int getForca() const { return forca; }
  std::string getNome() const { return nome; }

  virtual int atacar() { return forca + (rand() % 6); }

  int qtdeItem(const std::string& nomeItem) { return mochila[nomeItem]; }

  std::string usarPocao() {
    if (mochila["Pocao de Vida"] > 0) {
      mochila["Pocao de Vida"]--;
      setVida(vida + 30);
      return nome + " bebeu Pocao de Vida (+30 HP)!";
    } else {
      throw std::runtime_error("Sem Pocao de Vida na mochila!");
    }
  }
};

// ================== CLASSE MAGO ==================
class Mago : public Jogador {
private:
  int mana;
  int manaMax;

public:
  Mago(std::string nomeMago, int vidaInicial, int forcaInicial)
      : Jogador(nomeMago, vidaInicial, forcaInicial), mana(50), manaMax(50) {
    mochila["Elixir de Mana"] = 2;
  }

  int getMana() const { return mana; }
  int getManaMax() const { return manaMax; }

  int lancarBolaDeFogo(std::string& logMsg) {
    if (mana >= 20) {
      mana -= 20;
      logMsg = "Bola de Fogo conjurada!";
      return forca * 2 + (rand() % 10);
    } else {
      logMsg = "Sem mana! Ataque basico com cajado.";
      return atacar();
    }
  }

  std::string curar() {
    if (mana >= 15) {
      mana -= 15;
      int cura = 30 + (rand() % 11);
      setVida(vida + cura);
      return "Magia de Cura: +" + std::to_string(cura) + " HP!";
    } else {
      throw std::runtime_error("Mana insuficiente (min. 15 MP)!");
    }
  }

  std::string usarElixir() {
    if (mochila["Elixir de Mana"] > 0) {
      mochila["Elixir de Mana"]--;
      mana = (mana + 25 > manaMax) ? manaMax : (mana + 25);
      return nome + " bebeu Elixir (+25 MP)!";
    } else {
      throw std::runtime_error("Sem Elixir de Mana na mochila!");
    }
  }
};

// ================== CLASSES DE INIMIGO ==================
class Inimigo {
protected:
  std::string tipo;
  int vida;
  int vidaMax;
  int forca;

public:
  Inimigo(std::string tipoInicial, int vidaInicial, int forcaInicial)
      : tipo(tipoInicial), vida(vidaInicial), vidaMax(vidaInicial),
        forca(forcaInicial) {}
  virtual ~Inimigo() {}

  void setVida(int novaVida) {
    if (novaVida < 0)
      vida = 0;
    else if (novaVida > vidaMax)
      vida = vidaMax;
    else
      vida = novaVida;
  }

  int getVida() const { return vida; }
  int getVidaMax() const { return vidaMax; }
  std::string getTipo() const { return tipo; }

  virtual int atacar(bool& foiCritico) {
    foiCritico = false;
    return forca + (rand() % 4);
  }
};

class Chefao : public Inimigo {
public:
  Chefao(std::string nomeChefao, int vidaInicial, int forcaInicial)
      : Inimigo(nomeChefao, vidaInicial, forcaInicial) {}

  int atacar(bool& foiCritico) override {
    if ((rand() % 100) > 70) {
      foiCritico = true;
      return (forca + (rand() % 6)) * 2;
    }
    foiCritico = false;
    return forca + (rand() % 6);
  }
};
