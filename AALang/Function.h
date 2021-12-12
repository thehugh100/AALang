#pragma once

#include <string>
#include <functional>

class Variable;
class CallStack;

typedef std::function<Variable* (CallStack*)> Action;

class Function
{
public:
	Function(std::string identifier, int parameterCount, Action action);
	Variable* execute(CallStack* p);

	std::string identifier;
	int parameterCount;
	Action action;
};