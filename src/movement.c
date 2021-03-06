#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "aux.h"
#include "chess.h"
#include "endgame.h"
#include "input.h"
#include "movement.h"

bool is_movement_valid(square board[][BOARD_SIZE], History *history, Player player){
	if(!is_the_squares_valid(player.move))
		puts("\nYou've entered an invalid square!");
	else if(board[player.move.from_rank][player.move.from_file].color != player.turn)
		puts("\nYou must choose a piece of your color.");
	else if(board[player.move.to_rank][player.move.to_file].color == player.turn)
		puts("\nYou can't capture your own piece!");
	else if(!is_piece_movement_compatible(board, history, player))
		puts("\nThis movement is incompatible with your piece.");
	else if(is_jump_other_pieces(board, player.move))
		puts("\nYour piece can't jump over other pieces!");
	else if(will_king_be_in_check(board, *history, player))
		puts("\nYour can't do this move, because your king will be in check.");
	else
		return true;

	pause_screen();

	return false;
}

bool is_piece_movement_compatible(square board[][BOARD_SIZE], History *history, Player player){
	switch(board[player.move.from_rank][player.move.from_file].piece){
		case BISHOP:
			return is_bishop_movement_valid(player.move);
		case KING:
			history->is_castle = is_castle_valid(board, history, player);

			return is_king_movement_valid(player.move) ||
			       history->is_castle;
		case KNIGHT:
			return is_knight_movement_valid(player.move);
		case PAWN:
			if(history->board->prev != NULL)
				history->is_en_passant = is_en_passant_valid(board,
				                                             history->board->prev->player.move,
				                                             player.move);

			return is_pawn_movement_valid(player) &&
			       (is_pawn_advance_valid(board, player.move) ||
			       is_pawn_capture_valid(board, player.move) ||
			       history->is_en_passant);
		case QUEEN:
			return is_queen_movement_valid(player.move);
		case ROOK:
			return is_rook_movement_valid(player.move);
	}
}

bool is_bishop_movement_valid(move_coord move){
	return abs(move.to_rank - move.from_rank) ==
	       abs(move.to_file - move.from_file);
}

bool is_king_movement_valid(move_coord move){
	return move.from_rank - 1 <= move.to_rank &&
	       move.from_rank + 1 >= move.to_rank &&
	       move.from_file - 1 <= move.to_file &&
	       move.from_file + 1 >= move.to_file;
}

bool is_castle_valid(square board[][BOARD_SIZE], History *history, Player player){
	const int8_t START_KING_FILE = 4,
	             START_ROOK_FILE = (START_KING_FILE - 2 == player.move.to_file? 0 : 7),
	             START_RANK = (player.turn == WHITE? 7 : 0);

	if(START_RANK != player.move.from_rank ||
	   START_RANK != player.move.to_rank ||
	   START_KING_FILE != player.move.from_file ||
	   START_KING_FILE - 2 != player.move.to_file &&
	   START_KING_FILE + 2 != player.move.to_file)
		return false;

	return !has_castle_pieces_moved(history, START_RANK, START_KING_FILE) &&
	       !are_there_pieces_between(board[START_RANK], START_KING_FILE, START_ROOK_FILE) &&
	       is_king_safe(board, history, player);
}

bool has_castle_pieces_moved(History *history, int8_t rank, int8_t rook_file){
	const int8_t KING_FILE = 4;

	for(h_board *aux = history->board->prev; aux != NULL; aux = aux->prev)
		if((aux->player.move.from_file == KING_FILE ||
		   aux->player.move.from_file == rook_file) &&
		   aux->player.move.from_rank == rank)
			return true;

	return false;
}

bool are_there_pieces_between(square rank[], int8_t start, int8_t end){
	int8_t i = start;

	for(advance_to(&i, end); i != end; advance_to(&i, end))
		if(rank[i].piece != EMPTY)
			return true;

	return false;
}

bool is_king_safe(square board[][BOARD_SIZE], History *history, Player player){
	if(is_player_king_in_check(board, history, player.turn))
		return false;

	do{
		int8_t i = player.move.from_file;

		advance_to(&i, player.move.to_file);

		player.move.from_file = i;

		if(will_king_be_in_check(board, *history, player))
			return false;
	}while(player.move.from_file != player.move.to_file);

	return true;
}

bool is_knight_movement_valid(move_coord move){
	const int8_t DIFF_RANK = abs(move.to_rank - move.from_rank),
	             DIFF_FILE = abs(move.to_file - move.from_file);

	return DIFF_RANK == 2 && DIFF_FILE == 1 ||
	       DIFF_RANK == 1 && DIFF_FILE == 2;
}

