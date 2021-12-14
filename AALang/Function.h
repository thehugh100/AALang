#pragma once

#include <string>
#include <functional>

class Variable;
class CallStack;

typedef std::function<std::shared_ptr<Variable> (CallStack*)> Action;

class Function
{
public:
	Function(std::string identifier, int parameterCount, Action action);
	std::shared_ptr<Variable> execute(CallStack* p);

	std::string identifier;
	int parameterCount;
	Action action;
};