
#pragma once

#include <cstdint>
#include <vector>

// Paleta de cores 0x00RRGGBB (0 = Transparente)
inline uint32_t obterCorPaleta(char simbolo) {
  switch (simbolo) {
    case '.': return 0x00000000; // Transparente
    case 'K': return 0x00111118; // Contorno preto
    case 'W': return 0x00F8FAFC; // Branco / Brilho aco
    case 'S': return 0x0094A3B8; // Prata / Armadura
    case 'G': return 0x00475569; // Cinza escuro / Sombra aco
    case 'P': return 0x00FDBA74; // Pele humana
    case 'B': return 0x002563EB; // Azul real (Manto Mago / Capa)
    case 'C': return 0x0038BDF8; // Ciano / Cristal cajado
    case 'Y': return 0x00FACC15; // Amarelo / Ouro / Olho dragao
    case 'O': return 0x00F97316; // Laranja / Fogo
    case 'R': return 0x00DC2626; // Vermelho / Sangue / Dragao
    case 'D': return 0x007F1D1D; // Vermelho escuro / Escamas
    case 'V': return 0x0016A34A; // Verde (Pele Orc)
    case 'E': return 0x0014532D; // Verde escuro (Sombra Orc)
    case 'M': return 0x0078350F; // Marrom (Madeira / Couro)
    case 'N': return 0x001E1B4B; // Roxo/Preto profundo (Dragao Negro)
    default:  return 0x00000000;
  }
}

// Sprite 16x16: GUERREIRO
inline std::vector<std::string> spriteGuerreiro() {
  return {
    "....KKKK........",
    "...KSSSSK.......",
    "..KRRRRRRK......",
    "..KSPPPPSK...KW.",
    "..KSPKPKPK..KWSK",
    "...KPPPPK...KWSK",
    "..KKKKKKKK..KWSK",
    ".KSSSSSSSSK.KWSK",
    "KSSBBSSBBSSKKYYK",
    "KSSBBSSBBSSKKMMK",
    "KSSSSSSSSSSK.KK.",
    ".KMMMMMMMMK.....",
    "..KSSKKSSK......",
    "..KSSKKSSK......",
    ".KMMK..KMMK.....",
    ".KKKK..KKKK....."
  };
}

// Sprite 16x16: MAGO
inline std::vector<std::string> spriteMago() {
  return {
    ".....KK.........",
    "....KBBK........",
    "...KBBBBK....KCK",
    "..KBBBBBBK..KCCK",
    ".KYYYYYYYYK.KCK.",
    "..KPPPPPPK..KMK.",
    "..KPKPPKPK..KMK.",
    "...KWWWWK...KMK.",
    "..KBBBBBBK..KMK.",
    ".KBBBBBBBBK.KMK.",
    ".KBBYBBYBBK.KMK.",
    ".KBBBBBBBBK.KMK.",
    "..KBBBBBBK..KMK.",
    "..KBBBBBBK..KMK.",
    "...KMMMMK...KK..",
    "...KKKKKK......."
  };
}

// Sprite 16x16: ORC GUERREIRO (Inimigo Normal)
inline std::vector<std::string> spriteOrc() {
  return {
    ".....KKKKKK.....",
    "....KVVVVVVK....",
    "...KVVEVVEVVK...",
    "...KVVKVVKVVK...",
    "..WKVVVVVVVVKW..",
    "..WKKVVWWVVKKW..",
    "...KKKKKKKKKK...",
    "..KMMMMMMMMMMK..",
    ".KVVMMMMMMMMVVK.",
    ".KVVMMYYYYMMVVK.",
    "KVVVMMMMMMMMVVVK",
    "KVV.KMMMMMMK.VVK",
    "KK..KVVKKVVK..KK",
    "....KVVKKVVK....",
    "...KMMK..KMMK...",
    "...KKKK..KKKK..."
  };
}

// Sprite 16x16: DRAGAO NEGRO (Chefao)
inline std::vector<std::string> spriteDragao() {
  return {
    ".KK..........KK.",
    "KYYK..KKKK..KYYK",
    "KYYKKKNNNNKKKYYK",
    ".KKNNNNNNNNNNKK.",
    "..KNNYYNNYYNNK..",
    "..KNNNNNNNNNNK..",
    ".KNNRRRRRRRRNNK.",
    "KNNNKKKKKKKKNNNK",
    "KNNNNNNNNNNNNNNK",
    "KNNDRRRRRRRRDNNK",
    ".KNDRRRRRRRRDNK.",
    ".KNNDRRRRRRDNNK.",
    "..KNNNNNNNNNNK..",
    "..KNNNK..KNNNK..",
    ".KYYYK....KYYYK.",
    ".KKKK......KKKK."
  };
}