bool is_pawn_movement_valid(Player player){
	int8_t start_rank, two_squares, one_square;

	if(player.turn == WHITE){
		start_rank = 6;
		two_squares = -2;
		one_square = -1;
	}else{
		start_rank = 1;
		two_squares = 2;
		one_square = 1;
	}

	return player.move.from_rank + one_square == player.move.to_rank &&
	       player.move.from_file - 1 <= player.move.to_file &&
	       player.move.from_file + 1 >= player.move.to_file ||
	       player.move.from_rank == start_rank &&
	       player.move.from_rank + two_squares == player.move.to_rank &&
	       player.move.from_file == player.move.to_file;
}

bool is_pawn_advance_valid(square board[][BOARD_SIZE], move_coord move){
	return move.from_file == move.to_file &&
	       board[move.to_rank][move.to_file].piece == EMPTY;
}

bool is_pawn_capture_valid(square board[][BOARD_SIZE], move_coord move){
	return move.from_file != move.to_file &&
	       board[move.to_rank][move.to_file].piece != EMPTY;
}

bool is_en_passant_valid(square board[][BOARD_SIZE], move_coord last_move, move_coord move){
	int8_t two_squares, start_rank;

	if(board[move.from_rank][move.from_file].color == WHITE){
		two_squares = 2;
		start_rank = 1;
	}else{
		two_squares = -2;
		start_rank = 6;
	}

	return board[last_move.to_rank][last_move.to_file].piece == PAWN &&
	       last_move.from_rank + two_squares == last_move.to_rank &&
	       last_move.from_rank == start_rank &&
	       last_move.to_rank == move.from_rank &&
	       abs(last_move.to_file - move.from_file) == 1 &&
	       last_move.to_file == move.to_file;
}

bool is_queen_movement_valid(move_coord move){
	return is_bishop_movement_valid(move) ||
	       is_rook_movement_valid(move);
}

bool is_rook_movement_valid(move_coord move){
	return move.from_rank == move.to_rank ||
	       move.from_file == move.to_file;
}

bool is_jump_other_pieces(square board[][BOARD_SIZE], move_coord move){
	if(board[move.from_rank][move.from_file].piece != KNIGHT){
		int8_t i = move.from_rank, j = move.from_file;
		
		for(advance_to(&i, move.to_rank), advance_to(&j, move.to_file);
		    i != move.to_rank || j != move.to_file;
		    advance_to(&i, move.to_rank), advance_to(&j, move.to_file))
			if(board[i][j].piece != EMPTY)
				return true;
	}
	
	return false;
}

bool will_king_be_in_check(square board[][BOARD_SIZE], History history, Player player){
	bool has_special_move = history.is_castle || history.is_en_passant;
	move_coord aux_move;
	struct{
		square from, to, aux_from, aux_to;
	} move_sqrs;

	move_sqrs.from = board[player.move.from_rank][player.move.from_file];
	move_sqrs.to = board[player.move.to_rank][player.move.to_file];

	move_piece(board, player.move);

	if(has_special_move){
		if(history.is_castle)
			aux_move = get_castle_rook(player.move);
		else
			aux_move = history.board->player.move;

		move_sqrs.aux_from = board[aux_move.from_rank][aux_move.from_file];
		move_sqrs.aux_to = board[aux_move.to_rank][aux_move.to_file];

		move_piece(board, aux_move);
	}

	bool is_check = is_player_king_in_check(board, &history, player.turn);

	board[player.move.from_rank][player.move.from_file] = move_sqrs.from;
	board[player.move.to_rank][player.move.to_file] = move_sqrs.to;

	if(has_special_move){
		board[aux_move.from_rank][aux_move.from_file] = move_sqrs.aux_from;
		board[aux_move.to_rank][aux_move.to_file] = move_sqrs.aux_to;
	}

	return is_check;
}

bool is_player_king_in_check(square board[][BOARD_SIZE], History *history, char turn){
	Player opponent;

	opponent.turn = (turn == WHITE? BLACK : WHITE);
	
	// Searching where the king is
	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].piece == KING && board[i][j].color == turn){
				opponent.move.to_rank = i;
				opponent.move.to_file = j;
				break;
			}
		}
	}
	
	// Verify if the opponent's pieces are threatening the king
	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].color == opponent.turn){
				opponent.move.from_rank = i;
				opponent.move.from_file = j;

				if(is_piece_movement_compatible(board, history, opponent) &&
				   !is_jump_other_pieces(board, opponent.move)){
					history->last_check = opponent.move;
					
				   	return true;
				}
			}
		}
	}
				   	
	return false;
}
