package breakthrough;

//Jouer avec Ludii en utilisant un programme Cpp distant via ssh
//Compiler pour obtenir un .class
// $> javac -cp Ludii-XXX.jar PRG.java
//Comme la classe est dans le package bk, créer un rep et placer la classe dedans
// $> mkdir bk
// $> mv PRG.class bk/
//Faire un jar 
// $> jar cf PRG.jar bk/PRG.class
//Lancer Ludii, sélectionner le joueur et jouer
// $> java -jar Ludii-XXX.jar 

// peut être remplacer SuperBKPlayerRem par le nom de votre programme ?

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

public class DHAIRCPlayerRem extends AI
{		
	public final static int EMPTY = 0;
	public final static int BLACK = 1;
	public final static int WHITE = 2;
	public final static boolean LOG_ACTIVE = true;
	public final static String remote_player_str = "remote_DH_AI_RC/DH_RC_AI_mcts_player";
	public final static String remote_host_str = "cu-bocal";
  public final static int BOARDHEIGHT = 6;
  public final static int BOARDWIDTH = 10;

	protected int player = -1; // player_index
	
	public DHAIRCPlayerRem()	{
		this.friendlyName = "DHAIRCPlayerRem";
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
		StringBuffer str_board = new StringBuffer();
		for (final ContainerState containerState : context.state().containerStates()) {
			for(int i = BOARDHEIGHT-1; i >= 0; i--) {
				StringBuffer str_line = new StringBuffer();
				for(int j = 0; j < BOARDWIDTH; j++) {
					int idx = i*BOARDWIDTH+j;
					if(containerState.isEmptyCell(idx)) { /*board[i] = EMPTY;*/ str_line.append("."); }
					else if(containerState.whoCell(idx) == 1) { /*board[i] = BLACK;*/ str_line.append("o"); }
					else if(containerState.whoCell(idx) == 2) { /*board[i] = WHITE;*/ str_line.append("@"); }
				}
				str_board.append(str_line);
			}
		}
		
		String turn = "o";
		if(player==2) turn = "@";
		String genmove_time = "1";
		String res = "";		
		int line_i = -1;
		int col_i = -1;
		int line_f = -1;
		int col_f = -1;
		try {
			//System.err.println("[cmd] "+white_str+" "+black_str);
				Process process = startProcessRemote(remote_host_str,remote_player_str,str_board.toString(),turn,genmove_time);
				res = endProcessRemote(process);
				if(res.length()==4) {
					if(LOG_ACTIVE) {
						System.out.println(remote_player_str+" play "+res);
					}
  		  	line_i = res.charAt(0)-'1';
	  	  	col_i = res.charAt(1)-'a';
		    	line_f = res.charAt(2)-'1';
		    	col_f = res.charAt(3)-'a';
    			if(LOG_ACTIVE) System.err.println("= ("+line_i+" "+col_i+") ("+line_f+" "+col_f+")");
				} else {
					System.err.println("error res.length() "+res.length()+" |"+res+"|");
				}
    } catch(Exception e) { 
      e.printStackTrace();
    }
		if(line_i!=-1 && col_i !=-1 && line_f!=-1 && col_f!=-1) {
  		int pos_i = line_i*BOARDWIDTH+col_i;
	  	int pos_f = line_f*BOARDWIDTH+col_f;
    	if(LOG_ACTIVE) System.err.println("converted to "+pos_i+" "+pos_f);
		  for(int i = 0; i < legalMoves.size(); i++) {
				//System.err.print(legalMoves.get(i).from()+"-"+legalMoves.get(i).to()+" ");
			  if(legalMoves.get(i).from() == pos_i && legalMoves.get(i).to() == pos_f) { System.err.println(""); return legalMoves.get(i);}
		  }
			//System.err.println("");
		}	  
		System.out.println(remote_player_str+" play random");
		return legalMoves.get(0);
	}
	
	@Override
	public void initAI(final Game game, final int playerID)
	{
		this.player = playerID;
	}	
	public Process startProcessRemote(String _host, String _player, String _strboard, String _turn, String _time) throws IOException
	{
		String _ssh = "ssh";
		ProcessBuilder processBuilder = new ProcessBuilder(_ssh, _host, _player, _strboard, _turn, _time);
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
		System.err.println(processErr.toString().trim());
		return processOutput.toString().trim();
	}

}
