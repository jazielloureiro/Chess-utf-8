#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "aux.h"
#include "chess.h"
#include "endgame.h"
#include "input.h"
#include "movement.h"

void play(){
	square board[BOARD_SIZE][BOARD_SIZE];
	History history;
	Player player;

	init_board(board);
	init_history(board, &history);
	init_player(&player);

	do{
		do{
			clear_screen();
			print_top_menu(player.turn, player.is_in_check);
			print_board(board);

			player.move = read_input();

			if(is_action(player)){
				if(is_action_confirmed(player)){
					free_history(history.board);
					return;
				}else
					continue;
			}
		}while(!is_movement_valid(board, &history, player));

		update_history(board, &history, player);
		
		move_piece(board, player.move);

		if(history.is_castle){
			move_piece(board, get_castle_rook(player.move));
			history.is_castle = false;
		}

		if(history.is_en_passant){
			move_piece(board, history.board->player.move);
			history.is_en_passant = false;
		}

		if(is_there_promotion(board, player.move))
			promotion(&board[player.move.to_rank][player.move.to_file], player.turn);

		add_board_to_history(board, &history);

		player.turn = (player.turn == WHITE? BLACK : WHITE);
	}while(!is_game_done(board, &history, &player));

	free_history(history.board);
}

void help(){
	clear_screen();
	
	puts("How to move the pieces:\n"
	     "\tType the coordinate of the origin square and the coordinate of the destination square\n"
	     "\twith a character between them. Examples: e2 e4, c7-c5, g1$f3.\n\n"
	     "How to resign:\n"
	     "\tType: :r\n\n"
	     "How to ask for a draw:\n"
	     "\tType: :d");
	     
	pause_screen();
}

int main(){
	char menu_choose;

	do{
		clear_screen();

		printf("\tChess UTF-8\n1. Play\n2. Help\n0. Exit\n\n> ");
		menu_choose = getchar();
		clear_input_buffer();

		if(menu_choose == '1')
			play();
		else if(menu_choose == '2')
			help();
	}while(menu_choose != '0');

	return 0;
}
