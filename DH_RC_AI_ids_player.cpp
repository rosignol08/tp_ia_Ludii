#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>

#include <string>
#include <iostream>
#include <chrono>
#include "mybt.h"


/*
Breakthrough IDS Player
ids player pour breaktrough
usage: ./a.out TAILLE_X TAILLE_Y STRBOARD TURN TIME_LIMIT
./a.out 10 6 @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo @ 1.0
./a.out 10 6 @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo o 1.0

Compilation:
g++ -Wall -std=c++11 DH_RC_AI_ids_player.cpp -o DH_RC_AI_ids_player
Utilisation en mode tournoi:
./DH_RC_AI_ids_player TAILLE_X TAILLE_Y BOARD_STR TURN MAX_TIME
Exemple pour 6x3: ./DH_RC_AI_ids_player 6 3 @@@@@@......oooooo o 1.0
Exemple pour 6x10: ./DH_RC_AI_ids_player 6 10 @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo o 1.0
 */

bt_t B;
int taille_y = 10;
int taille_x = 6;
int branches_count = 0;
bool verbose = false; //debug
//une variable globale pour suivre le temps
std::chrono::steady_clock::time_point start_time;
//@@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo
double time_limit;
void init(char _strboard[], bt_t& board) {
    board.init(taille_y, taille_x);  // Initialisation avec les dimensions

    if (verbose) {
        fprintf(stderr, "Initialisation du plateau %dx%d\n", taille_x, taille_y);
    }
    
    // Le tableau d'entrée est organisé ligne par ligne
    // @ sont les pièces noires (en haut)
    // o sont les pièces blanches (en bas)
    // . sont les cases vides
    /*
      a b c d e f g h i j 
    6 @ @ @ @ @ @ @ @ @ @ 
    5 @ @ @ @ @ @ @ @ @ @ 
    4 . . . . . . . . . . 
    3 . . . . . . . . . . 
    2 o o o o o o o o o o 
    1 o o o o o o o o o o 
    */
   
    
   int idx = 0;
   for (int y = 0; y < taille_y; y++) {
       for (int x = 0; x < taille_x; x++) {
           if (_strboard[idx] == '@') {
               board.board[y][x] = BLACK;
           } else if (_strboard[idx] == 'o') {
               board.board[y][x] = WHITE;
           } else {
               board.board[y][x] = EMPTY;
           }
           idx++;
       }
   }
   
   if (verbose) {
       fprintf(stderr, "État du plateau après initialisation:\n");
       board.print_board(stderr);
   }
}

//ça c'est une fonction qui vérifie si on a dépassé la limite de temps
bool is_time_expired() {
    auto current_time = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1000.0;
    return elapsed >= time_limit;
}

//fonction heuristique améliorée pour un jeu plus offensif et défensif
double h(const bt_t& board) {
    double score = 0.0;
    int last_row = board.nbl - 1;
    double advancement_weight = 3.0; // Poids pour favoriser l'avancement
    double capture_bonus = 5.0; // Bonus pour capturer un pion adverse
    double defense_penalty = 10.0; // Pénalité pour un pion adverse proche de vos lignes

    for(int i = 0; i < board.nbl; i++) {
        for(int j = 0; j < board.nbc; j++) {
            if(board.board[i][j] == WHITE) {
                // Favorise les pions blancs avancés
                //double piece_value = advancement_weight * (board.nbl - i);
                double piece_value = advancement_weight * pow(1.5, board.nbl - i);
                score += piece_value;

                // Bonus pour attaque possible
                if(i-1 >= 0 && j+1 < board.nbc && board.board[i-1][j+1] == BLACK) {
                    score += capture_bonus;
                }
                if(i-1 >= 0 && j-1 >= 0 && board.board[i-1][j-1] == BLACK) {
                    score += capture_bonus;
                }

                // Bonus pour protection
                if(i+1 < board.nbl && j+1 < board.nbc && board.board[i+1][j+1] == WHITE) {
                    score += 2.0;
                }
                if(i+1 < board.nbl && j-1 >= 0 && board.board[i+1][j-1] == WHITE) {
                    score += 2.0;
                }

                // Grosse pénalité si un pion noir est proche (défendre)
                if((i <= 1) && (board.turn % 2 == 1)) {
                    score -= defense_penalty;
                }
            }
            else if(board.board[i][j] == BLACK) {
                // Favorise les pions noirs avancés
                double piece_value = advancement_weight * i;
                score -= piece_value;

                // Bonus pour attaque possible
                if(i+1 < board.nbl && j+1 < board.nbc && board.board[i+1][j+1] == WHITE) {
                    score -= capture_bonus;
                }
                if(i+1 < board.nbl && j-1 >= 0 && board.board[i+1][j-1] == WHITE) {
                    score -= capture_bonus;
                }

                // Bonus pour protection
                if(i-1 >= 0 && j+1 < board.nbc && board.board[i-1][j+1] == BLACK) {
                    score -= 2.0;
                }
                if(i-1 >= 0 && j-1 >= 0 && board.board[i-1][j-1] == BLACK) {
                    score -= 2.0;
                }

                // Grosse pénalité si un pion blanc est proche (défendre)
                if((i >= last_row - 1) && (board.turn % 2 == 0)) {
                    score += defense_penalty;
                }
            }
        }
    }
    return score;
}
//std::unordered_map<uint64_t, double> transposition_table;

