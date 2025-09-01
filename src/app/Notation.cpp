#include "gomoku/Notation.hpp"
#include <cctype>

namespace gomoku::notation {

std::optional<Pos> parse(const std::string& in){
  if (in.size()<2 || in.size()>3) return std::nullopt;
  char c = (char)std::toupper((unsigned char)in[0]);
  if (c < 'A' || c > 'A' + BOARD_SIZE - 1) return std::nullopt;
  int x = c - 'A';
  if (!std::isdigit((unsigned char)in[1])) return std::nullopt;
  int y = (in.size()==2) ? (in[1]-'0') : (in[1]-'0')*10 + (in[2]-'0');
  if (y < 1 || y > BOARD_SIZE) return std::nullopt;
  return Pos{(uint8_t)x, (uint8_t)(y-1)};
}

std::string toString(Pos p){
  std::string s;
  s.push_back(char('A' + p.x));
  int yy = p.y + 1;
  if (yy >= 10) s.push_back(char('0' + (yy/10)));
  s.push_back(char('0' + (yy%10)));
  return s;
}

} // namespace gomoku::notation
