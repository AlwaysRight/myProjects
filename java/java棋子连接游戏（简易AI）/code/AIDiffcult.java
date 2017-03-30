//-----------------------------------------
// CLASS:  AIDiffcult 
//
// Author: Your name, student number
//
// REMARKS: This class implements a smart AI. It can make a smart move
//
//-----------------------------------------

public class AIDiffcult implements ConnectPlayer {
	private final int AI_DEPTH = 7; // the recursion depth
	private final int ALPHA = -1000000;
	private final int BETA = 1000000;
	Status board[][]; // record board to decide a smart move
	private int choose = 0; // store the choose of the AI.

	/**
	 * PURPOSE: Construct a AI, initialize the board..
	 */
	public AIDiffcult() {
		this.board = new Status[BoardUtil.HEIGHT][BoardUtil.WIDTH];
		for (int i = 0; i < BoardUtil.HEIGHT; i++) {
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				this.board[i][j] = Status.NEITHER;
			}
		}
	}

	/**
	 * makeMove
	 * 
	 * PURPOSE: AI choose a smart column to move.
	 * 
	 * @param lastCol the column that the player move last step.
	 * @return the column the AI choose to move.
	 */
	@Override
	public int makeMove(int lastCol) {
		// record the player's move
		int rowPlayer = BoardUtil.getNextRowLocation(board, lastCol);
		board[rowPlayer][lastCol] = Status.ONE;
		
		AIChoose(board, 0, Status.TWO, ALPHA, BETA);
		int row = BoardUtil.getNextRowLocation(board, choose);
		board[row][choose] = Status.TWO;
		
		return this.choose;
	}

	/**
	 * AIChoose
	 * 
	 * PURPOSE: recursion method help to decide the next move.
	 * 
	 * @param curBoard
	 * @param depth
	 * @param curPlayer
	 * @param alpha
	 * @param beta
	 * @return the score of the curBoard.
	 */
	private int AIChoose(Status curBoard[][], int depth, Status curPlayer,
			int alpha, int beta) {
		if (depth > this.AI_DEPTH) {
			return BoardUtil.eval(curBoard, Status.TWO);
		}
		if (BoardUtil.isOver(curBoard)) {
			return BoardUtil.eval(curBoard, Status.TWO);
		}
		int score;
		if (curPlayer == Status.TWO) {
			/*AI player*/
			for (int i = 0; i < BoardUtil.WIDTH; i++) {
				if (!BoardUtil.isFullCol(curBoard, i)) {
					Status[][] newBoard = BoardUtil.getNextBoard(curBoard, i,
							curPlayer);
					score = AIChoose(newBoard, depth + 1, Status.ONE, alpha,
							beta);
					if (score > alpha) {
						alpha = score;
						if (depth == 0) {
							this.choose = i;
						}
					}
					if (beta <= alpha) {
						break;
					}
				}
			}
			return alpha;
		} else {
			/* User player*/
			for (int i = 0; i < BoardUtil.WIDTH; i++) {
				if (!BoardUtil.isFullCol(curBoard, i)) {
					Status[][] newBoard = BoardUtil.getNextBoard(curBoard, i,
							curPlayer);
					score = AIChoose(newBoard, depth + 1, Status.TWO, alpha,
							beta);
					if (score < beta) {
						beta = score;
					}
					if (beta <= alpha) {
						break;
					}
				}
			}
			return beta;
		}
	}

}
