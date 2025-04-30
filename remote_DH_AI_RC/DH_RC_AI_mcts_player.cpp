#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>
#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

/*
 * Compilation:
 * g++ -Wall -std=c++11 bk_mcts_player.cpp -o bk_mcts_player
 *
 * ./bk_mcts_player BOARD_STR TURN MAX_TIME
 * Exemple pour 6x3: ./bk_mcts_player "@@@@@@@@@...oooooo" o 1.0
 * Exemple pour 6x10: ./bk_mcts_player "@@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo" o 1.0
 *
 * ./bk_mcts_player
 *
 * Dans le mode interactif:
 * - Choisir la taille du plateau (1 pour 6x3, 2 pour 6x10)
 * - L'IA joue les blancs (o)
 */

#define EMPTY 0
#define WHITE 1 // 'o'
#define BLACK 2 // '@'
#define LOG_ACTIVE 0

#define C_PUCT 2.0 // Facteur d'exploration
#define MAX_SIMULATION_MOVES 150
#define CAPTURE_BIAS 0.8        // Probabilité de préférer les captures en simulation
#define HASH_TABLE_SIZE 100000 // Taille maximale de la table de hachage

// Structure pour les mouvements
struct Move
{
    int from; // Position de départ
    int to;   // Position d'arrivée

    Move(int f, int t) : from(f), to(t) {}

    // Opérateur d'égalité pour pouvoir comparer les mouvements
    bool operator==(const Move &other) const
    {
        return from == other.from && to == other.to;
    }
};

// Structure de hachage pour le plateau
struct BoardHash
{
    uint64_t hash;

    BoardHash() : hash(0) {}

