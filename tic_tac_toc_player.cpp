#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>

#define EMPTY 0
#define WHITE 1
#define BLACK 2

#define LOG_ACTIVE 0

/*
 * quasi random tic tac toe player called tic tac toc :-)
 * plays for direct victory if it is possible
 * otherwise plays randomly
 */

void init(char _strboard[9], int _board[9]) {
  for(int i = 0; i < 9; i++) {
    if(_strboard[i] == '.') _board[i] = EMPTY;
    if(_strboard[i] == 'x') _board[i] = BLACK;
    if(_strboard[i] == 'o') _board[i] = WHITE;
  }
}
bool is_a_win(int _board[9], int _color) {
  if(_board[0]==_color && _board[1]==_color && _board[2]==_color) return true;
  if(_board[3]==_color && _board[4]==_color && _board[5]==_color) return true;
  if(_board[6]==_color && _board[7]==_color && _board[8]==_color) return true;
  if(_board[0]==_color && _board[3]==_color && _board[6]==_color) return true;
  if(_board[1]==_color && _board[4]==_color && _board[7]==_color) return true;
  if(_board[2]==_color && _board[5]==_color && _board[8]==_color) return true;
  if(_board[0]==_color && _board[4]==_color && _board[8]==_color) return true;
  if(_board[2]==_color && _board[4]==_color && _board[6]==_color) return true;
  return false;
}
void fprint_board(FILE* _out, int _board[9]) {
  fprintf(_out, "[%d, ", _board[0]);
  for(int i = 1; i < 8; i++) {
    fprintf(_out, "%d, ", _board[i]);
  }
  fprintf(_out, "%d]\n", _board[8]);
}
int genmove(int _board[9], int _color){
  if(LOG_ACTIVE) {
    fprintf(stderr, "--- genmove\n");
    fprintf(stderr, "board : "); fprint_board(stderr,_board);
    fprintf(stderr, "color : %d\n", _color);
  }
  int nb_empty = 0;
  for(int i = 0; i < 9; i++) {
    if(_board[i] == EMPTY) {
      _board[i] = _color;
      if(is_a_win(_board, _color)) return i;
      _board[i] = EMPTY;
      nb_empty++;
    }
  }
  int r = std::rand() % nb_empty;
  for(int i = 0; i < 9; i++) {
    if(_board[i] == EMPTY) {
      r--;
      if(r==-1) {
        if(LOG_ACTIVE) {
          fprintf(stderr, "--- return %d\n", i);
        }
        return i;
      }
    }
  }  
  return -1;
}
/*
 * g++ -Wall tic_tac_toc_player.cpp -o tic_tac_toc_player
 * to generate x move : ./a.out x.x...... x
 * to generate o move : ./a.out x.x...... o
 * returns where to play
 */
int main(int _ac, char** _av) {
  if(_ac != 3) {
    fprintf(stderr, "usage: %s STRBOARD TURN\n", _av[0]);
    return 0;
  }
  char* input_board = _av[1];
  int turn_board = WHITE;
  if(strcmp(_av[2],"x")==0) turn_board = BLACK;
  std::srand(std::time(0));
  int r = std::rand(); // first call to get more variable rand
  r++; // to avoid a warning :-)
  int ttt_board[9];
  init(input_board, ttt_board);
  printf("%d\n", genmove(ttt_board, turn_board));
  return 0;
}