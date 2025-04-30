package breakthrough;

import game.Game;
import main.collections.FastArrayList;
import other.AI;
import other.context.Context;
import other.state.container.ContainerState;
import other.move.Move;
import other.action.others.ActionPass;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;

public class BreakthroughIDSPlayer extends AI
{		
    public final static int EMPTY = 0;
    public final static int BLACK = 1;  // Premier joueur dans Ludii
    public final static int WHITE = 2;  // Deuxième joueur dans Ludii
    public final static boolean LOG_ACTIVE = true;  // Pour le débogage
    public final static String ids_player_str = "DH_RC_AI_ids_player";
    
    protected int player = -1;  // L'indice du joueur
    protected int rows = 6;     // Nombre de lignes du plateau Breakthrough
    protected int cols = 10;    // Nombre de colonnes du plateau Breakthrough
    
    public BreakthroughIDSPlayer()	{
        this.friendlyName = "BT_IDS";
    }
    
    @Override
    public Move selectAction(final Game game, final Context context, 
                             final double maxSeconds, final int maxIterations, 
                             final int maxDepth)
    {
        // Récupérer tous les coups légaux
        FastArrayList<Move> legalMoves = game.moves(context).moves();
        
        if (legalMoves.isEmpty()) {
            // Aucun coup légal, on passe
            return new Move(Game.createPassMove(context, true));
        }
        
        // Initialiser notre plateau
        int[][] board = new int[rows][cols];
        StringBuilder boardStr = new StringBuilder();
        
        // Remplir le tableau avec l'état actuel du jeu
        for (final ContainerState containerState : context.state().containerStates()) {
            for (int pos = 0; pos < rows * cols; pos++) {
                int r = pos / cols;
                int c = pos % cols;
                
                if (containerState.isEmptyCell(pos)) {
                    board[r][c] = EMPTY;
                    boardStr.append(".");
                } else if (containerState.whoCell(pos) == 1) {  // Premier joueur dans Ludii
                    board[r][c] = BLACK;
                    boardStr.append("@");
                } else if (containerState.whoCell(pos) == 2) {  // Deuxième joueur dans Ludii
                    board[r][c] = WHITE;
                    boardStr.append("o");
                }
            }
        }
        
        String boardString = boardStr.toString();
        String turn = (player == 1) ? "@" : "o";  // Convertir l'indice du joueur en symbole
        
        if (LOG_ACTIVE) {
            System.err.println("État du plateau: " + boardString);
            System.err.println("Tour du joueur: " + turn);
        }
        
        try {
            // Appeler le programme externe avec la configuration du plateau et le joueur actuel
            String moveStr = getProcessOutput(cols, rows, boardString, turn, maxSeconds);
            
            if (LOG_ACTIVE) {
                System.err.println("Coup retourné par IDS: " + moveStr);
            }
            
            // Vérifier si le coup est valide
            if (moveStr != null && moveStr.length() == 4) {
                // Format de retour attendu: "LiCiLfCf" (e.g., "5a4b")
                int li = Character.getNumericValue(moveStr.charAt(0));
                char ci = moveStr.charAt(1);
                int lf = Character.getNumericValue(moveStr.charAt(2));
                char cf = moveStr.charAt(3);
                
                // Convertir en indices de tableau
                int from_row = rows - li; // Lignes inversées
                int from_col = ci - 'a';
                int to_row = rows - lf;
                int to_col = cf - 'a';
                
                // Calculer les positions linéaires
                int from_pos = from_row * cols + from_col;
                int to_pos = to_row * cols + to_col;
                
                if (LOG_ACTIVE) {
                    System.err.println("Positions calculées: de " + from_pos + " à " + to_pos);
                }
                
                // Trouver le coup correspondant dans les coups légaux
                for (int i = 0; i < legalMoves.size(); i++) {
                    Move move = legalMoves.get(i);
                    if (move.from() == from_pos && move.to() == to_pos) {
                        if (LOG_ACTIVE) {
                            System.err.println("Coup trouvé dans les coups légaux");
                        }
                        return move;
                    }
                }
                
                // Si aucun coup exact n'est trouvé, vérifier si nous pouvons trouver un coup similaire
                for (int i = 0; i < legalMoves.size(); i++) {
                    Move move = legalMoves.get(i);
                    if (move.to() == to_pos) {
                        if (LOG_ACTIVE) {
                            System.err.println("Coup similaire trouvé par destination");
                        }
                        return move;
                    }
                }
            } else if (moveStr != null && moveStr.equals("-1")) {
                // Code de retour spécial pour indiquer un "pass"
                return new Move(Game.createPassMove(context, true));
            } else {
                System.err.println("Format de coup invalide retourné par IDS: " + moveStr);
            }
        } catch (Exception e) {
            System.err.println("Erreur lors de l'appel au programme IDS:");
            e.printStackTrace();
        }
        
        // En cas d'échec, jouer le premier coup légal
        System.err.println("IDS n'a pas pu générer un coup valide, sélection du premier coup légal");
        return legalMoves.get(0);
    }
    
    @Override
    public void initAI(final Game game, final int playerID)
    {
        this.player = playerID;
        
        // Détecter la taille du plateau à partir du jeu
        // Ceci est une estimation simple, vous pourriez avoir besoin de l'ajuster selon la façon dont Ludii représente le plateau
        int totalSites = game.board().topology().cells().size();
        
        // Ajuster les dimensions en fonction de la taille du plateau
        if (totalSites == 60) {  // 6x10
            rows = 6;
            cols = 10;
        } else if (totalSites == 18) {  // 6x3
            rows = 6;
            cols = 3;
        } else {
            System.err.println("Taille de plateau non reconnue: " + totalSites + " cellules");
        }
        
        if (LOG_ACTIVE) {
            System.err.println("Plateau détecté: " + rows + "x" + cols);
        }
    }
    
    public String getProcessOutput(int taille_x, int taille_y, String boardStr, String turn, double maxTime) 
            throws IOException, InterruptedException {
        // Construire la commande pour exécuter le programme
        ProcessBuilder processBuilder = new ProcessBuilder(
            System.getProperty("user.dir") + "/" + ids_player_str,
            //Integer.toString(taille_x),
            //Integer.toString(taille_y),
            boardStr,
            turn,
            Double.toString(maxTime * 0.95)  // Utiliser 90% du temps maximal pour permettre le retour
        );
        
        processBuilder.redirectErrorStream(false);
        Process process = processBuilder.start();
        
        StringBuilder processOutput = new StringBuilder();
        StringBuilder processError = new StringBuilder();
        
        // Lire la sortie standard
        try (BufferedReader outputReader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
            String line;
            while ((line = outputReader.readLine()) != null) {
                processOutput.append(line).append(System.lineSeparator());
            }
            process.waitFor();
        }
        
        // Lire la sortie d'erreur
        try (BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
            String line;
            while ((line = errorReader.readLine()) != null) {
                processError.append(line).append(System.lineSeparator());
            }
            process.waitFor();
        }
        
        if (LOG_ACTIVE && !processError.toString().isEmpty()) {
            System.err.println("Erreur du programme IDS: " + processError.toString());
        }
        
        return processOutput.toString().trim();
    }
}