    // Calcule le hachage d'un plateau
    void compute(const int *board, int size, int player)
    {
        hash = 0;
        std::hash<int> hasher;

        // Combiner le hash du plateau
        for (int i = 0; i < size; i++)
        {
            hash ^= hasher(board[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2) + i;
        }

        // Ajouter le joueur en cours
        hash ^= hasher(player) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    // Opérateur d'égalité
    bool operator==(const BoardHash &other) const
    {
        return hash == other.hash;
    }
};

// Fonction de hachage pour std::unordered_map
namespace std
{
    template <>
    struct hash<BoardHash>
    {
        size_t operator()(const BoardHash &h) const
        {
            return h.hash;
        }
    };
}

// Structure pour stocker les statistiques d'un état
struct StateStats
{
    int visit_count;
    double value_sum;

    StateStats() : visit_count(0), value_sum(0.0) {}
};

// Table de hachage globale
std::unordered_map<BoardHash, StateStats> transposition_table;

std::string convert_move_to_string(const Move &move, int rows, int cols);

// Afficher les mouvements légaux
void print_legal_moves(const std::vector<Move> &moves, int rows, int cols)
{
    fprintf(stderr, "Mouvements légaux (%zu):\n", moves.size());
    for (size_t i = 0; i < moves.size(); i++)
    {
        int from_row = moves[i].from / cols;
        int from_col = moves[i].from % cols;
        int to_row = moves[i].to / cols;
        int to_col = moves[i].to % cols;

        fprintf(stderr, "  %zu. (%d,%d) -> (%d,%d) = %s\n",
                i + 1, from_row, from_col, to_row, to_col,
                convert_move_to_string(moves[i], rows, cols).c_str());
    }
}

// Classe noeud pour MCTS
class MCTSNode
{
public:
    int board[60];
    int player;
    int rows;
    int cols;
    int visit_count;
    double value_sum;
    std::vector<Move> untried_moves;
    MCTSNode *parent;
    std::vector<MCTSNode *> children;
    std::vector<Move> moves;
    BoardHash hash; // Hachage du nœud

    MCTSNode(int *_board, int _player, int _rows, int _cols, MCTSNode *_parent = nullptr)
        : player(_player), rows(_rows), cols(_cols), visit_count(0), value_sum(0), parent(_parent)
    {
        // Copie du plateau
        for (int i = 0; i < rows * cols; i++)
        {
            board[i] = _board[i];
        }

        // Calcule le hash de l'état
        hash.compute(board, rows * cols, player);

        // Vérifie si l'état existe déjà dans la table de transposition
        auto it = transposition_table.find(hash);
        if (it != transposition_table.end())
        {
            // Utilise les statistiques existantes
            visit_count = it->second.visit_count;
            value_sum = it->second.value_sum;
        }

        // Récupère tous les mouvements légaux
        get_legal_moves();
    }

    ~MCTSNode()
    {
        for (auto child : children)
        {
            delete child;
        }
    }

    void get_legal_moves()
    {
        untried_moves.clear();

        // Direction du mouvement: les blancs montent (direction négative), les noirs descendent (direction positive)
        int direction = (player == WHITE) ? -1 : 1;

        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                int pos = row * cols + col;

                if (board[pos] == player)
                {
                    // Mouvement vers l'avant
                    int new_row = row + direction;
                    if (new_row >= 0 && new_row < rows)
                    {
                        int new_pos = new_row * cols + col;
                        if (board[new_pos] == EMPTY)
                        {
                            untried_moves.push_back(Move(pos, new_pos));
                        }

                        // Mouvements diagonaux
                        for (int dc : {-1, 1})
                        {
                            int new_col = col + dc;
                            if (new_col >= 0 && new_col < cols)
                            {
                                int diag_pos = new_row * cols + new_col;
                                // Déplacement vers une case vide ou capture d'un pion adverse
                                if (board[diag_pos] == EMPTY ||
                                    (board[diag_pos] != EMPTY && board[diag_pos] != player))
                                {
                                    untried_moves.push_back(Move(pos, diag_pos));
                                }
                            }
                        }
                    }
                }
            }
        }

        if (LOG_ACTIVE)
        {
            fprintf(stderr, "get_legal_moves() a trouvé %zu mouvements pour le joueur %d\n",
                    untried_moves.size(), player);
            print_legal_moves(untried_moves, rows, cols);
        }

        // Trie les mouvements (priorité aux captures et aux mouvements vers l'avant)
        if (!untried_moves.empty())
        {
            std::sort(untried_moves.begin(), untried_moves.end(),
                      [this](const Move &a, const Move &b)
                      {
                          bool a_is_capture = (board[a.to] != EMPTY);
                          bool b_is_capture = (board[b.to] != EMPTY);

                          if (a_is_capture != b_is_capture)
                          {
                              return a_is_capture > b_is_capture; // Captures en premier
                          }

                          // Ensuite, priorité aux progrès vers l'avant
                          int a_progress = (player == WHITE) ? (a.from - a.to) : (a.to - a.from);
                          int b_progress = (player == WHITE) ? (b.from - b.to) : (b.to - b.from);
                          return a_progress > b_progress;
                      });
        }

        // Mélange les mouvements pour diversifier l'exploration
        // Ne mélange qu'une partie des mouvements pour maintenir la priorisation
        if (untried_moves.size() > 3)
        {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(untried_moves.begin() + 2, untried_moves.end(), g);
        }
    }

    bool is_terminal(int &winner)
    {
        // Vérifie si un joueur a atteint le bord opposé
        for (int col = 0; col < cols; col++)
        {
            // Noir a atteint la première ligne
            if (board[col] == BLACK)
            {
                winner = BLACK;
                return true;
            }
            // Blanc a atteint la dernière ligne
            if (board[(rows - 1) * cols + col] == WHITE)
            {
                winner = WHITE;
                return true;
            }
        }

        // Vérifie si un joueur n'a plus de pions
        bool has_white = false;
        bool has_black = false;

        for (int i = 0; i < rows * cols; i++)
        {
            if (board[i] == WHITE)
                has_white = true;
            if (board[i] == BLACK)
                has_black = true;
            if (has_white && has_black)
                break;
        }

        if (!has_white)
        {
            winner = BLACK;
            return true;
        }
        if (!has_black)
        {
            winner = WHITE;
            return true;
        }

        return false;
    }

