#pragma once

#include <string>
#include <map>

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

	VariableType type;
	std::string sValue;
	std::map<std::string, Variable*> mValue;
	float fValue;
};