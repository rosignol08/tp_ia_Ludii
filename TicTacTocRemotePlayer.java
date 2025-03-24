package ttt;

//Jouer avec Ludii
//Compiler pour obtenir un .class
// $> javac -cp Ludii-XXX.jar TicTacTocRemotePlayer.java
//Comme la classe est dans le package ttt, créer un rep et placer la classe dedans
// $> mkdir ttt
// $> mv TicTacTocRemotePlayer.class ttt/
//Faire un jar 
// $> jar cf TicTacTocPlayer.jar ttt/TicTacTocRemotePlayer.class
//Lancer Ludii, sélectionner le joueur et jouer
// $> java -jar Ludii-XXX.jar 

import java.util.concurrent.ThreadLocalRandom;

import game.Game;
import main.collections.FastArrayList;
import other.AI;
import other.context.Context;
import other.state.container.ContainerState;
import other.move.Move;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Map;
import java.util.HashMap;

public class TicTacTocRemotePlayer extends AI
{		
	public final static int EMPTY = 0;
	public final static int BLACK = 1;
	public final static int WHITE = 2;
	public final static boolean LOG_ACTIVE = true;
	public final static String remote_player_str = "ttt-2025/tic_tac_toc_remote_player.sh";
	public final static String remote_host_str = "mil-www";

	protected int player = -1; // player_index
	
	public TicTacTocRemotePlayer()	{
		this.friendlyName = "TicTacTocRemote";
	}
	
	@Override
	public Move selectAction
	(
		final Game game, 
		final Context context, 
		final double maxSeconds,
		final int maxIterations,
		final int maxDepth
	)
	{
		FastArrayList<Move> legalMoves = game.moves(context).moves();
    int board[] = new int[9];
		StringBuilder board_sb = new StringBuilder();
		board_sb.append("........."); 
		for (final ContainerState containerState : context.state().containerStates()) {
			for(int i = 0; i < 9; i++) {
				if(containerState.isEmptyCell(i)) {
					board[i] = EMPTY;
				}
				if(containerState.whoCell(i) == 2) {
					board[i] = BLACK;
				}
				if(containerState.whoCell(i) == 1) {
					board[i] = WHITE;
				}
			}
		}
/*	inverse board representation
 *  (Self)      (Ludii)
 *   012         678
 *	 345    and  345
 *	 678         012
 */
		for(int i = 0; i < 3; i++) {
			if(board[i] == BLACK) board_sb.setCharAt(i+6, 'x');
			if(board[i] == WHITE) board_sb.setCharAt(i+6, 'o');
			if(board[i+3] == BLACK) board_sb.setCharAt(i+3, 'x');
			if(board[i+3] == WHITE) board_sb.setCharAt(i+3, 'o');
			if(board[i+6] == BLACK) board_sb.setCharAt(i, 'x');
			if(board[i+6] == WHITE) board_sb.setCharAt(i, 'o');
		}
		String board_str = board_sb.toString();
		String turn = "o";
		if(player==2) turn = "x";
		String genmove_time = ""+0.98*maxSeconds;
		String res_str = "";
		try {
			if(LOG_ACTIVE) {
				System.out.println("[cmd] "+board_str+" "+turn);
			}
			Process process = startProcessRemote(remote_host_str,remote_player_str,board_str, turn, genmove_time);
			res_str = endProcessRemote(process);
			if(res_str.length()==1) {
				if(LOG_ACTIVE) {
					System.out.println(remote_player_str+" play "+res_str);
				}
				int res = Integer.parseInt(res_str);
				if(LOG_ACTIVE) {
					if(player==2) {
						board_sb.setCharAt(res, 'x');
					} else {
						board_sb.setCharAt(res, 'o');
					}
					System.out.println("[tictactoc] "+board_sb.toString());
				}   			
				// from Self to Ludii representation
				if(res < 3) res+=6;
				else if(res > 5) res-=6;
				// for Ludii, tictactoe moves have equal initial and final move index
				// let's take final move index
				for(int i = 0; i < legalMoves.size(); i++) {
				  if(legalMoves.get(i).to() == res) { return legalMoves.get(i);}
  		  }
 			} else {
				int res = Integer.parseInt(res_str);
				if(res == -1) {
		  		return new Move(Game.createPassMove(context,true));
				} else {
					System.err.println("error --"+res_str+"-- res_str.length() "+res_str.length());
				}
			}
    } catch(Exception e) { 
      e.printStackTrace();
    }
  	System.out.println(remote_player_str+" play random");
		return legalMoves.get(0);
	}
	
	@Override
	public void initAI(final Game game, final int playerID)
	{
		this.player = playerID;
	}	
	public Process startProcessRemote(String _host, String _player, String _board, String _turn, String _time) throws IOException
	{
		String _ssh = "ssh";
		ProcessBuilder processBuilder = new ProcessBuilder(_ssh, _host, _player,_board,_turn,_time);
		processBuilder.redirectErrorStream(false);
		try {
			return processBuilder.start();
		} catch(Exception e) { System.err.println(e); }
		return null;

	}
	public String endProcessRemote(Process process) throws IOException, InterruptedException
	{
		StringBuilder processOutput = new StringBuilder();
		StringBuilder processErr = new StringBuilder();
		try (BufferedReader processOutputReader = new BufferedReader(
						new InputStreamReader(process.getInputStream()));) {
				String readLine;
				while ((readLine = processOutputReader.readLine()) != null) {
						processOutput.append(readLine + System.lineSeparator());
				}
				process.waitFor();
		} catch(Exception e) { System.err.println(e); } 
		try (BufferedReader processOutputReader = new BufferedReader(
						new InputStreamReader(process.getErrorStream()));) {
				String readLine;
				while ((readLine = processOutputReader.readLine()) != null) {
						processErr.append(readLine + System.lineSeparator());
				}
				process.waitFor();
		} catch(Exception e) { System.err.println(e); } 		
		//System.err.println(processErr.toString().trim());
		//System.err.println("getProcessOutput end ---\n");
		return processOutput.toString().trim();
	}

}