    // Crée un nouvel état après l'application d'un mouvement
    MCTSNode *apply_move(const Move &move)
    {
        int new_board[60];
        for (int i = 0; i < rows * cols; i++)
        {
            new_board[i] = board[i];
        }

        // Déplace la pièce
        new_board[move.to] = new_board[move.from];
        new_board[move.from] = EMPTY;

        // Change de joueur
        int next_player = (player == WHITE) ? BLACK : WHITE;

        // Vérifie si l'état existe déjà parmi les enfants
        BoardHash new_hash;
        new_hash.compute(new_board, rows * cols, next_player);

        for (size_t i = 0; i < children.size(); i++)
        {
            if (children[i]->hash == new_hash)
            {
                return children[i]; // Réutilise le nœud existant
            }
        }

        return new MCTSNode(new_board, next_player, rows, cols, this);
    }

    // Sélectionne le meilleur enfant selon UCT
    MCTSNode *select_best_child()
    {
        double best_score = -1.0;
        MCTSNode *best_child = nullptr;

        for (MCTSNode *child : children)
        {
            if (child->visit_count == 0)
                continue;

            // Calcul du score UCT
            double exploitation = child->value_sum / child->visit_count;
            double exploration = C_PUCT * sqrt(log(visit_count) / child->visit_count);

            // Inverse le terme d'exploitation pour le tour de l'adversaire
            if (child->player != player)
            {
                exploitation = 1.0 - exploitation;
            }

            double score = exploitation + exploration;

            if (score > best_score)
            {
                best_score = score;
                best_child = child;
            }
        }

        if (best_child == nullptr && LOG_ACTIVE)
        {
            fprintf(stderr, "Avertissement: select_best_child n'a trouvé aucun enfant avec visites > 0\n");
            // Si aucun nœud n'a de visites, on prend le premier
            if (!children.empty())
            {
                best_child = children[0];
                fprintf(stderr, "Utilisation du premier enfant disponible\n");
            }
        }

        return best_child;
    }

    MCTSNode *expand()
    {
        // Ne pas étendre trop de nœuds
        if (!children.empty() && children.size() > 3 * sqrt(visit_count))
        {
            return select_best_child();
        }

        if (untried_moves.empty())
            return nullptr;

        // Prend un mouvement non essayé
        Move move = untried_moves.back();
        untried_moves.pop_back();

        // Crée un nouvel état
        MCTSNode *child = apply_move(move);

        // Ajoute l'enfant à la liste
        children.push_back(child);
        moves.push_back(move);

        return child;
    }

