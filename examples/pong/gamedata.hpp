#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Up, Down};
enum class State { Playing, GameOver, Win };

struct GameData {
  State m_state{State::Playing};
  std::bitset<2> m_inputLeft; // LeftBar
  std::bitset<2> m_inputRight;  // RightBar
};

#endif