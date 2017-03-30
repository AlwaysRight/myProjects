import java.util.Random;
//-----------------------------------------
//CLASS:  AIEasy 
//
//Author: Your name, student number
//
//REMARKS: This class implements a simple AI. It can make a valid move
//
//-----------------------------------------

public class AIEasy implements ConnectPlayer {
	Status board[][];

	public AIEasy() {
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
	 * PURPOSE: AI choose a valid column to move.
	 * 
	 * @param lastCol
	 *            the column that the player move last step.
	 * @return the column the AI choose to move.
	 */
	@Override
	public int makeMove(int lastCol) {
		// record the player's move
		int rowPlayer = BoardUtil.getNextRowLocation(board, lastCol);
		board[rowPlayer][lastCol] = Status.ONE;
		int choose = randomChoose();
		int row = BoardUtil.getNextRowLocation(board, choose);
		board[row][choose] = Status.TWO;
		return choose;
	}

	/**
	 * randomChoose
	 * 
	 * PURPOSE: randomly choose a valid column
	 * 
	 * @return the column choose to move
	 */
	public int randomChoose() {
		int randomLength = 0;
		for (int i = 0; i < BoardUtil.WIDTH; i++) {
			if (!BoardUtil.isFullCol(board, i)) {
				randomLength++;
			}
		}
		int canChooseCol[] = new int[randomLength];
		int j = 0;
		for (int i = 0; i < BoardUtil.WIDTH; i++) {
			if (!BoardUtil.isFullCol(board, i)) {
				canChooseCol[j++] = i;
			}
		}
		Random rand = new Random();
		int randIndex = rand.nextInt(randomLength);
		return canChooseCol[randIndex];
	}

}
