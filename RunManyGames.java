import java.util.ArrayList;
import java.util.List;

import game.Game;
import other.AI;
import other.GameLoader;
import other.context.Context;
import other.model.Model;
import other.trial.Trial;
import utils.RandomAI;

import ttt.TicTacTocPlayer;

public class RunManyGames
{
	private static final int NUM_TRIALS_PER_SIDE = 100;
	public static int nb_win = 0;
	public static int nb_lost = 0;
	public static int nb_draw = 0;

	public static void initResults() {
		nb_win = 0;
		nb_lost = 0;
		nb_draw = 0;
	}
	public static void printResults() {
		System.out.println("win/lost/draw : "+nb_win+" "+nb_lost+" "+nb_draw);
	}

	public static void playManyGames(List<AI> ais, int player_id, int opp_id, int nb_games) {
		Game game = GameLoader.loadGameFromName("Tic-Tac-Toe.lud");
		Trial trial = new Trial(game);
		Context context = new Context(game, trial);
		for (int i = 0; i < nb_games; i++) {
			game.start(context);
			for (int p = 1; p <= game.players().count(); ++p) {
				ais.get(p).initAI(game, p);
			}
			Model model = context.model();
			while (!trial.over()) {
				model.startNewStep(context, ais, 1.0);
			}			
			int res = trial.status().winner();
			if(res == 0) { nb_draw ++; }
			else if(res == opp_id) { nb_lost ++; }
			else if(res == player_id) { nb_win ++; }
			else { System.out.println("trial status winner error : "+trial.status().winner()); }
		}
	}

	public static void main(String[] args) {
		List<AI> ais = new ArrayList<AI>();
		ais.add(null);
		ais.add(new TicTacTocPlayer()); // player 1
		ais.add(new RandomAI()); // player 2
    playManyGames(ais, 1, 2, NUM_TRIALS_PER_SIDE);		
		System.out.print("As player 1 : ");
		printResults();
		initResults();
		ais.remove(1);
		ais.add(new TicTacTocPlayer()); // is now player 2
    playManyGames(ais, 2, 1, NUM_TRIALS_PER_SIDE);		
		System.out.print("As player 2 : ");
		printResults();
	}
}