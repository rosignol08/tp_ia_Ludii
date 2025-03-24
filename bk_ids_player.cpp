#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>

#include <string>
#include <iostream>
#include <chrono>
//#include "mybt.h"


#define EMPTY 0
#define WHITE 1
#define BLACK 2

#define LOG_ACTIVE 0

/*
 * breaktrough player 6*10
 * 
 * 
 */

void init(char _strboard[6*10], int _board[6*10]) {
    //board vide
    for(int i = 0; i < 6*10; i++) {
        _board[i] = EMPTY;
    }
    
    //set up BLACK (lignes du haut)
    for(int y = 0; y < 2; y++) {
        for(int x = 0; x < 7; x++) {
            _board[y * 6 + x] = BLACK;
        }
    }
    
    //set up WHITE (lignes du bas)
    for(int y = 4; y < 6; y++) {
        for(int x = 0; x < 7; x++) {
            _board[y * 6 + x] = WHITE;
        }
    }
    //si strboard est fourni, remplacer par ses valeurs
    if(_strboard) {
        for(int i = 0; i < 6*10; i++) {
            if(_strboard[i] == '.') _board[i] = EMPTY;
            if(_strboard[i] == 'x') _board[i] = BLACK;
            if(_strboard[i] == 'o') _board[i] = WHITE;
        }
    }
}
std::chrono::time_point<std::chrono::steady_clock> start_time;
double time_limit;
bool verbose = LOG_ACTIVE;

bool is_a_win(int _board[6*10], int _color, int taille_plateau_x = 6, int taille_plateau_y = 10) {
    
    //check si le joueur a gagné en ligne (horizontalement)
    if (_color == BLACK) {
        for (int y = 0; y < taille_plateau_y; y++) {
            //verifie la colonne la plus à gauche
            if (_board[y * taille_plateau_x] == _color) {
                return true;
            }
        }
    }
    //check si l'autre joueur a gagné en ligne (horizontalement)
    else if (_color == WHITE) {
        for (int y = 0; y < taille_plateau_y; y++) {
            //verifie la colonne la plus à droite
            if (_board[y * taille_plateau_x + (taille_plateau_x - 1)] == _color) {
                return true;
            }
        }
    }
    
    return false;
}

void fprint_board(FILE* _out, int _board[9]) {
    fprintf(_out, "[%d, ", _board[0]);
    for(int i = 1; i < 8; i++) {
        fprintf(_out, "%d, ", _board[i]);
    }
    fprintf(_out, "%d]\n", _board[8]);
}

bool is_time_expired() {
    auto current_time = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1000.0;
    return elapsed >= time_limit;
}
//heuristique
double h(int _board[6*10], int _color, int taille_plateau_x = 6, int taille_plateau_y = 10) {
    double score = 0.0;
    
    //boucle pour chaque position sur le plateau
    for(int y = 0; y < taille_plateau_y; y++) {
        for(int x = 0; x < taille_plateau_x; x++) {
            int idx = y * taille_plateau_x + x;
            if(_board[idx] == WHITE) {
                //la valeur de base d'une pièce dépend de sa position
                double piece_value = (taille_plateau_y + 1) * (taille_plateau_y - y);
                score += piece_value;
                
                //bonus pour la protection mutuelle
                if(y+1 < taille_plateau_y && x+1 < taille_plateau_x && 
                   _board[(y+1) * taille_plateau_x + (x+1)] == WHITE) 
                    score += taille_plateau_y;
                if(y+1 < taille_plateau_y && x-1 >= 0 && 
                   _board[(y+1) * taille_plateau_x + (x-1)] == WHITE) 
                    score += taille_plateau_y;
                
                //bonus pour les menaces
                if(y-1 >= 0 && x+1 < taille_plateau_x && 
                   _board[(y-1) * taille_plateau_x + (x+1)] == BLACK) 
                    score += 2 * taille_plateau_y;
                if(y-1 >= 0 && x-1 >= 0 && 
                   _board[(y-1) * taille_plateau_x + (x-1)] == BLACK) 
                    score += 2 * taille_plateau_y;
            }
            else if(_board[idx] == BLACK) {
                //même logique pour les pièces noires
                double piece_value = (taille_plateau_y + 1) * y;
                score -= piece_value;
                
                if(y-1 >= 0 && x+1 < taille_plateau_x && 
                   _board[(y-1) * taille_plateau_x + (x+1)] == BLACK) 
                    score -= taille_plateau_y;
                if(y-1 >= 0 && x-1 >= 0 && 
                   _board[(y-1) * taille_plateau_x + (x-1)] == BLACK) 
                    score -= taille_plateau_y;
                
                if(y+1 < taille_plateau_y && x+1 < taille_plateau_x && 
                   _board[(y+1) * taille_plateau_x + (x+1)] == WHITE) 
                    score -= 2 * taille_plateau_y;
                if(y+1 < taille_plateau_y && x-1 >= 0 && 
                   _board[(y+1) * taille_plateau_x + (x-1)] == WHITE) 
                    score -= 2 * taille_plateau_y;
            }
        }
    }
    
    //retourne le score en fonction de la couleur
    return _color == WHITE ? score : -score;
}