    // Simule une partie aléatoire
    int simulate()
    {
        // Copie du plateau et du joueur
        int sim_board[60];
        for (int i = 0; i < rows * cols; i++)
        {
            sim_board[i] = board[i];
        }
        int sim_player = player;

        // Pour éviter les boucles infinies
        int max_moves = MAX_SIMULATION_MOVES;
        int move_count = 0;

        // Simule des coups aléatoires jusqu'à la fin de la partie
        while (move_count < max_moves)
        {
            move_count++;
            int winner = EMPTY;

            // Vérifie si la partie est terminée
            if (is_terminal_state(sim_board, rows, cols, winner))
            {
                return winner;
            }

            // Trouve les mouvements légaux
            std::vector<Move> legal_moves;
            get_legal_moves_for_state(sim_board, sim_player, rows, cols, legal_moves);

            if (legal_moves.empty())
            {
                // Si aucun mouvement légal, l'autre joueur gagne
                return (sim_player == WHITE) ? BLACK : WHITE;
            }

            // Choix d'un mouvement avec préférence pour les captures et avancées
            int random_index = 0;
            if (legal_moves.size() > 1)
            {
                // Identifie les mouvements de capture
                std::vector<int> capture_indices;
                std::vector<int> forward_indices;

                for (size_t i = 0; i < legal_moves.size(); i++)
                {
                    bool is_capture = (sim_board[legal_moves[i].to] != EMPTY);
                    bool is_forward;

                    if (sim_player == WHITE)
                    {
                        is_forward = legal_moves[i].to < legal_moves[i].from; // Blanc monte (indice diminue)
                    }
                    else
                    {
                        is_forward = legal_moves[i].to > legal_moves[i].from; // Noir descend (indice augmente)
                    }

                    if (is_capture)
                    {
                        capture_indices.push_back(i);
                    }
                    else if (is_forward)
                    {
                        forward_indices.push_back(i);
                    }
                }

                // Avec une probabilité CAPTURE_BIAS, sélectionne une capture si disponible,
                // sinon sélectionne un mouvement vers l'avant si disponible
                if (!capture_indices.empty() && ((double)rand() / RAND_MAX) < CAPTURE_BIAS)
                {
                    random_index = capture_indices[rand() % capture_indices.size()];
                }
                else if (!forward_indices.empty() && ((double)rand() / RAND_MAX) < CAPTURE_BIAS)
                {
                    random_index = forward_indices[rand() % forward_indices.size()];
                }
                else
                {
                    random_index = rand() % legal_moves.size();
                }
            }

            Move random_move = legal_moves[random_index];

            // Applique le mouvement
            sim_board[random_move.to] = sim_board[random_move.from];
            sim_board[random_move.from] = EMPTY;

            // Change de joueur
            sim_player = (sim_player == WHITE) ? BLACK : WHITE;
        }

        // Si nombre maximal de coups atteint, c'est un match nul
        return EMPTY;
    }

    // Met à jour les statistiques du noeud
    void update(int winner)
    {
        visit_count++;

        // Mise à jour de la valeur selon le gagnant
        if (winner == player)
        {
            value_sum += 1.0;
        }
        else if (winner == EMPTY)
        {
            value_sum += 0.5; // Match nul (rare)
        }

        // Met à jour la table de transposition
        StateStats &stats = transposition_table[hash];
        stats.visit_count = visit_count;
        stats.value_sum = value_sum;

        // Limite la taille de la table de transposition
        if (transposition_table.size() > HASH_TABLE_SIZE)
        {
            // Supprime 10% des entrées les moins utilisées
            std::vector<BoardHash> to_remove;

            for (auto &entry : transposition_table)
            {
                if (entry.second.visit_count < 5)
                { // Supprime les entrées peu visitées
                    to_remove.push_back(entry.first);
                    if (to_remove.size() >= HASH_TABLE_SIZE / 10)
                        break;
                }
            }

            for (const auto &key : to_remove)
            {
                transposition_table.erase(key);
            }
        }
    }

    // Fonctions utilitaires statiques
    static bool is_terminal_state(int *board, int rows, int cols, int &winner)
    {
        // Vérifie si un joueur a atteint le bord opposé
        for (int col = 0; col < cols; col++)
        {
            // Noir a atteint la première ligne
            if (board[col] == BLACK)
            {
                winner = BLACK;
                return true;
            }
            // Blanc a atteint la dernière ligne
            if (board[(rows - 1) * cols + col] == WHITE)
            {
                winner = WHITE;
                return true;
            }
        }

        // Vérifie si un joueur n'a plus de pions
        bool has_white = false;
        bool has_black = false;

        for (int i = 0; i < rows * cols; i++)
        {
            if (board[i] == WHITE)
                has_white = true;
            if (board[i] == BLACK)
                has_black = true;
            if (has_white && has_black)
                break;
        }

        if (!has_white)
        {
            winner = BLACK;
            return true;
        }
        if (!has_black)
        {
            winner = WHITE;
            return true;
        }

        return false;
    }

