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
//taille du plateau en variables globales
int taille_x = 6;
int taille_y = 10;


/*
 * breaktrough player 6*10
 * taille_x = 6
 * taille_y = 10
 * ./a.out @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo o 1
 * ./a.out @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo @ 1
 * ./a.out @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo o 1


 
 */

void init(char _strboard[], int _board[]) {
    //board vide
    for(int i = 0; i < taille_x*taille_y; i++) {
        _board[i] = EMPTY;
    }
    
    //set up BLACK (lignes du haut)
    for(int y = 0; y < 2; y++) {
        for(int x = 0; x < taille_x; x++) {
            _board[y * taille_x + x] = BLACK;
        }
    }
    
    //set up WHITE (lignes du bas)
    for(int y = taille_y - 2; y < taille_y; y++) {
        for(int x = 0; x < taille_x; x++) {
            _board[y * taille_x + x] = WHITE;
        }
    }
    //si strboard est fourni, remplacer par ses valeurs
    if(_strboard) {
        for(int i = 0; i < taille_x*taille_y; i++) {
            if(_strboard[i] == '.') _board[i] = EMPTY;
            if(_strboard[i] == 'x') _board[i] = BLACK;
            if(_strboard[i] == 'o') _board[i] = WHITE;
        }
    }
}
std::chrono::time_point<std::chrono::steady_clock> start_time;
double time_limit;
bool verbose = LOG_ACTIVE;

