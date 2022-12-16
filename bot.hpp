/*
#include <iostream>
#include <random>
#include <unordered_map>

const int BOARD_SIZE = 9; // size of the Go board

// structure to represent a move in No-Capture Go
struct Move
{
	int x, y; // coordinates of the move
	int player; // player who made the move (1 or 2)
};

// structure to represent the state of the game
struct GameState
{
	int board[BOARD_SIZE][BOARD_SIZE]; // Go board
	std::vector<Move> moves; // list of moves made so far
	int player; // player to make the next move (1 or 2)

	int getWinner() const {
		if (!isOver()) return 0;
		int captures1 = 0, captures2 = 0;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (board[i][j] == 1) captures1++;
				if (board[i][j] == 2) captures2++;
			}
		}
		if (captures1 > captures2) return 1;
		if (captures2 > captures1) return 2;
		return 3;
	}

	// function to make a move in a given game state
	void makeMove(const Move& move) {
		board[move.x][move.y] = move.player;
		moves.push_back(move);
		player = (player == 1) ? 2 : 1;
	}

	// function to get a list of all the valid moves that can be made in a given game state
	std::vector<Move> getValidMoves() const {
		std::vector<Move> moves;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (board[i][j] == 0) {
					GameState tempState = *this;
					tempState.makeMove({ i, j, player });
					if (tempState.getWinner() != player) {
						moves.push_back({ i, j, player });
					}
				}
			}
		}
		return moves;
	}

};

// structure to represent a node in the Monte Carlo Tree
struct MCTSNode
{
	GameState state; // state of the game
	MCTSNode* parent; // parent node in the tree
	std::vector<MCTSNode*> children; // children nodes in the tree
	std::unordered_map<int, int> visits; // number of visits for each player
	// constructor to initialize a node
	MCTSNode(const GameState& state_, MCTSNode* parent_ = nullptr)
		: state(state_), parent(parent_) {}

	// function to add a child node to the tree
	MCTSNode* addChild(const GameState& state) {
		MCTSNode* node = new MCTSNode(state, this);
		children.push_back(node);
		return node;
	}
};

// function to perform the Monte Carlo Tree Search
MCTSNode* MCTS(MCTSNode* root, int maxIterations) {
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);
	for (int i = 0; i < maxIterations; i++) {
		// select the node to expand
		MCTSNode* node = root;
		while (!node->state.isOver() && !node->children.empty()) {
			double maxUCB1 = -1;
			MCTSNode* maxChild = nullptr;
			for (MCTSNode* child : node->children) {
				double ucb1 = child->state.player == 1
					? (double)child->state.moves.size() / child->visits[1]
					+ sqrt(2 * log(node->visits[1]) / child->visits[1])
					: (double)child->state.moves.size() / child->visits[2]
					+ sqrt(2 * log(node->visits[2]) / child->visits[2]);
				if (ucb1 > maxUCB1) {
					maxUCB1 = ucb1;
					maxChild = child;
				}
			}
			node = maxChild;
		}
		// expand the selected node
		if (!node->state.isOver()) {
			std::vector<Move> moves = node->state.getValidMoves();
			for (const Move& move : moves) {
				GameState state = node->state;
				state.makeMove(move);
				node->addChild(state);
			}
		}
		// simulate the game from the expanded node
		GameState state = node->state;
		while (!state.isOver()) {
			std::vector<Move> moves = state.getValidMoves();
			int index = dist(rng) * moves.size();
			state.makeMove(moves[index]);
		}
		// backpropagate the result of the simulation
		while (node) {
			node->visits[state.getWinner()]++;
			node = node->parent;
		}
	}
	// return the child node with the most visits
	MCTSNode* maxChild = nullptr;
	int maxVisits = -1;
	for (MCTSNode* child : root->children) {
		int visits = child->visits[root->state.player];
		if (visits > maxVisits) {
			maxChild = child;
			maxVisits = visits;
		}
	}
	return maxChild;
}


int main() {
	// initialize the game
	GameState state;
	state.player = 1;

	// create the root node of the Monte Carlo Tree
	MCTSNode* root = new MCTSNode(state);

	// perform the Monte Carlo Tree Search
	MCTSNode* bestNode = MCTS(root, 10000);

	// make the best move found by the MCTS
	state.makeMove(bestNode->state.moves.back());

	// print the final state of the game
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			std::cout << state.board[i][j] << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}
*/