    static void get_legal_moves_for_state(int *board, int player, int rows, int cols, std::vector<Move> &legal_moves)
    {
        legal_moves.clear();

        // Direction du mouvement: les blancs montent (direction négative), les noirs descendent (direction positive)
        int direction = (player == WHITE) ? -1 : 1;

        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                int pos = row * cols + col;

                if (board[pos] == player)
                {
                    // Mouvement vers l'avant
                    int new_row = row + direction;
                    if (new_row >= 0 && new_row < rows)
                    {
                        int new_pos = new_row * cols + col;
                        if (board[new_pos] == EMPTY)
                        {
                            legal_moves.push_back(Move(pos, new_pos));
                        }

                        // Mouvements diagonaux
                        for (int dc : {-1, 1})
                        {
                            int new_col = col + dc;
                            if (new_col >= 0 && new_col < cols)
                            {
                                int diag_pos = new_row * cols + new_col;
                                // Déplacement vers une case vide ou capture d'un pion adverse
                                if (board[diag_pos] == EMPTY ||
                                    (board[diag_pos] != EMPTY && board[diag_pos] != player))
                                {
                                    legal_moves.push_back(Move(pos, diag_pos));
                                }
                            }
                        }
                    }
                }
            }
        }

        // Trie les mouvements pour prioriser les captures et les mouvements vers l'avant
        if (!legal_moves.empty())
        {
            std::sort(legal_moves.begin(), legal_moves.end(),
                      [board, player](const Move &a, const Move &b)
                      {
                          bool a_is_capture = (board[a.to] != EMPTY);
                          bool b_is_capture = (board[b.to] != EMPTY);

                          if (a_is_capture != b_is_capture)
                          {
                              return a_is_capture > b_is_capture; // Captures en premier
                          }

                          // Ensuite, priorité aux progrès vers l'avant
                          int a_progress = (player == WHITE) ? (a.from - a.to) : (a.to - a.from);
                          int b_progress = (player == WHITE) ? (b.from - b.to) : (b.to - b.from);
                          return a_progress > b_progress;
                      });

            // Ajoute un peu d'aléatoire pour la variété
            if (legal_moves.size() > 3)
            {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(legal_moves.begin() + 2, legal_moves.end(), g);
            }
        }
    }
};

