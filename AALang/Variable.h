#pragma once

#include <string>

class Variable
{
public:
	enum class VariableType {
		P_NULL = 0,
		P_String,
		P_Float,
		P_Block,
	};

	Variable();
	Variable(std::string value, bool block = false);
	Variable(float value);

	std::string toString();

	VariableType type;
	std::string sValue;
	float fValue;
};