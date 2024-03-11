#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>


#define TILE_LEN	16
#define ROW_LEN		4
#define COL_LEN		4
#define MOVES		4
#define TILE		1
#define BLANK_TILE	0

//
typedef struct node{
	int state[16];
	int g;
	int f;
	int prev_move;
} node;

typedef struct Route{
	int pos;
	char direction;
	int test;
} Route;

//Global Variables

	//used to track the position of the blank in a state.
int blank_pos;

	//initial state.
node initial_node;

	//record the route
Route routing[1000];
Route now_route;



#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down 
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


// return 1 if op is applicable in state, otherwise return 0
int applicable( int op )
{
    return( ap_ops[op][blank_pos] );
}

/* apply operator */
void apply( node* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == LEFT ? -1 : (op == RIGHT ? 1 : (op == UP ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	
	now_route.pos = n->state[t];
	
	n->state[t] = BLANK_TILE;
	
	now_route.direction = (op == LEFT? 'R': (op == RIGHT? 'L': (op == UP? 'D': 'U')));
	//update blank pos
	blank_pos = t;
}

// copy the initial state
void state_copy(int* new_state, int* prev_state){
	int i;
	for (i = 0; i < TILE_LEN; i++) {
		new_state[i] = prev_state[i];
	}
}

// Finds the position of the blank tile given the state
void find_blank_pos(int* state) {
	int i;
	for (i = 0; i < TILE_LEN; i++) {
		if (state[i] == BLANK_TILE) {
			blank_pos = i;
			break;
		}
	}
}	

//h:estimate cost
int manhattan( int* state )
{
	int sum = 0;
	int row,col,x,y;
	int index;
	for (col = 0; col < COL_LEN; col++) {
		for (row = 0; row < ROW_LEN; row++) {
			index = (col * COL_LEN) + row;
			// Manhattan Distance 1D -> 2D excluding Blank Tile
			if (state[index] != BLANK_TILE) {
				y = state[index] / COL_LEN;
				x = state[index] - (y * ROW_LEN);
				sum += abs(row - x) + abs(col - y);
			}
		}
	}

	return sum;
}

node* ida(node* node, int f_limit, int* next_f){	
	
	int a;
	int estimate;
	int actions[MOVES] = {};
	
	struct node* r = NULL;
	struct node new_node;
	
	// Find out all the applicable moves in the current state
	for (a = 0; a < MOVES; a++) {
		actions[a] = applicable(a);
	}
	
	for (a = 0; a < MOVES; a++) {
		if (actions[a] != 0 && a != node->prev_move) {
								// OPTIMIZATION - Last Moves Heuristic //
			
			new_node.prev_move = (a == LEFT ? RIGHT : (a == RIGHT ? LEFT : (a == UP ? DOWN : UP)));
			

			
			// Find the position of the blank tile
			find_blank_pos(node->state);

			state_copy(new_node.state, node->state);
			apply(&new_node, a);

			// Add 1 cost
			new_node.g = node->g + 1;
			

			// New estimated cost
			estimate = manhattan(new_node.state);
			new_node.f = new_node.g + estimate;

			// If estimated cost is greater than f_limit, backtrack.
			if (new_node.f > f_limit) {
			
				
				if (new_node.f < *next_f) {
					// Update next_f
					*next_f = new_node.f;
					
				}
			}
			
			// If estimated cost is smaller than f_limit
			else {
				//reaching the goal node
				if (estimate == 0) {
					//record the routing
					routing[new_node.g-1].pos = now_route.pos;
					routing[new_node.g-1].direction = now_route.direction;
					
					
					r = &new_node;
					return r;
				}
				
				//record the routing
				routing[new_node.g-1].pos = now_route.pos;
				routing[new_node.g-1].direction = now_route.direction;
			
				
				r = ida(&new_node, f_limit, next_f);
				if (r != NULL) {
					return r;
				}
			}

		}
	}
	return( NULL );
}

int IDA_control_loop(){
	struct node* r = NULL;
	
	int f_limit;
	initial_node.f = f_limit = manhattan(initial_node.state);
	
	while(r == NULL){
		//next_f -> infinity
		int next_f = INT_MAX;
		struct node curr_node;
		
		// Node state = initial state
		state_copy(curr_node.state, initial_node.state);
		
		//Set cost=0
		curr_node.g = 0;
		curr_node.prev_move = 0;
		
		r = ida(&curr_node, f_limit, &next_f);
		
		if(r == NULL){
			f_limit = next_f;
		}
	}
	if(r){
		return r->g;
	}
	else{
		return -1;
	}
	
	
	
}




int main(){
	//times
	clock_t start, end;
	
	//input & 0's position
	for(int i=0; i<16; i++){
		scanf("%d", &initial_node.state[i]);
		if(initial_node.state[i] == 0){
			blank_pos = i;
		}
	}
	
	//initialize.
	//f = g + h
	//g:true cost
	//h:estimate cost
	initial_node.g=0;
	initial_node.f=0;
	
	int solution_length;
	
	//Solve and time
	start = clock();
	solution_length = IDA_control_loop();
	end = clock();
	
	//print out the result
	if(solution_length == -1){
		printf("No solution.");
	}
	else{
		printf("Total run time = %.4f seconds.\n", (double)(end-start)/CLK_TCK);
		printf("An optimal solution has %d moves:\n", solution_length);
		for(int i=0; i<solution_length; i++){
			printf("%2d%c ", routing[i].pos, routing[i].direction);
			if((i+1)%10 == 0){
				printf("\n");
			}
		}
	}
	
	
	return 0;
}
