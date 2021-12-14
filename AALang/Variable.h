#pragma once

#include <string>
#include <map>
#include <memory>
class Variable;

class Variable
{
public:
	enum class VariableType {
		P_NULL = 0,
		P_String,
		P_Float,
		P_Block,
		P_Map,
	};

	Variable();
	Variable(std::string value, bool block = false);
	Variable(float value);

	~Variable();

	std::string toString();
	std::string typeToString();

	VariableType type;
	std::string sValue;
	std::map<std::string, std::shared_ptr<Variable>> mValue;
	float fValue;
	bool registered;
};