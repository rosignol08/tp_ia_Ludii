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

public class BreakthroughMCTSPlayer extends AI
{		
    public final static int EMPTY = 0;
    public final static int BLACK = 1;  // Premier joueur dans Ludii
    public final static int WHITE = 2;  // Deuxième joueur dans Ludii
    public final static boolean LOG_ACTIVE = true;  // Pour le débogage
    public final static String mcts_player_str = "DH_RC_AI_mcts_player";
    
    protected int player = -1;  // L'indice du joueur
    
    public BreakthroughMCTSPlayer()	{
        this.friendlyName = "BT_MCTS";
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
        
        // Récupérer la taille du plateau
        // Note: c'est une approximation, peut nécessiter des ajustements selon comment Ludii représente le plateau
        int boardSize = game.board().topology().cells().size();
        String boardStr = convertBoardToString(context);
        String turn = (player == 1) ? "@" : "o";  // Convertir l'indice du joueur en symbole
        
        if (LOG_ACTIVE) {
            System.err.println("État du plateau: " + boardStr);
            System.err.println("Tour du joueur: " + turn);
            System.err.println("Taille du plateau: " + boardSize);
        }
        
        try {
            // Appeler le programme MCTS externe
            String moveStr = getProcessOutput(boardStr, turn, maxSeconds);
            
            if (LOG_ACTIVE) {
                System.err.println("Coup retourné par MCTS: " + moveStr);
            }
            
            // Vérifier si le coup est valide
            if (moveStr != null && moveStr.length() == 4) {
                // Le format attendu est "LiCiLfCf" (ex: "5a4b")
                // Li = ligne initiale, Ci = colonne initiale, Lf = ligne finale, Cf = colonne finale
                
                // Analyser le coup retourné
                int li = Character.getNumericValue(moveStr.charAt(0));
                char ci = moveStr.charAt(1);
                int lf = Character.getNumericValue(moveStr.charAt(2));
                char cf = moveStr.charAt(3);
                
                // Déterminer les dimensions du plateau
                int cols = (boardSize == 60) ? 10 : (boardSize == 18) ? 3 : 10;  // Défaut à 10 si inconnu
                int rows = boardSize / cols;
                
                // Convertir en indices de tableau (selon la représentation de Ludii)
                int from_row = rows - li; // Inverser car les lignes sont numérotées du bas vers le haut
                int from_col = ci - 'a';  // Convertir de lettre à indice (a=0, b=1, etc.)
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
                
                // Si aucun coup exact n'est trouvé, essayons de trouver un coup similaire
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
                System.err.println("Format de coup invalide retourné par MCTS: " + moveStr);
            }
        } catch (Exception e) {
            System.err.println("Erreur lors de l'appel au programme MCTS:");
            e.printStackTrace();
        }
        
        // En cas d'échec, jouer le premier coup légal
        System.err.println("MCTS n'a pas pu générer un coup valide, sélection du premier coup légal");
        return legalMoves.get(0);
    }
    
    /**
     * Convertit l'état du plateau en chaîne de caractères.
     */
    private String convertBoardToString(Context context) {
        StringBuilder boardStr = new StringBuilder();
        int boardSize = context.game().board().topology().cells().size();
        int cols = (boardSize == 60) ? 10 : (boardSize == 18) ? 3 : 10; // Défaut à 10 si inconnu
        int rows = boardSize / cols;
        
        // Créer un tableau temporaire
        char[] board = new char[boardSize];
        for (int i = 0; i < boardSize; i++) {
            board[i] = '.'; // Par défaut, toutes les cases sont vides
        }
        
        // Remplir le tableau avec l'état actuel
        for (final ContainerState containerState : context.state().containerStates()) {
            for (int pos = 0; pos < boardSize; pos++) {
                if (!containerState.isEmptyCell(pos)) {
                    if (containerState.whoCell(pos) == 1) {
                        board[pos] = '@'; // Premier joueur (noir)
                    } else if (containerState.whoCell(pos) == 2) {
                        board[pos] = 'o'; // Deuxième joueur (blanc)
                    }
                }
            }
        }
        
        // Convertir le tableau en chaîne
        for (char c : board) {
            boardStr.append(c);
        }
        
        return boardStr.toString();
    }
    
    @Override
    public void initAI(final Game game, final int playerID)
    {
        this.player = playerID;
        
        if (LOG_ACTIVE) {
            System.err.println("BreakthroughMCTSPlayer initialisé pour le joueur " + playerID);
            
            // Détecter et afficher la taille du plateau
            int boardSize = game.board().topology().cells().size();
            int cols = (boardSize == 60) ? 10 : (boardSize == 18) ? 3 : 10;
            int rows = boardSize / cols;
            System.err.println("Taille du plateau détectée: " + rows + "x" + cols + " (" + boardSize + " cases)");
        }
    }
    
    public String getProcessOutput(String boardStr, String turn, double maxTime) 
            throws IOException, InterruptedException {
        // Construire la commande pour exécuter le programme
        ProcessBuilder processBuilder = new ProcessBuilder(
            System.getProperty("user.dir") + "/" + mcts_player_str,
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
            System.err.println("Erreur du programme MCTS: " + processError.toString());
        }
        
        return processOutput.toString().trim();
    }
}