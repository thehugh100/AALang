#include <iostream>
#include "Function.h"
#include "CallStack.h"

Function::Function(std::string identifier, int parameterCount, Action action)
	:identifier(identifier), parameterCount(parameterCount), action(action)
{
}

std::shared_ptr<Variable> Function::execute(CallStack* p)
{
	if (p->size() >= parameterCount)
	{
		return action(p);
	}
	else
	{
		std::cout << "Runtime Error: Call to " << identifier << "() failed. Too few stack items for call. returning nullptr from Function::execute()." << std::endl;
	}

	return nullptr;
}