// Convertir l'indice du tableau en format LI-CI-LF-CF
// Les lignes sont numérotées de 1 (bas) à 6 (haut)
// Les colonnes sont étiquetées de a (gauche) à j (droite) pour 6x10
std::string convert_move_to_string(const Move &move, int rows, int cols)
{
    // Position de départ
    int from_row = move.from / cols;
    int from_col = move.from % cols;

    // Position d'arrivée
    int to_row = move.to / cols;
    int to_col = move.to % cols;

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

// Fonction pour convertir une chaîne de format LI-CI-LF-CF en structure Move
Move convert_string_to_move(const std::string &move_str, int rows, int cols)
{
    int li = move_str[0] - '0';
    char ci = move_str[1];
    int lf = move_str[2] - '0';
    char cf = move_str[3];

    // Convertir les coordonnées au format interne
    int from_row = rows - li;
    int from_col = ci - 'a';
    int to_row = rows - lf;
    int to_col = cf - 'a';

    // Créer le mouvement
    return Move(from_row * cols + from_col, to_row * cols + to_col);
}

// Fonction pour appliquer un mouvement au plateau
void apply_move_to_board(int *board, const Move &move)
{
    board[move.to] = board[move.from];
    board[move.from] = EMPTY;
}

bool check_for_immediate_win(int *board, int player, int rows, int cols, std::string &winning_move)
{
    std::vector<Move> legal_moves;
    MCTSNode::get_legal_moves_for_state(board, player, rows, cols, legal_moves);

    for (const Move &move : legal_moves)
    {
        // Sauvegarde l'état du plateau
        int piece_from = board[move.from];
        int piece_to = board[move.to];

        // Applique temporairement le mouvement
        board[move.to] = piece_from;
        board[move.from] = EMPTY;

        // Vérifie la victoire
        int winner = EMPTY;
        bool is_winner = MCTSNode::is_terminal_state(board, rows, cols, winner) && winner == player;

        // Rétablit l'état initial du plateau
        board[move.from] = piece_from;
        board[move.to] = piece_to;

        if (is_winner)
        {
            winning_move = convert_move_to_string(move, rows, cols);
            return true;
        }
    }

    return false;
}

// Fonction principale MCTS
std::string mcts_genmove(int *board, int player, int rows, int cols, double max_time)
{
    auto start_time = std::chrono::steady_clock::now();

    // Réinitialise la table de transposition si elle devient trop grande
    if (transposition_table.size() > HASH_TABLE_SIZE / 2)
    {
        transposition_table.clear();
    }

    // Vérifie d'abord s'il y a des coups gagnants immédiats
    std::string winning_move;
    if (check_for_immediate_win(board, player, rows, cols, winning_move))
    {
        if (LOG_ACTIVE)
        {
            fprintf(stderr, "Coup gagnant immédiat trouvé : %s\n", winning_move.c_str());
        }
        return winning_move;
    }

    // Crée le noeud racine
    MCTSNode root(board, player, rows, cols);

    // Vérifier si des mouvements légaux existent
    if (root.untried_moves.empty())
    {
        if (LOG_ACTIVE)
        {
            fprintf(stderr, "Aucun mouvement légal trouvé!\n");
        }
        return "-1";
    }

    // Vérifie le début de l'algorithme
    if (LOG_ACTIVE)
    {
        fprintf(stderr, "MCTS avec %zu mouvements initiaux\n", root.untried_moves.size());
    }

    int iterations = 0;

    // Vérification du temps adaptative
    int time_check_interval = 10;

    // Exécute MCTS jusqu'à épuisement du temps
    int max_iterations = 100000; // Limite le nombre d'itérations pour éviter les boucles infinies
    while (iterations < max_iterations)
    {
        // Vérification périodique du temps (adaptative)
        if (iterations % time_check_interval == 0)
        {
            auto current_time = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(current_time - start_time).count();

            if (elapsed >= max_time * 0.95)
                break; // 95% du temps pour éviter les dépassements

            // Met à jour l'intervalle de vérification (vérifie moins souvent avec plus d'itérations)
            if (iterations > 1000)
            {
                time_check_interval = 100;
            }
            else if (iterations > 100)
            {
                time_check_interval = 20;
            }
        }

        // Sélection: descend dans l'arbre en choisissant les meilleurs noeuds
        MCTSNode *node = &root;
        while (node->untried_moves.empty() && !node->children.empty())
        {
            node = node->select_best_child();
            if (node == nullptr)
            {
                if (LOG_ACTIVE)
                    fprintf(stderr, "select_best_child() a retourné nullptr!\n");
                break;
            }
        }

        // Expansion: ajoute un nouvel enfant si possible
        int winner = EMPTY;
        if (!node->is_terminal(winner) && !node->untried_moves.empty())
        {
            node = node->expand();
        }

        // Simulation: joue une partie aléatoire à partir de ce noeud
        if (!node->is_terminal(winner))
        {
            winner = node->simulate();
        }

        // Backpropagation: met à jour les statistiques en remontant l'arbre
        while (node != nullptr)
        {
            node->update(winner);
            node = node->parent;
        }

        iterations++;
    }

    // Trouve le meilleur mouvement (celui avec le plus de visites)
    if (root.children.empty())
    {
        // Aucun mouvement possible
        return "-1";
    }

    int best_visits = -1;
    int best_move_index = -1;

    for (size_t i = 0; i < root.children.size(); i++)
    {
        if (root.children[i]->visit_count > best_visits)
        {
            best_visits = root.children[i]->visit_count;
            best_move_index = i;
        }
    }

    if (LOG_ACTIVE)
    {
        auto end_time = std::chrono::steady_clock::now();
        double total_time = std::chrono::duration<double>(end_time - start_time).count();
        fprintf(stderr, "MCTS terminé - itérations: %d, temps: %.2f s\n", iterations, total_time);

        // Affiche les statistiques des meilleurs mouvements
        for (size_t i = 0; i < root.children.size(); i++)
        {
            Move move = root.moves[i];
            std::string move_str = convert_move_to_string(move, rows, cols);
            fprintf(stderr, "Mouvement %s : visites=%d, valeur=%.3f\n",
                    move_str.c_str(),
                    root.children[i]->visit_count,
                    root.children[i]->value_sum / root.children[i]->visit_count);
        }
    }

    // Retourne le meilleur mouvement au format LI-CI-LF-CF
    return convert_move_to_string(root.moves[best_move_index], rows, cols);
}

void init(char *_strboard, int *_board, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (_strboard[i] == '.')
            _board[i] = EMPTY;
        if (_strboard[i] == '@')
            _board[i] = BLACK; // Noir
        if (_strboard[i] == 'o')
            _board[i] = WHITE; // Blanc
    }
}

