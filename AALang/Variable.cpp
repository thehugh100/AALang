#include "Variable.h"
#include <iostream>

Variable::Variable()
{
	type = VariableType::P_NULL;
	sValue = "";
	fValue = 0;
}
Variable::Variable(std::string value, bool block)
{
	type = VariableType::P_String;
	sValue = value;
	fValue = 0;

	if (block)
	{
		type = VariableType::P_Block;
	}
}
Variable::Variable(float value)
{
	type = VariableType::P_Float;
	sValue = "";
	fValue = value;
}

Variable::~Variable()
{
	//std::cout << "Deconstructing " << std::endl;
}

std::string Variable::toString()
{
	if (type == VariableType::P_NULL)
		return "NULL";

	if (type == VariableType::P_String)
		return sValue;

	if (type == VariableType::P_Float)
		return std::to_string(fValue);

	if (type == VariableType::P_Block)
		return "{ BLOCK }";

	return "";
}