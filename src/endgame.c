#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "aux.h"
#include "chess.h"
#include "endgame.h"
#include "input.h"
#include "movement.h"

bool is_game_done(square board[][BOARD_SIZE], History *history, Player *player){
	const int MAX_MOVES = 100;

	player->is_in_check = is_player_king_in_check(board, history, player->turn);

	if(player->is_in_check){
		if(is_checkmate(board, *history, player->turn)){
			print_final_board(board, player->turn);
			return true;
		}
	}else{
		if(is_stalemate(board, *history, player->turn)){
			print_final_board(board, STALEMATE);
			return true;
		}
	}

	if(history->moves_counter == MAX_MOVES){
		print_final_board(board, FIFTY_MOVES);
		return true;
	}else if(is_threefold_repetition(history, player->turn)){
		print_final_board(board, THREEFOLD_REP);
		return true;
	}else if(is_insufficient_material(board)){
		print_final_board(board, INSUFFICIENT_MAT);
		return true;
	}

	return false;
}

bool is_checkmate(square board[][BOARD_SIZE], History history, char turn){
	if(can_king_move(board, history, turn))
		return false;
	else if(can_piece_cover_check(board, history, turn))
		return false;
	else if(can_attacking_piece_be_captured(board, history, turn))
		return false;

	return true;
}

bool can_king_move(square board[][BOARD_SIZE], History history, char turn){
	for(int8_t i = history.last_check.to_rank - 1;
	    i <= history.last_check.to_rank + 1;
	    i++)
	{
		for(int8_t j = history.last_check.to_file - 1;
		    j <= history.last_check.to_file + 1;
		    j++)
		{
			if(i >= 0 && i <= 7 && j >= 0 && j <= 7 &&
			   board[i][j].color !=
			   board[history.last_check.to_rank][history.last_check.to_file].color)
			{
				Player temp;

				temp.turn = turn;
				temp.move.to_rank = i;
				temp.move.to_file = j;
				temp.move.from_rank = history.last_check.to_rank;
				temp.move.from_file = history.last_check.to_file;

				if(!will_king_be_in_check(board, history, temp))
				   	return true;
			}
		}
	}

	return false;
}

bool can_piece_cover_check(square board[][BOARD_SIZE], History history, char turn){
	int8_t i = history.last_check.from_rank, j = history.last_check.from_file;

	if(board[i][j].piece != KNIGHT){
		do{
			advance_to(&i, history.last_check.to_rank);
			advance_to(&j, history.last_check.to_file);

			history.last_check.from_rank = i;
			history.last_check.from_file = j;
			
			if(i != history.last_check.to_rank || j != history.last_check.to_file)
				if(can_attacking_piece_be_captured(board, history, turn))
					return true;
		}while(i != history.last_check.to_rank || j != history.last_check.to_file);
	}

	return false;
}

bool can_attacking_piece_be_captured(square board[][BOARD_SIZE], History history, char turn){
	Player piece;

	piece.turn = turn;
	piece.move.to_rank = history.last_check.from_rank;
	piece.move.to_file = history.last_check.from_file;

	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].color == turn){
				piece.move.from_rank = i;
				piece.move.from_file = j;
			
				if(is_piece_movement_compatible(board, &history, piece) &&
				   !is_jump_other_pieces(board, piece.move) &&
				   !will_king_be_in_check(board, history, piece))
				   	return true;
			}
		}
	}

	return false;
}

bool is_stalemate(square board[][BOARD_SIZE], History history, char turn){
	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].color == turn){
				Player temp;

				temp.turn = turn;
				temp.move.from_rank = i;
				temp.move.from_file = j;
				
				if(is_there_possible_move(board, history, temp))
					return false;
			}
		}
	}

	return true;
}

bool is_there_possible_move(square board[][BOARD_SIZE], History history, Player temp){
	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].color != temp.turn){
				temp.move.to_rank = i;
				temp.move.to_file = j;

				if(is_piece_movement_compatible(board, &history, temp) &&
				   !is_jump_other_pieces(board, temp.move) &&
				   !will_king_be_in_check(board, history, temp))
					return true;
			}
		}
	}

	return false;
}

bool is_threefold_repetition(History *history, char turn){
	uint8_t repetition_counter = 1;

	for(h_board *current = history->board, *aux = history->board->prev;
	    aux != NULL && current->pieces_qty == aux->pieces_qty;
	    aux = aux->prev)
	{
		bool is_different = false;

		for(uint8_t i = 0; i < current->pieces_qty && !is_different; i++)
			if(turn != aux->player.turn ||
			   is_squares_different(current->pieces[i], aux->pieces[i]))
				is_different = true;

		if(!is_different)
			repetition_counter++;

		if(repetition_counter == 3)
			return true;
	}

	return false;
}

bool is_squares_different(h_square sqr1, h_square sqr2){
	return sqr1.piece != sqr2.piece ||
	       sqr1.color != sqr2.color ||
	       sqr1.rank != sqr2.rank ||
	       sqr1.file != sqr2.file;
}

bool is_insufficient_material(square board[][BOARD_SIZE]){
	int8_t white_minor_pieces = 0, black_minor_pieces = 0;

	for(int8_t i = 0; i < BOARD_SIZE; i++){
		for(int8_t j = 0; j < BOARD_SIZE; j++){
			if(board[i][j].piece == BISHOP || board[i][j].piece == KNIGHT){
				if(board[i][j].color == WHITE)
					white_minor_pieces++;
				else
					black_minor_pieces++;

				if(white_minor_pieces == 2 || black_minor_pieces == 2)
					return false;
			}else if(board[i][j].piece == QUEEN ||
			         board[i][j].piece == ROOK ||
			         board[i][j].piece == PAWN)
				return false;
		}
	}

	return true;
}
