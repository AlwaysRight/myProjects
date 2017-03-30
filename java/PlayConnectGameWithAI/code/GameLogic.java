//-----------------------------------------
// CLASS:  GameLogic 
//
// Author: Your name, student number
//
// REMARKS: This class implements the ConnectController and the game
//          logic, including check the end of the game, make a move..
//
//-----------------------------------------

public class GameLogic implements ConnectController {
	private final int MAXDIFFICULTY = 2; // set the AI count to 2
	private GameDisplay gamedisplay; // use this to control the AI display
	private int difficulty; //Current difficulty of the game
	private ConnectPlayer AI; // the AI player
	Status board[][]; // Record the board state

	/**
	 * GameLogic
	 * 
	 * PURPOSE: Use gamedisplay to construct a game logic class
	 * 
	 * @param gamedisplay
	 */
	public GameLogic(GameDisplay gamedisplay) {
		this.gamedisplay = gamedisplay;
		this.board = new Status[BoardUtil.HEIGHT][BoardUtil.WIDTH];
	}

	/**
	 * getCurrentStatus
	 * 
	 * PURPOSE: Get current game status.
	 * 
	 * @return the Status(winner or a draw)
	 */
	private Status getCurrentStatus() {
		/* check whether a player has win */
		// check for row connection
		Status s = BoardUtil.canConnectRow(board);
		if (s != null) {
			return s;
		}
		// check for column connection
		s = BoardUtil.canConnectColumn(board);
		if (s != null) {
			return s;
		}
		// check for oblique connection
		s = BoardUtil.canConnectOblique(board);
		if (s != null) {
			return s;
		}

		// check if is full
		return BoardUtil.isFull(board);
	}

	/**
	 * checkForEnd
	 * 
	 * Check whether the game is end, inform the GUI.
	 * @return true if the game is end.
	 */
	private boolean checkForEnd() {
		Status curr = this.getCurrentStatus();
		if (curr != null) {
			// End of the game. Tell the GUI
			this.gamedisplay.gameOver(curr);
			return true;
		}
		return false;
	}

	/**
	 * addPiece
	 * 
	 * PURPOSE: player add a piece to a column.
	 * 
	 * @return true if add successfully, otherwise return false
	 */
	@Override
	public boolean addPiece(int col) {
		int i;
		i = BoardUtil.getNextRowLocation(board, col);
		if (i < 0) {
			// column is already full
			return false;
		}
		this.board[i][col] = Status.ONE;
		this.gamedisplay.updateBoard(board);
		if (checkForEnd()) {
			return true;
		}
		int aiCol = AI.makeMove(col);
		int row = BoardUtil.getNextRowLocation(board, aiCol);
		this.board[row][aiCol] = Status.TWO;
		this.gamedisplay.updateBoard(board);
		checkForEnd();
		return true;
	}

	/**
	 * reset
	 * 
	 * PURPOSE: reset the board state, AI player ...
	 */
	@Override
	public void reset() {
		for (int i = 0; i < BoardUtil.HEIGHT; i++) {
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				this.board[i][j] = Status.NEITHER;
			}
		}

		this.difficulty = gamedisplay
				.promptForOpponentDifficulty(MAXDIFFICULTY);
		if (this.difficulty == 1) {
			AI = new AIEasy();
		} else {
			AI = new AIDiffcult();
		}
	}

}
