#include "CallStack.h"

void CallStack::push(std::shared_ptr<Variable> v)
{
	cs.push(v);
}

std::shared_ptr<Variable> CallStack::top()
{
	return cs.top();
}

size_t CallStack::size()
{
	return cs.size();
}

bool CallStack::empty()
{
	return cs.empty();
}

void CallStack::pop()
{
	//delete cs.top();
	cs.pop();
}