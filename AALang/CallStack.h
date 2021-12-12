#pragma once

#include <stack>

class Variable;

class CallStack
{
public:
	void push(Variable* v);
	Variable* top();
	size_t size();
	bool empty();
	void pop();

	std::stack<Variable*> cs;
};