void fprint_board(FILE *_out, int *_board, int rows, int cols)
{
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            int pos = row * cols + col;
            if (_board[pos] == EMPTY)
                fprintf(_out, ". ");
            else if (_board[pos] == WHITE)
                fprintf(_out, "o ");
            else
                fprintf(_out, "@ ");
        }
        fprintf(_out, "\n");
    }
}

std::string genmove(int *_board, int _color, int rows, int cols, double max_time)
{
    if (LOG_ACTIVE)
    {
        fprintf(stderr, "--- genmove\n");
        fprintf(stderr, "board :\n");
        fprint_board(stderr, _board, rows, cols);
        fprintf(stderr, "color : %d\n", _color);
    }

    // Vérifier d'abord s'il y a des mouvements légaux directs
    MCTSNode temp(_board, _color, rows, cols);
    if (temp.untried_moves.empty())
    {
        if (LOG_ACTIVE)
        {
            fprintf(stderr, "Aucun mouvement légal trouvé directement!\n");
        }
        return "-1";
    }

    if (LOG_ACTIVE)
    {
        fprintf(stderr, "Nombre de mouvements légaux: %zu\n", temp.untried_moves.size());
        // Prenons juste le premier mouvement légal si MCTS échoue
        if (!temp.untried_moves.empty())
        {
            fprintf(stderr, "Premier mouvement légal: %s\n",
                    convert_move_to_string(temp.untried_moves[0], rows, cols).c_str());
        }
    }

    // Essayons de faire MCTS maintenant
    std::string mcts_move = mcts_genmove(_board, _color, rows, cols, max_time);

    // Si MCTS échoue, prenons simplement le premier mouvement légal
    if (mcts_move == "-1" && !temp.untried_moves.empty())
    {
        if (LOG_ACTIVE)
        {
            fprintf(stderr, "MCTS a échoué, utilisation du premier mouvement légal\n");
        }
        mcts_move = convert_move_to_string(temp.untried_moves[0], rows, cols);
    }

    return mcts_move;
}

// Fonction pour afficher le plateau
void print_board(int *board, int rows, int cols)
{
    std::cout << "\nPlateau actuel:\n";
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            int pos = row * cols + col;
            if (board[pos] == EMPTY)
                std::cout << ". ";
            else if (board[pos] == WHITE)
                std::cout << "o ";
            else
                std::cout << "@ ";
        }
        std::cout << " | " << (rows - row) << std::endl;
    }

    // Affiche les lettres des colonnes
    std::cout << "--";
    for (int col = 0; col < cols; col++)
    {
        std::cout << "--";
    }
    std::cout << "\n  ";
    for (int col = 0; col < cols; col++)
    {
        std::cout << (char)('a' + col) << " ";
    }
    std::cout << std::endl;
}

// Fonction pour convertir le plateau en chaîne de caractères
std::string board_to_string(int *board, int size)
{
    std::string result;
    for (int i = 0; i < size; i++)
    {
        if (board[i] == EMPTY)
            result += '.';
        else if (board[i] == WHITE)
            result += 'o';
        else
            result += '@';
    }
    return result;
}