double dls(int _board[9], int depth, int _color, bool& completed) {
    if(is_time_expired()) {
        completed = false;
        return h(_board, _color);
    }
    
    if(is_a_win(_board, _color)) return 1000.0;
    if(is_a_win(_board, (_color == WHITE) ? BLACK : WHITE)) return -1000.0;
    
    if(depth == 0) {
        completed = true;
        return h(_board, _color);
    }
    
    // Count empty squares and build moves
    int moves[9];
    int nb_moves = 0;
    for(int i = 0; i < 9; i++) {
        if(_board[i] == EMPTY) {
            moves[nb_moves++] = i;
        }
    }
    
    if(nb_moves == 0) {
        completed = true;
        return 0.0; // Draw
    }

    bool is_maximizing = (_color == WHITE);
    double best_value = is_maximizing ? -1000000.0 : 1000000.0;
    bool any_move_completed = false;
    
    for(int i = 0; i < nb_moves; i++) {
        int next_board[9];
        memcpy(next_board, _board, 9 * sizeof(int));
        next_board[moves[i]] = _color;
        
        bool move_completed;
        double value = dls(next_board, depth-1, (_color == WHITE) ? BLACK : WHITE, move_completed);
        
        if(move_completed) any_move_completed = true;
        
        if(is_maximizing) {
            best_value = std::max(best_value, value);
        } else {
            best_value = std::min(best_value, value);
        }
        
        if(!move_completed) break;
    }
    
    completed = any_move_completed;
    return best_value;
}

int ids(int _board[9], int _color, double max_time, int& reached_depth) {
    start_time = std::chrono::steady_clock::now();
    time_limit = max_time;
    reached_depth = 0;
    
    // Count empty squares and build moves
    int moves[9];
    int nb_moves = 0;
    for(int i = 0; i < 9; i++) {
        if(_board[i] == EMPTY) {
            moves[nb_moves++] = i;
        }
    }
    
    if(nb_moves == 0) return -1; // No moves available
    
    int best_move = moves[0];
    bool is_maximizing = (_color == WHITE);
    double best_value = is_maximizing ? -1000000.0 : 1000000.0;
    
    // Check for immediate wins
    for(int i = 0; i < nb_moves; i++) {
        int next_board[9];
        memcpy(next_board, _board, 9 * sizeof(int));
        next_board[moves[i]] = _color;
        if(is_a_win(next_board, _color)) {
            return moves[i]; // Winning move
        }
    }
    
    // Iterative deepening
    for(int d = 1; d <= 9; d++) { // Max depth is 9 in tic-tac-toe
        bool depth_completed = false;
        bool improved = false;
        
        for(int i = 0; i < nb_moves; i++) {
            if(is_time_expired()) {
                if(verbose && reached_depth > 0) {
                    fprintf(stderr, "Stopped at depth %d\n", d);
                }
                return best_move;
            }
            
            int next_board[9];
            memcpy(next_board, _board, 9 * sizeof(int));
            next_board[moves[i]] = _color;
            
            bool move_completed;
            double value = dls(next_board, d-1, (_color == WHITE) ? BLACK : WHITE, move_completed);
            
            if(move_completed) {
                depth_completed = true;
                if((is_maximizing && value > best_value) || (!is_maximizing && value < best_value)) {
                    best_value = value;
                    best_move = moves[i];
                    improved = true;
                }
            }
        }
        
        if(depth_completed) {
            reached_depth = d;
            if(!improved && d > 2) {
                return best_move;
            }
        } else {
            break;
        }
    }
    
    return best_move;
}

int genmove(int _board[9], int _color) {
    if(LOG_ACTIVE) {
        fprintf(stderr, "--- genmove\n");
        fprintf(stderr, "board : "); fprint_board(stderr, _board);
        fprintf(stderr, "color : %d\n", _color);
    }
    
    auto search_start = std::chrono::high_resolution_clock::now();
    
    // Count empty squares
    int nb_empty = 0;
    for(int i = 0; i < 9; i++) {
        if(_board[i] == EMPTY) nb_empty++;
    }
    
    // Adjust time limit based on number of moves
    double time_limit_val = (nb_empty < 5) ? 0.2 : 0.95;
    
    int reached_depth;
    int move = ids(_board, _color, time_limit_val, reached_depth);
    
    auto search_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();
    
    if(LOG_ACTIVE) {
        fprintf(stderr, "Reached depth: %d\n", reached_depth);
        fprintf(stderr, "Search took: %ld milliseconds\n", duration);
        fprintf(stderr, "--- return %d\n", move);
    }
    
    return move;
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