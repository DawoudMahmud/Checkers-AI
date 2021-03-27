#include "StudentAI.h"
#include <limits>
#include <random>
#include <ctime>
#include <queue>
//The following part should be completed by students.
//The students can modify anything except the class name and exisiting functions and varibles.
StudentAI::StudentAI(int col,int row,int p)
	:AI(col, row, p)
{
    board = Board(col,row,p);
    board.initializeGame();
    player = 2;
    secondsForMove = 15;
    secondsForGame = 480;
}

Move StudentAI::GetMove(Move move)
{
     

    //Player 2 is white, player 1 is black
    Move res;
    if (move.seq.empty())
    {
        player = 1; 
    } else{
        board.makeMove(move,player == 1?2:1);
    }
    vector<vector<Move>> moves = board.getAllPossibleMoves(player);



    if (moves.size() == 1 && moves[0].size() == 1) {
        board.makeMove(moves[0][0], player);
        return moves[0][0];
    }

    if (this->secondsForGame <= 16) return GetRandomMove();

    time_t timestamp = time(NULL);

    Node* tree = new Node();
    tree->player = player;

    int iter = 0;
    while(difftime(time(NULL), timestamp) < this->secondsForMove) {
    //while (iter==0) {
        int movesMade = 0;
        Node* leaf = tree->Select(board, movesMade);
        Node* child = leaf->Expand(board, movesMade);
        int eval = child->Simulate(board, movesMade);
        child->BackPropogate(eval);
        for (int i = 0; i < movesMade; i++) board.Undo();
        iter++;
    }
    res = tree->GetMostPlayouts();

    delete tree;

    this->secondsForGame -= difftime(time(NULL), timestamp);

    board.makeMove(res,player);
    return res;
}

Move StudentAI::GetRandomMove(Move move)
{
    if (move.seq.empty())
    {
        player = 1;
    }
    else {
        board.makeMove(move, player == 1 ? 2 : 1);
    }
    vector<vector<Move> > moves = board.getAllPossibleMoves(player);
    int i = rand() % (moves.size());
    vector<Move> checker_moves = moves[i];
    int j = rand() % (checker_moves.size());
    Move res = checker_moves[j];
    board.makeMove(res, player);
    return res;


}

Move StudentAI::GetRandomMove()
{
    vector<vector<Move> > moves = board.getAllPossibleMoves(player);
    int i = rand() % (moves.size());
    vector<Move> checker_moves = moves[i];
    int j = rand() % (checker_moves.size());
    Move res = checker_moves[j];
    board.makeMove(res, player);
    return res;
}

//============================================NODE==========================================//
Node::Node() {
    parent = nullptr;
    wins = 0;
    AMAF_wins = 0;
    playouts = 0;
    player = -1;
}

Node::~Node() {
    for (Node* c : children) {
        delete c;
    }
}

Node::Node(Node* parent, Move const & move) {
    this->parent = parent;
    this->move = move;
    this->player = parent->player == 1 ? 2 : 1;
    wins = 0;
    AMAF_wins = 0;
    playouts = 0;
}


void Node::AddNode(Move const &move)
{
    Node* child = new Node(this, move);
    this->children.push_back(child);

}

Node* Node::UCB1()
{
    double max = 0;
    double C = sqrt(2);
    Node* res = this->children[0];
    for (Node* c : this->children) {
        if (c->playouts == 0) return c;
        double wins = c->playouts - c->wins;
        double UCB = wins / c->playouts + C * sqrt(log(c->parent->playouts) / c->playouts);
        if (UCB >= max) {
            max = UCB;
            res = c;
        }
    }
    return res;
}

Node* Node::RAVE()
{
    double max = 0;
    double C = sqrt(2);
    double P = 10;
    double A = std::max(0.0, (P-this->playouts)/P);
    Node* res = this->children[0];
    for (Node* c : this->children) {
        if (c->playouts == 0) return c;
        double wins = A*AMAF_wins+(1-A)*(c->playouts - c->wins);
        double RAVE = wins / c->playouts + C * sqrt(log(c->parent->playouts) / c->playouts);
        if (RAVE >= max) {
            max = RAVE;
            res = c;
        }
    }
    return res;
}

Node* Node::Select(Board& board, int& movesMade)
{
    Node* leaf=this;
    while (!(leaf->children.empty())) {
        leaf = leaf->RAVE();
        board.makeMove(leaf->move, (leaf->player == 1 ? 2 : 1));
        movesMade++;
    }
    return leaf;
}

Node* Node::Expand(Board& board, int& movesMade)
{
    vector<vector<Move>> moves = board.getAllPossibleMoves(this->player);
    if (moves.size() > 0) {
        for (vector<Move> v : moves) {
            for (Move m : v) {
                this->AddNode(m);
            }
        }
        Node* res = this->children[rand() % (this->children.size())];
        board.makeMove(res->move, this->player);
        movesMade++;
        return res;
    }
    return this;
}

int Node::Simulate(Board& board, int& movesMade)
{
    int cplayer = this->player;
    while (true) {
        vector<vector<Move>> moves = board.getAllPossibleMoves(cplayer);
        if (moves.size() != 0) {
            int i = rand() % (moves.size());
            vector<Move> checker_moves = moves[i];
            int j = rand() % (checker_moves.size());
            Move nmove = checker_moves[j];

            board.makeMove(nmove, cplayer);
            movesMade++;
        }
        if (board.isWin(cplayer == 1 ? "B" : "W") != 0) break;
        cplayer = cplayer == 1 ? 2 : 1;
    }
    return board.isWin(cplayer == 1 ? "B" : "W");
}

void Node::BackPropogate(int result)
{
    Node* current = this;
    while (current != nullptr) {
        current->playouts++;
        if (result == current->player) {
            current->AMAF_moves.push_back(current->move);
            if (current->parent != nullptr) {
                for (Node* c : current->parent->children) {
                    if (MoveInVector(c->move, current->AMAF_moves)) {
                        c->AMAF_wins++;
                    }
                }
                current->parent->AMAF_moves = current->AMAF_moves;
            }
            current->wins++;
        }
        //current->wins += result == -1 ? 0.5 : 0;
        current = current->parent;
    } 
}

Move Node::GetMostPlayouts()
{
    Move res;
    int max = 0;
    for (Node* c : this->children) {
        if (c->playouts >= max) {
            max = c->playouts;
            res = c->move;
        }
    }
    return res;
}

bool MoveInVector(Move move, vector<Move> vect)
{
    for (Move m : vect) {
        if (m.seq == move.seq) return true;
    }
    return false;
}


