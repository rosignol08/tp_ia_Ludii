package ttt;

//Playing with Ludii
//Compile to obtain a .class
// $> javac -cp Ludii-XXX.jar TicTacTocPlayer.java
//As the class is in the ttt package, create a dir and place the class inside
// $> mkdir ttt
// $> mv TicTacTocPlayer.class ttt/
//Make a  jar 
// $> jar cf TicTacTocPlayer.jar ttt/TicTacTocPlayer.class
//Start Ludii, select the player and play with it
// $> 	

import java.util.concurrent.ThreadLocalRandom;

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

public class TicTacTocPlayer extends AI
{		
	public final static int EMPTY = 0;
	public final static int BLACK = 1;
	public final static int WHITE = 2;
	public final static boolean LOG_ACTIVE = false;

	public final static boolean CPP_ACTIVE = false;
	public final static String local_cpp_player_str = "tic_tac_toc_player";
	public final static boolean PYTHON_ACTIVE = true;
	public final static String local_python_player_str = "tic_tac_toc_player.py";
	String local_player_str; // will be set to cpp or python player

	protected int player = -1; // player_index
	
	public TicTacTocPlayer()	{
		this.friendlyName = "TicTacToc";
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
		String res_str = "";
		try {
			if(LOG_ACTIVE) {
				System.out.println("[cmd] "+board_str+" "+turn);
			}
   		res_str = getProcessOutput(board_str, turn);
			if(res_str.length()==1) {
				if(LOG_ACTIVE) {
					System.out.println(local_player_str+" play "+res_str);
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
  	System.out.println(local_player_str+" play random");
		return legalMoves.get(0);
	}
	
	@Override
	public void initAI(final Game game, final int playerID)
	{
		this.player = playerID;
	}	
	public String getProcessOutput(String _board, String _turn) throws IOException, InterruptedException
	{
		if(CPP_ACTIVE) {
			local_player_str = local_cpp_player_str;
		} else if(PYTHON_ACTIVE) {
			local_player_str = local_python_player_str;
		}
		if(CPP_ACTIVE && PYTHON_ACTIVE) {
			System.err.println("WARNING : cpp and Python players are ON !!!");
		}
		if(CPP_ACTIVE==false && PYTHON_ACTIVE==false) {
			System.err.println("WARNING : cpp and Python players are OFF !!!");
			local_player_str = local_python_player_str; // python player = default player
		}

		ProcessBuilder processBuilder = new ProcessBuilder(System.getProperty("user.dir")+"/"+local_player_str,_board,_turn);	
		processBuilder.redirectErrorStream(false);
		Process process = processBuilder.start();
		StringBuilder processOutput = new StringBuilder();
		StringBuilder processErr = new StringBuilder();
		try (BufferedReader processOutputReader = new BufferedReader(
						new InputStreamReader(process.getInputStream()));) {
				String readLine;
				while ((readLine = processOutputReader.readLine()) != null) {
						processOutput.append(readLine + System.lineSeparator());
				}
				process.waitFor();
		}
		try (BufferedReader processOutputReader = new BufferedReader(
						new InputStreamReader(process.getErrorStream()));) {
				String readLine;
				while ((readLine = processOutputReader.readLine()) != null) {
						processErr.append(readLine + System.lineSeparator());
				}
				process.waitFor();
		}		
		//System.err.println(processErr.toString().trim());
		return processOutput.toString().trim();
	}
}
