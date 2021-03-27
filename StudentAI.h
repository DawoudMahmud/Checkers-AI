#ifndef STUDENTAI_H
#define STUDENTAI_H
#include "AI.h"
#include "Board.h"
#pragma once

//The following part should be completed by students.
//Students can modify anything except the class name and exisiting functions and varibles.
class StudentAI :public AI
{
public:
    Board board;
	int secondsForMove;
	int secondsForGame;
	StudentAI(int col, int row, int p);

	virtual Move GetMove(Move board);
	Move GetRandomMove(Move board);
	Move GetRandomMove();
};

class Node
{
public:

	Move move;
	Node* parent;
	vector<Node*> children;
	double wins;
	double AMAF_wins;
	double playouts;
	vector<Move> AMAF_moves;
	int player; //Player who is making the next move, i.e. if this node is a result of white's move, this is going to be black

	Node();
	~Node();
	Node(Node* parent, Move const & move);
	
	void AddNode(Move const &move);
	Node* UCB1();
	Node* RAVE();
	Node* Select(Board &board, int &movesMade);
	Node* Expand(Board& board, int& movesMade);
	int Simulate(Board& board, int& movesMade);
	void BackPropogate(int result);
	Move GetMostPlayouts();

};

Board CopyBoard(Board const& parent);
bool MoveInVector(Move move, vector<Move> vect);
#endif //STUDENTAI_H
