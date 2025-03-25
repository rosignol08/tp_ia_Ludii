#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>

#include <string>
#include <iostream>
#include <chrono>
#include "mybt.h"

bt_t B;
int taille_x = 6;
int taille_y = 10;
int branches_count = 0;
bool verbose = true; //debug
//une variable globale pour suivre le temps
std::chrono::steady_clock::time_point start_time;
//@@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo
double time_limit;
void init(char _strboard[], bt_t& board) {
    board.init(taille_x, taille_y);  // Initialisation avec les dimensions

    if (verbose) {
        fprintf(stderr, "Initialisation du plateau %dx%d\n", taille_y, taille_x);
    }
    
    // Le tableau d'entrée est organisé ligne par ligne
    // @ sont les pièces noires (en haut)
    // o sont les pièces blanches (en bas)
    // . sont les cases vides
    /*
      a b c d e f g h i j 
    5 @ @ @ @ @ @ @ @ @ @ 
    4 @ @ @ @ @ @ @ @ @ @ 
    3 . . . . . . . . . . 
    2 . . . . . . . . . . 
    1 o o o o o o o o o o 
    0 o o o o o o o o o o 
    */
   int x, y;
   
    for (x = 0; x < taille_x; x++) {
        for (y = 0; y < taille_y; y++) {
            int index = x * taille_y + y; // Calcul de l'index dans la chaîne
            if (_strboard[index] == '@') {
                board.board[x][y] = BLACK;
            } else if (_strboard[index] == 'o') {
                board.board[x][y] = WHITE;
            } else {
                board.board[x][y] = EMPTY;
            }
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

//fonction heuristique qui prend en compte la taille du plateau
double h(const bt_t& board) {
    double score = 0.0;
    
    //boucle pour chaque position sur le plateau
    for(int i = 0; i < board.nbl; i++) {
        for(int j = 0; j < board.nbc; j++) {
            if(board.board[i][j] == WHITE) {
                //la valeur de base d'une pièce dépend de sa position
                double piece_value = (board.nbl + 1) * (board.nbl - i);
                score += piece_value;
                
                //bonus pour la protection mutuelle
                if(i+1 < board.nbl && j+1 < board.nbc && board.board[i+1][j+1] == WHITE) 
                    score += board.nbl;
                if(i+1 < board.nbl && j-1 >= 0 && board.board[i+1][j-1] == WHITE) 
                    score += board.nbl;
                
                //bonus pour les menaces
                if(i-1 >= 0 && j+1 < board.nbc && board.board[i-1][j+1] == BLACK) 
                    score += 2 * board.nbl;
                if(i-1 >= 0 && j-1 >= 0 && board.board[i-1][j-1] == BLACK) 
                    score += 2 * board.nbl;
            }
            else if(board.board[i][j] == BLACK) {
                //même logique pour les pièces noires
                double piece_value = (board.nbl + 1) * i;
                score -= piece_value;
                
                if(i-1 >= 0 && j+1 < board.nbc && board.board[i-1][j+1] == BLACK) 
                    score -= board.nbl;
                if(i-1 >= 0 && j-1 >= 0 && board.board[i-1][j-1] == BLACK) 
                    score -= board.nbl;
                
                if(i+1 < board.nbl && j+1 < board.nbc && board.board[i+1][j+1] == WHITE) 
                    score -= 2 * board.nbl;
                if(i+1 < board.nbl && j-1 >= 0 && board.board[i+1][j-1] == WHITE) 
                    score -= 2 * board.nbl;
            }
        }
    }
    return score;
}

//Depth Limited Search retourne un bool pour indiquer si la recherche a été complétée
double dls(bt_t& board, int depth, bool& completed) {
    //vérifie d'abord si le temps est écoulé
    if(is_time_expired()) {
        completed = false;
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
        
        // Afficher les coups disponibles pour déboguer
        for (int i = 0; i < board.nb_moves; i++) {
            fprintf(stderr, "Coup %d: (%d,%d) -> (%d,%d)\n", i,
                   board.moves[i].line_i, board.moves[i].col_i,
                   board.moves[i].line_f, board.moves[i].col_f);
        }
    }
    
    // Appliquer IDS pour trouver le meilleur coup
    int reached_depth;
    bt_move_t move = ids(board, time_limit_val, reached_depth);
    
    if (verbose) {
        fprintf(stderr, "Meilleur coup trouvé: (%d,%d) -> (%d,%d)\n",
               move.line_i, move.col_i, move.line_f, move.col_f);
        fprintf(stderr, "Profondeur atteinte: %d\n", reached_depth);
    }
    
    // Conversion du move en notation du premier programme
    // Le format doit être: ligne_départ colonne_départ ligne_arrivée colonne_arrivée
    std::string move_str = std::to_string(move.line_i) + 
                          (char)('a' + move.col_i) + 
                          std::to_string(move.line_f) + 
                          (char)('a' + move.col_f);
    
    if (verbose) {
        fprintf(stderr, "Format de sortie: %s\n", move_str.c_str());
    }
    
    return move_str;
}

int main(int _ac, char** _av) {
    if(_ac != 6) {
        fprintf(stderr, "usage: %s TAILLE_X TAILLE_Y STRBOARD TURN TIME_LIMIT\n", _av[0]);
        return 0;
    }
    
    taille_x = atoi(_av[1]); //recupere la taille x
    taille_y = atoi(_av[2]); //recupere la taille y
    char* input_board = _av[3];
    int turn_board = WHITE;
    if(strcmp(_av[4],"@")==0) turn_board = BLACK;
    else if(strcmp(_av[4],"o")==0) turn_board = WHITE;
    double time_limit_val = atof(_av[5]); // limite de temps en secondes
    
    std::srand(std::time(0));
    
    // Initialisation du board
    init(input_board, B);
    
    // Génération et affichage du coup
    printf("%s\n", genmove(B, turn_board, time_limit_val).c_str());
    
    return 0;
}

/*
./a.out 10 6 @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo @ 1
./a.out 10 6 @@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo o 1
*/