// Mode interactif permettant de jouer coup par coup
void interactive_mode()
{
    int rows, cols;
    int board_size;
    int board[60]; // Taille maximale possible

    std::cout << "Choisissez la taille du plateau (1 pour 6x3, 2 pour 6x10): ";
    int choice;
    std::cin >> choice;

    if (choice == 1)
    {
        rows = 6;
        cols = 3;
        board_size = rows * cols;

        // Initialisation du plateau 6x3
        const char *initial_board = "@@@@@@@@@...oooooo";
        init(const_cast<char *>(initial_board), board, board_size);
    }
    else
    {
        rows = 6;
        cols = 10;
        board_size = rows * cols;

        // Initialisation du plateau 6x10
        const char *initial_board = "@@@@@@@@@@@@@@@@@@@@....................oooooooooooooooooooo";
        init(const_cast<char *>(initial_board), board, board_size);
    }

    // Affichage du plateau initial
    print_board(board, rows, cols);

    // Boucle de jeu
    int current_player = WHITE; // Le blanc commence
    double time_limit = 1.0;    // Temps limite en secondes
    bool game_over = false;

    while (!game_over)
    {
        std::cout << "\nAu tour de " << (current_player == WHITE ? "Blanc (o)" : "Noir (@)") << std::endl;

        std::string move_str;

        if (current_player == WHITE)
        {
            // IA joue pour le blanc
            std::cout << "L'IA réfléchit...\n";
            move_str = genmove(board, current_player, rows, cols, time_limit);
            std::cout << "L'IA joue: " << move_str << std::endl;
        }
        else
        {
            // Humain joue pour le noir
            std::cout << "Entrez votre coup (format LI-CI-LF-CF, par exemple 5a4a): ";
            std::cin >> move_str;

            // Vérification du format
            if (move_str.length() != 4)
            {
                std::cout << "Format invalide! Réessayez.\n";
                continue;
            }
        }

        // Appliquer le mouvement
        Move move = convert_string_to_move(move_str, rows, cols);
        apply_move_to_board(board, move);

        // Afficher le plateau mis à jour
        print_board(board, rows, cols);

        // Vérifier si la partie est terminée
        int winner = EMPTY;
        if (MCTSNode::is_terminal_state(board, rows, cols, winner))
        {
            std::cout << "\n*** FIN DE LA PARTIE ***\n";
            if (winner == WHITE)
            {
                std::cout << "Le joueur Blanc (o) a gagné!\n";
            }
            else
            {
                std::cout << "Le joueur Noir (@) a gagné!\n";
            }
            game_over = true;
        }

        // Passer au joueur suivant
        current_player = (current_player == WHITE) ? BLACK : WHITE;
    }
}

int main(int _ac, char **_av)
{
    // Initialisation du générateur aléatoire
    std::srand(std::time(0));

    if (_ac == 1)
    {
        // Mode interactif
        interactive_mode();
        return 0;
    }
    else if (_ac == 4)
    {
        // Mode normal (pour le tournoi)
        char *input_board = _av[1];
        char turn = _av[2][0];
        double max_time = atof(_av[3]);

        int turn_board = WHITE;
        if (turn == '@')
            turn_board = BLACK;

        // Détermine les dimensions du plateau
        int rows, cols;
        int len = strlen(input_board);

        if (len == 60)
        {
            rows = 6;
            cols = 10;
        }
        else if (len == 18)
        {
            rows = 6;
            cols = 3;
        }
        else if (len == 12)
        {
            // Format compacté pour le plateau 6x3
            rows = 6;
            cols = 3;
            // Note: assume l'ordre correct des caractères
        }
        else
        {
            fprintf(stderr, "Taille de plateau non prise en charge: %d\n", len);
            return 1;
        }

        // Initialise le plateau
        int board[60]; // Taille maximale possible
        init(input_board, board, len);

        // Génère et affiche le meilleur coup
        std::string best_move = genmove(board, turn_board, rows, cols, max_time);
        printf("%s\n", best_move.c_str());

        return 0;
    }
    else
    {
        fprintf(stderr, "usage: %s [BOARD_STR TURN MAX_TIME] ou sans arguments pour le mode interactif\n", _av[0]);
        return 1;
    }
}