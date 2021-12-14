#pragma once
#include <stack>
#include<memory>

class Variable;

class CallStack
{
public:
	void push(std::shared_ptr<Variable> v);
	std::shared_ptr<Variable> top();
	size_t size();
	bool empty();
	void pop();

	std::stack<std::shared_ptr<Variable>> cs;
};