//Depth Limited Search retourne un bool pour indiquer si la recherche a été complétée
double dls(bt_t& board, int depth, bool& completed) {
    //vérifie d'abord si le temps est écoulé
    if(is_time_expired()) {
        completed = false;
        if (verbose) {
            fprintf(stderr, "Temps écoulé, retour à l'évaluation heuristique\n");
        }
        return h(board);
    }
    
    //test conditions de fin
    int result = board.endgame();
    if(result == WHITE) return 1000000.0;
    if(result == BLACK) return -1000000.0;
    
    if(depth == 0) {
        completed = true;
        return h(board);
    }
    
    board.update_moves();
    if(board.nb_moves == 0) {
        completed = true;
        return (board.turn % 2 == 0) ? -1000000.0 : 1000000.0;
    }

    bool is_white = (board.turn % 2 == 0);
    double best_value = is_white ? -1000000.0 : 1000000.0;
    bool any_move_completed = false;
    
    for(int i = 0; i < board.nb_moves; i++) {
        branches_count++;
        bt_t next_board = board;
        next_board.play(board.moves[i]);
        
        bool move_completed;
        double value = dls(next_board, depth-1, move_completed);
        
        if(move_completed) any_move_completed = true;
        
        if(is_white) {
            best_value = std::max(best_value, value);
        } else {
            best_value = std::min(best_value, value);
        }
        //if (verbose) {
        //    fprintf(stderr, "Coup évalué: (%d,%d) -> (%d,%d), Valeur: %f\n",
        //            board.moves[i].line_i, board.moves[i].col_i,
        //            board.moves[i].line_f, board.moves[i].col_f, value);
        //}
        
        if(!move_completed) break;
    }
    
    completed = any_move_completed;
    return best_value;
}

