#ifndef CHESS_H
#define CHESS_H

#define BOARD_SIZE 8

enum pieces{
	EMPTY = ' ',
	BISHOP = 'B',
	KING = 'K',
	KNIGHT = 'N',
	PAWN = 'P',
	QUEEN = 'Q',
	ROOK = 'R'
};

enum colors{
	WHITE = 'W',
	BLACK = 'B'
};

enum flags{
	ASK_RESIGN = -3,
	ASK_DRAW,
	INVALID_SQUARE,
	FIFTY_MOVES,
	THREEFOLD_REP,
	INSUFFICIENT_MAT,
	STALEMATE
};

typedef struct{
	char *image;
	char piece;
	char color;
} square;

typedef struct{
	int8_t from_rank : 4;
	int8_t from_file : 4;
	int8_t to_rank : 4;
	int8_t to_file : 4;
} move_coord;

typedef struct{
	move_coord move;
	char turn;
	bool is_in_check;
} Player;
	
typedef struct{
	char piece;
	char color;
	int8_t rank : 4;
	int8_t file : 4;
} h_square;
	
typedef struct hist_board{
	int8_t pieces_qty;
	h_square *pieces;
	Player player;
	struct hist_board *prev;
} h_board;

typedef struct{
	h_board *board;
	int moves_counter;
	bool is_castle, is_en_passant;
	move_coord last_check;
} History;

void init_board(square board[][BOARD_SIZE]);

void init_history(square board[][BOARD_SIZE], History *history);

void init_player(Player *player);

void update_history(square board[][BOARD_SIZE], History *history, Player player);

h_board *get_current_board(square board[][BOARD_SIZE]);

void add_board_to_history(square board[][BOARD_SIZE], History *history);

int8_t count_pieces(square board[][BOARD_SIZE]);

void move_piece(square board[][BOARD_SIZE], move_coord move);

move_coord get_castle_rook(move_coord move);

void advance_to(char *from, char to);

void free_history(h_board *aux);

#endif