bool is_a_win(int _board[], int _color, int taille_x, int taille_y) {
    
    //check si le joueur a gagné en ligne (horizontalement)
    if (_color == WHITE) {
        for (int x = 0; x < taille_x; x++) {
            if (_board[x] == WHITE) return true; // Ligne du haut
        }
    } else {
        for (int x = 0; x < taille_x; x++) {
            if (_board[(taille_y - 1) * taille_x + x] == BLACK) return true; // Ligne du bas
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
double h(int _board[], int _color, int taille_plateau_x = 6, int taille_plateau_y = 10) {
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

// Depth Limited Search
double dls(int _board[], int depth, int _color, bool& completed, int taille_plateau_x = 6, int taille_plateau_y = 10) {
    //verifie d'abord si le temps est écoulé
    if (is_time_expired()) {
        completed = false;
        return h(_board, _color, taille_plateau_x, taille_plateau_y);
    }
    
    //conditions de fin
    if (is_a_win(_board, WHITE, taille_plateau_x, taille_plateau_y)) return 1000000.0;
    if (is_a_win(_board, BLACK, taille_plateau_x, taille_plateau_y)) return -1000000.0;
    
    if (depth == 0) {
        completed = true;
        return h(_board, _color, taille_plateau_x, taille_plateau_y);
    }
    
    //tous les coups possibles
    int moves[100][3]; // [indice_origine, indice_destination, capture]
    int nb_moves = 0;
    
    for (int y = 0; y < taille_plateau_y; y++) {
        for (int x = 0; x < taille_plateau_x; x++) {
            int idx = y * taille_plateau_x + x;
            if (_board[idx] == _color) {
                // Direction de déplacement selon la couleur
                int dy = (_color == WHITE) ? -1 : 1;
                
                // Avancer tout droit
                int ny = y + dy;
                if (ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + x;
                    if (_board[nidx] == EMPTY) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 0; //pas de capture
                        nb_moves++;
                    }
                }
                
                //capture diagonale gauche
                if (x > 0 && ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + (x-1);
                    if (_board[nidx] != EMPTY && _board[nidx] != _color) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 1; //capture
                        nb_moves++;
                    }
                }
                
                //capture diagonale droite
                if (x < taille_plateau_x-1 && ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + (x+1);
                    if (_board[nidx] != EMPTY && _board[nidx] != _color) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 1; //capture
                        nb_moves++;
                    }
                }
            }
        }
    }
    
    if (nb_moves == 0) {
        completed = true;
        return (_color == WHITE) ? -1000000.0 : 1000000.0; //pas de coup possible = défaite
    }

    bool is_white = (_color == WHITE);
    double best_value = is_white ? -1000000.0 : 1000000.0;
    bool any_move_completed = false;
    
    for (int i = 0; i < nb_moves; i++) {
        int next_board[taille_x * taille_y];
        memcpy(next_board, _board, sizeof(int) * 6 * 10);
        
        //applique le coup
        int from = moves[i][0];
        int to = moves[i][1];
        next_board[from] = EMPTY;
        next_board[to] = _color;
        
        bool move_completed;
        int next_color = (_color == WHITE) ? BLACK : WHITE;
        double value = dls(next_board, depth-1, next_color, move_completed, taille_plateau_x, taille_plateau_y);
        
        if (move_completed) any_move_completed = true;
        
        if (is_white) {
            best_value = std::max(best_value, value);
        } else {
            best_value = std::min(best_value, value);
        }
        
        if (!move_completed) break;
    }
    
    completed = any_move_completed;
    return best_value;
}

// Iterative Deepening Search
int ids(int _board[], int _color, double _time_limit, int& reached_depth, int taille_plateau_x = 6, int taille_plateau_y = 10) {
    start_time = std::chrono::steady_clock::now();
    time_limit = _time_limit;
    
    int best_move = -1;
    reached_depth = 0;
    
    //tous les coups possibles
    int moves[100][3]; // [indice_origine, indice_destination, capture]
    int nb_moves = 0;
    
    for (int y = 0; y < taille_plateau_y; y++) {
        for (int x = 0; x < taille_plateau_x; x++) {
            int idx = y * taille_plateau_x + x;
            if (_board[idx] == _color) {
                //direction de déplacement selon la couleur
                int dy = (_color == WHITE) ? -1 : 1;
                
                //avance tout droit
                int ny = y + dy;
                if (ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + x;
                    if (_board[nidx] == EMPTY) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 0;
                        nb_moves++;
                    }
                }
                
                //capture diagonale gauche
                if (x > 0 && ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + (x-1);
                    if (_board[nidx] != EMPTY && _board[nidx] != _color) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 1;
                        nb_moves++;
                    }
                }
                
                //capture diagonale droite
                if (x < taille_plateau_x-1 && ny >= 0 && ny < taille_plateau_y) {
                    int nidx = ny * taille_plateau_x + (x+1);
                    if (_board[nidx] != EMPTY && _board[nidx] != _color) {
                        moves[nb_moves][0] = idx;
                        moves[nb_moves][1] = nidx;
                        moves[nb_moves][2] = 1;
                        nb_moves++;
                    }
                }
            }
        }
    }
    
    if (nb_moves == 0) return -1;
    
    //choix d'un coup aléatoire par défaut
    best_move = rand() % nb_moves;
    
    for (int depth = 1; ; depth++) {
        bool is_white = (_color == WHITE);
        double best_value = is_white ? -1000000.0 : 1000000.0;
        int current_best_move = -1;
        bool any_completed = false;
        
        for (int i = 0; i < nb_moves; i++) {
            int next_board[taille_x * taille_y];
            memcpy(next_board, _board, sizeof(int) * 6 * 10);
            
            //applicat le coup
            int from = moves[i][0];
            int to = moves[i][1];
            next_board[from] = EMPTY;
            next_board[to] = _color;
            
            bool move_completed;
            int next_color = (_color == WHITE) ? BLACK : WHITE;
            double value = dls(next_board, depth-1, next_color, move_completed, taille_plateau_x, taille_plateau_y);
            
            if (move_completed) {
                any_completed = true;
                if ((is_white && value > best_value) || (!is_white && value < best_value)) {
                    best_value = value;
                    current_best_move = i;
                }
            }
            
            if (is_time_expired()) break;
        }
        
        if (any_completed && current_best_move != -1) {
            best_move = current_best_move;
            reached_depth = depth;
        }
        
        if (is_time_expired()) break;
    }
    
    //return le meilleur coup trouvé
    return moves[best_move][1]; //return la position 'destination'
}
// Fonction de génération de coup pour le jeu Breakthrough
int genmove(int _board[], int _color, double time_limit_val = 1) {
    if(LOG_ACTIVE) {
        fprintf(stderr, "--- genmove pour Breakthrough\n");
        fprintf(stderr, "couleur : %d (1=BLANC, 2=NOIR)\n", _color);
    }
    
    //temps de départ
    auto search_start = std::chrono::high_resolution_clock::now();
    
    //le nombre de pièces restantes pour ajuster le temps de réflexion
    int nb_pieces = 0;
    for(int i = 0; i < taille_x * taille_y; i++) {
        if(_board[i] != EMPTY) nb_pieces++;
    }
    
    //ajustement du temps limite en fonction de l'avancement de la partie
    //moin de temps en fin de partie où les choix sont plus limités
    if(nb_pieces < 10) time_limit_val = 0.2;
    
    // Appel à la recherche itérative avec approfondissement
    int reached_depth;
    int move = ids(_board, _color, time_limit_val, reached_depth);
    
    //mesure du temps de fin et calcul de la durée
    auto search_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start).count();
    
    // Affichage des informations de recherche si le mode verbeux est activé
    if(LOG_ACTIVE) {
        fprintf(stderr, "Profondeur atteinte: %d\n", reached_depth);
        fprintf(stderr, "Temps de recherche: %ld millisecondes\n", duration);
        fprintf(stderr, "--- retourne le coup: %d\n", move);
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
  /*
  int move = ids(ttt_board, turn_board, time_limit, reached_depth);
  int from_x = move / taille_x;
  int from_y = move % taille_x;
  int to_x = (move + direction) / taille_x;  // direction = -1 pour Blancs, +1 pour Noirs
  int to_y = (move + direction) % taille_x;
  printf("%d %d %d %d\n", from_x, from_y, to_x, to_y);
  */
  char* input_board = _av[1];
  int turn_board = WHITE;
  if(strcmp(_av[2],"@")==0) turn_board = BLACK;
  std::srand(std::time(0));
  int r = std::rand(); // first call to get more variable rand
  r++; // to avoid a warning :-)
  int ttt_board[taille_x * taille_y];
  init(input_board, ttt_board);
  printf("%d\n", genmove(ttt_board, turn_board));
  return 0;
}