//Iterative Deepening Search retourne la profondeur atteinte
bt_move_t ids(bt_t& board, double max_time, int& reached_depth) {
    start_time = std::chrono::steady_clock::now();
    time_limit = max_time;
    reached_depth = 0;
    branches_count = 0;
    
    bt_move_t best_move = board.moves[0];
    bool white = (board.turn % 2 == 0);
    double best_value = white ? -1000000.0 : 1000000.0;
    
    //ajout d'une condition de victoire rapide
    for(int i = 0; i < board.nb_moves; i++) {
        bt_t next_board = board;
        next_board.play(board.moves[i]);
        if(next_board.endgame() == (white ? WHITE : BLACK)) {
            return board.moves[i];  // Coup gagnant immédiat
        }
    }
    
    for(int d = 1; ; d++) {
        bool depth_completed = false;
        board.update_moves();
        bool improved = false;  //pour détecter si on a trouvé un meilleur coup
        
        for(int i = 0; i < board.nb_moves; i++) {
            if(is_time_expired()) {
                if(verbose && reached_depth > 0) {
                    fprintf(stderr, "On a arrete à la profondeur %d\n", d);
                }
                return best_move;
            }
            
            bt_t next_board = board;
            next_board.play(board.moves[i]);
            
            bool move_completed;
            double value = dls(next_board, d-1, move_completed);
            
            if(move_completed) {
                depth_completed = true;
                if((white && value > best_value) || (!white && value < best_value)) {
                    best_value = value;
                    best_move = board.moves[i];
                    improved = true;
                }
            }
        }
        
        if(depth_completed) {
            reached_depth = d;
            if(!improved && d > 2) {
                //si on n'a pas trouvé de meilleur coup à cette profondeur et qu'on a déjà une profondeur décente, on peut s'arrêter
                return best_move;
            }
        } else {
            break;
        }
    }
    
    return best_move;
}
std::string convert_move_to_string(const bt_move_t& move, int rows) {
    // Position de départ
    int from_row = move.line_i;
    int from_col = move.col_i;
    
    // Position d'arrivée
    int to_row = move.line_f;
    int to_col = move.col_f;
    
    // Convertir en format LI-CI-LF-CF
    // Les lignes dans notre modèle sont de 0 (haut) à 5 (bas)
    // Pour les lignes: convertir de 0-5 à 6-1 (inverser et décaler)
    int li = rows - from_row;
    int lf = rows - to_row;
    
    // Pour les colonnes: a est à gauche (colonne 0) et j est à droite (colonne 9) pour 6x10
    char ci = 'a' + from_col;
    char cf = 'a' + to_col;
    
    // Construire la chaîne
    char result[5];
    sprintf(result, "%d%c%d%c", li, ci, lf, cf);
    return std::string(result);
}
// Implémentation de genmove compatible avec le premier programme
std::string genmove(bt_t& board, int _color, double time_limit_val = 1) {
    // Configurer le tour du joueur correctement
    board.turn = (_color == BLACK) ? 0 : 1;
    
    // Mettre à jour les coups disponibles
    board.update_moves();
    
    // Vérifier qu'il y a des coups disponibles
    if (board.nb_moves == 0) {
        return " "; // Aucun coup disponible
    }
    
    if (verbose) {
        fprintf(stderr, "Joueur: %s, Nombre de coups disponibles: %d\n", 
                (_color == WHITE) ? "BLANC (o)" : "NOIR (@)", board.nb_moves);
        
        //les coups disponibles
        for (int i = 0; i < board.nb_moves; i++) {
            fprintf(stderr, "Coup %d: (%d,%d) -> (%d,%d)\n", i,
                   board.moves[i].line_i, board.moves[i].col_i,
                   board.moves[i].line_f, board.moves[i].col_f);
        }
    }
    
    //IDS pour trouver le meilleur coup
    int reached_depth;
    bt_move_t move = ids(board, time_limit_val, reached_depth);
    bool valid = false;
    for (int i = 0; i < board.nb_moves; i++) {
        if (board.moves[i].line_i == move.line_i &&
            board.moves[i].col_i == move.col_i &&
            board.moves[i].line_f == move.line_f &&
            board.moves[i].col_f == move.col_f) {
            valid = true;
            break;
        }
    }

    if (!valid) {
        fprintf(stderr, "Problème : le coup sélectionné par IDS (%d,%d) -> (%d,%d) n'est pas un coup valide\n",
                move.line_i, move.col_i, move.line_f, move.col_f);
    }

    if (verbose) {
        //fprintf(stderr, "Meilleur coup trouvé: %s\n", move);
        fprintf(stderr, "Profondeur atteinte: %d\n", reached_depth);
    }
    
    //conversion du move en notation du premier programme
    //std::strinfg move_str = std::to_string(move.line_i + 1) + (char)('a' + move.col_i) + std::to_string(move.line_f + 1) + (char)('a' + move.col_f);
                          // conversion du move en notation standard LI-CI-LF-CF
    std::string move_str = std::to_string(board.nbl - move.line_i) + (char)('a' + move.col_i) + std::to_string(board.nbl - move.line_f) +(char)('a' + move.col_f);
    //std::string move_str = convert_move_to_string(move, taille_y, taille_x);
    if (verbose) {
        fprintf(stderr, "Format de sortie: %s\n", move_str.c_str());
    }
    //printf ("%s\n", move_str.c_str());
    return move_str;
}

int main(int _ac, char** _av) {
    if (_ac != 4) {
        fprintf(stderr, "Usage: %s STRBOARD TURN TIME_LIMIT\n", _av[0]);
        return 1; // Return an error code
    }
    
    
    char* input_board = _av[1];
    int turn_board = WHITE;
    if(strcmp(_av[2],"@")==0) turn_board = BLACK;
    else if(strcmp(_av[2],"o")==0) turn_board = WHITE;
    double time_limit_val = atof(_av[3]); //limite de temps en secondes
    if (strlen(input_board) == 18) {
        taille_x = 3;
        taille_y = 6;
        } else {
        taille_x = 10;
        taille_y = 6;
    }
    std::srand(std::time(0));
    
    //initialisation du board
    init(input_board, B);
    
    //génération et affichage du coup
    std::string best_move = genmove(B, turn_board, time_limit_val);
    // Inverser les deux premiers caractères avec les deux derniers
    //if (best_move.size() >= 4) {
    //    std::swap(best_move[0], best_move[2]);
    //    std::swap(best_move[1], best_move[3]);
    //}
    
    printf("%s\n", best_move.c_str());
    
    return 0;
}