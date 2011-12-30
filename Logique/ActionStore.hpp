
#ifndef ACTIONSTORE
#define ACTIONSTORE

#include "DecisionTree.hpp"
#include "Square.hpp"

namespace Logique {

	struct ActionStore 
	{
		unsigned int foodcount;
		Square present;
		Square up;
		Square left;
		Square down;
		Square right;
		int upleft;
		int upright;
		int downright;
		int downleft;
		int upup;
		int rightright;
		int downdown;
		int leftleft;

		DecisionTree::ReturnValue result;
		DecisionTree::ReturnValue last;

		ActionStore(unsigned int _foodcount_, const Square& _present_, const Square& _up_, const Square& _left_, const Square& _down_, const Square& _right_, DecisionTree::ReturnValue _result_, DecisionTree::ReturnValue _last_)
			: foodcount(_foodcount_), present (_present_), up(_up_), left(_left_), down(_down_), right(_right_), result(_result_), last(_last_)
		{}

		ActionStore(unsigned int _foodcount_, DecisionTree::ReturnValue _last_, 
			const Square& _present_, const Square& _up_, const Square& _left_, const Square& _down_, const Square& _right_,
			int _upleft_, int _upright_, int _downright_, int _downleft_, int _upup_, int _rightright_, int _downdown_, int _leftleft_, 
			DecisionTree::ReturnValue _result_) 
			: foodcount(_foodcount_), 
			present(_present_), up(_up_), left(_left_), down(_down_), right(_right_), 
			upleft(_upleft_), upright(_upright_), downright(_downright_), downleft(_downleft_), upup(_upup_), rightright(_rightright_), downdown(_downdown_), leftleft(_leftleft_),
			result(_result_), last(_last_)
		{
		}
	};

}

#endif