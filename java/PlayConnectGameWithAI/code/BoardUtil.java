//-----------------------------------------
// CLASS:  BoardUtil 
//
// Author: Your name, student number
//
// REMARKS: This class is deal with board array, including make grade
//			for the current state, check whether the game is end....
//
//-----------------------------------------

public class BoardUtil {
	static final int WIDTH = 7;
	static final int HEIGHT = 6;
	static final int CONNECT = 4;
	private final static int WINSCORE = 1000;
	private final static int SINGLECONNECTSCORE = 5;

	/**
	 * isOver
	 * 
	 * PURPOSE: Check whether a board is over
	 * 
	 * @param board
	 * @return true if the game board is over
	 */
	public static boolean isOver(Status board[][]) {
		if (BoardUtil.canConnectColumn(board) != null
				|| BoardUtil.canConnectRow(board) != null
				|| BoardUtil.canConnectOblique(board) != null
				|| BoardUtil.isFull(board) != null) {
			return true;
		}
		return false;
	}

	/**
	 * canConnectRow
	 * 
	 * PURPOSE: Check whether the row can connect 4 pieces.
	 * 
	 * @param board
	 *            the board to check.
	 * @return the status that connect 4 or null if no connect
	 */
	public static Status canConnectRow(Status board[][]) {
		for (int i = BoardUtil.HEIGHT - 1; i >= 0; i--) {
			int connectNum = 0;
			Status pre = Status.NEITHER;
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				if (board[i][j] == Status.NEITHER) {
					connectNum = 0;
					pre = Status.NEITHER;
					continue;
				} else {
					if (pre == board[i][j]) {
						connectNum++;
						if (connectNum == CONNECT) {
							return board[i][j];
						}
					} else {
						connectNum = 1;
						pre = board[i][j];
					}
				}
			}
		}
		return null;
	}

	/**
	 * canConnectColumn
	 * 
	 * PURPOSE: Check whether a column can connect 4 pieces.
	 * 
	 * @param board
	 *            the board to check.
	 * @return the status that connect 4 or null if no connect
	 */
	public static Status canConnectColumn(Status board[][]) {
		for (int i = 0; i < BoardUtil.WIDTH; i++) {
			int connectNum = 0;
			Status pre = Status.NEITHER;
			for (int j = BoardUtil.HEIGHT - 1; j >= 0; j--) {
				if (board[j][i] == Status.NEITHER) {
					connectNum = 0;
					pre = Status.NEITHER;
					continue;
				} else {
					if (pre == board[j][i]) {
						connectNum++;
						if (connectNum == CONNECT) {
							return board[j][i];
						}
					} else {
						connectNum = 1;
						pre = board[j][i];
					}
				}
			}
		}
		return null;
	}

	/**
	 * canConnectOblique
	 * 
	 * PURPOSE: Check whether the oblique line can connect 4 pieces.
	 * 
	 * @param board
	 *            the board to check.
	 * @return the status that connect 4 or null if no connect
	 */
	public static Status canConnectOblique(Status board[][]) {
		for (int i = BoardUtil.HEIGHT - 1; i >= 0; i--) {
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				Status s = board[i][j];
				if (s == Status.NEITHER) {
					continue;
				}
				if (i >= CONNECT - 1) { // may connect for up
					if (j >= CONNECT - 1) {
						// may connect left
						if (board[i - 1][j - 1] == s
								&& board[i - 2][j - 2] == s
								&& board[i - 3][j - 3] == s) {
							return s;
						}
						if (j == CONNECT - 1) {
							// may connect for right
							if (board[i - 1][j + 1] == s
									&& board[i - 2][j + 2] == s
									&& board[i - 3][j + 3] == s) {
								return s;
							}
						}
					} else {
						// may connect for right
						if (board[i - 1][j + 1] == s
								&& board[i - 2][j + 2] == s
								&& board[i - 3][j + 3] == s) {
							return s;
						}
					}
				} else {// may connect for down

					if (j >= CONNECT - 1) {
						// may connect left
						if (board[i + 1][j - 1] == s
								&& board[i + 2][j - 2] == s
								&& board[i + 3][j - 3] == s) {
							return s;
						}
						if (j == CONNECT - 1) {
							// may connect for right
							if (board[i + 1][j + 1] == s
									&& board[i + 2][j + 2] == s
									&& board[i + 3][j + 3] == s) {
								return s;
							}
						}
					} else {
						// may connect for right
						if (board[i + 1][j + 1] == s
								&& board[i + 2][j + 2] == s
								&& board[i + 3][j + 3] == s) {
							return s;
						}
					}
				}
			}
		}
		return null;
	}

	/**
	 * isFull
	 * 
	 * PURPOSE: Check whether the board is full.
	 * 
	 * @param board
	 *            the board to check
	 * @return NEITHER if full or null if is not full
	 */
	public static Status isFull(Status board[][]) {
		boolean isFull = true;
		for (int i = 0; i < BoardUtil.HEIGHT; i++) {
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				if (board[i][j] == Status.NEITHER) {
					isFull = false;
					break;
				}
			}
		}
		if (isFull) {
			// is a draw
			return Status.NEITHER;
		}
		return null;
	}

	/**
	 * isFullCol
	 * 
	 * PURPOSE: Check whether a column is full
	 * 
	 * @param board
	 * @param col
	 * @return true if the column is full
	 */
	public static boolean isFullCol(Status[][] board, int col) {
		return board[0][col] != Status.NEITHER;
	}

	/**
	 * getNextRowLocation
	 * 
	 * PURPOSE: A help method to decide the row location to add piece.
	 * 
	 * @param col
	 *            the column to add piece
	 * @return the row location. -1 if this is no valid location
	 */
	public static int getNextRowLocation(Status[][] board, int col) {
		if (col < 0 || col >= BoardUtil.WIDTH) {
			return -1;
		}
		int i;

		for (i = BoardUtil.HEIGHT - 1; i >= 0; i--) {
			if (board[i][col] == Status.NEITHER) {
				break;
			}
		}
		if (i < 0) {
			// column is already full
			return -1;
		}
		return i;
	}

	/**
	 * maxConnectRow
	 * 
	 * PURPOSE: get the max number of connect pieces in a row
	 * 
	 * @param board
	 * @param player
	 * @return the max number of connect pieces in a row
	 */
	public static int maxConnectRow(Status[][] board, Status player) {
		int maxConnectRow = 0;
		for (int i = 0; i < HEIGHT; i++) {
			int connectNum = 0;
			for (int j = 0; j < WIDTH; j++) {
				if (board[i][j] == Status.NEITHER) {
					connectNum = 0;
					continue;
				} else {
					if (board[i][j] == player) {
						connectNum++;
						if (connectNum > maxConnectRow) {
							maxConnectRow = connectNum;
						}
					} else {
						connectNum = 0;
					}
				}
			}
		}
		return maxConnectRow;
	}

	
	/**
	 * maxConnectColummn
	 * 
	 * PURPOSE: get the max number of connect pieces in a column
	 * 
	 * @param board
	 * @param player
	 * @return the max number of connect pieces in a column
	 */
	public static int maxConnectColumn(Status[][] board, Status player) {
		int maxConnectCol = 0;
		for (int i = 0; i < WIDTH; i++) {
			int connectNum = 0;
			for (int j = 0; j < HEIGHT; j++) {
				if (board[j][i] == Status.NEITHER) {
					connectNum = 0;
					continue;
				} else {
					if (board[j][i] == player) {
						connectNum++;
						if (connectNum > maxConnectCol) {
							maxConnectCol = connectNum;
						}
					} else {
						connectNum = 0;
					}
				}
			}
		}
		return maxConnectCol;
	}

	
	/**
	 * maxConnectOblique
	 * 
	 * PURPOSE: get the max number of connect pieces in a oblique line
	 * 
	 * @param board
	 * @param player
	 * @return the max number of connect pieces in a oblique line
	 */
	public static int maxConnectOblique(Status[][] board, Status player) {
		int maxConnect = 0;
		for (int i = BoardUtil.HEIGHT - 1; i >= 0; i--) {
			for (int j = 0; j < BoardUtil.WIDTH; j++) {
				int connect = 1;
				if (board[i][j] != player) {
					continue;
				}
				if (i >= CONNECT - 1) { // may connect for up
					if (j >= CONNECT - 1) {
						// may connect left
						if (board[i - 1][j - 1] == player) {
							connect++;
						}
						if (board[i - 2][j - 2] == player) {
							connect++;
						}
						if (connect > maxConnect) {
							maxConnect = connect;
							connect = 1;
						}
						if (j == CONNECT - 1) {
							// may connect for right
							if (board[i - 1][j + 1] == player) {
								connect++;
							}
							if (board[i - 2][j + 2] == player) {
								connect++;
							}
							if (connect > maxConnect) {
								maxConnect = connect;
							}
						}
					} else {
						// may connect for right
						if (board[i - 1][j + 1] == player) {
							connect++;
						}
						if (board[i - 2][j + 2] == player) {
							connect++;
						}
						if (connect > maxConnect) {
							maxConnect = connect;
						}
					}
				} else {// may connect for down
					// may connect left
					if (j >= CONNECT - 1) {
						if (board[i + 1][j - 1] == player) {
							connect++;
						}
						if (board[i + 2][j - 2] == player) {
							connect++;
						}
						if (connect > maxConnect) {
							maxConnect = connect;
							connect = 1;
						}

						if (j == CONNECT - 1) {
							// may connect for right
							if (board[i + 1][j + 1] == player) {
								connect++;
							}
							if (board[i + 2][j + 2] == player) {
								connect++;
							}
							if (connect > maxConnect) {
								maxConnect = connect;
							}
						}
					} else {
						// may connect for right
						if (board[i + 1][j + 1] == player) {
							connect++;
						}
						if (board[i + 2][j + 2] == player) {
							connect++;
						}
						if (connect > maxConnect) {
							maxConnect = connect;
						}
					}
				}
			}
		}
		return maxConnect;
	}

	/**
	 * eval
	 * 
	 * PURPOSE: Calculate a score for the board state. If the AI has more possibility to
	 * win, the score is higher.
	 * 
	 * @param board
	 *            the board to eval.
	 * @return the score of the board state
	 */
	public static int eval(Status[][] board, Status player) {
		Status other = Status.ONE;
		if (player == Status.ONE) {
			other = Status.TWO;
		}
		if (BoardUtil.canConnectColumn(board) == player
				|| BoardUtil.canConnectRow(board) == player
				|| BoardUtil.canConnectOblique(board) == player) {
			return WINSCORE;
		} else if (BoardUtil.canConnectColumn(board) == other
				|| BoardUtil.canConnectRow(board) == other
				|| BoardUtil.canConnectOblique(board) == other) {
			return -WINSCORE;
		} else if (BoardUtil.isFull(board) != null) {
			return WINSCORE / 2;
		} else {
			int score = 0;
			int maxRow = BoardUtil.maxConnectRow(board, player);
			if (maxRow >= 2) {
				score += SINGLECONNECTSCORE * maxRow;
			}
			int maxCol = BoardUtil.maxConnectColumn(board, player);
			if (maxCol >= 2) {
				score += SINGLECONNECTSCORE * maxCol;
			}
			int maxOblique = BoardUtil.maxConnectOblique(board, player);
			if (maxOblique >= 2) {
				score += SINGLECONNECTSCORE * maxOblique;
			}
			return score;
		}
	}

	/**
	 * getNextBoard
	 * 
	 * PURPOSE: Get the next state of board after a move.
	 * 
	 * @param board
	 * @param col
	 * @param s
	 * @return a new board
	 */
	public static Status[][] getNextBoard(Status[][] board, int col, Status s) {
		Status newBoard[][] = new Status[HEIGHT][WIDTH];
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				newBoard[i][j] = board[i][j];
			}
		}
		int row = BoardUtil.getNextRowLocation(board, col);
		newBoard[row][col] = s;
		return newBoard;
	}
}
