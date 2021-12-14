#include "Variable.h"
#include <iostream>

Variable::Variable()
{
	type = VariableType::P_NULL;
	sValue = "";
	fValue = 0;
	registered = false;
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
	registered = false;
}
Variable::Variable(float value)
{
	type = VariableType::P_Float;
	sValue = "";
	fValue = value;
	registered = false;
}

Variable::~Variable()
{
	//std::cout << "Deconstructing " << typeToString() << std::endl;
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

std::string Variable::typeToString()
{
	if (type == VariableType::P_NULL)
		return "P_NULL";

	if (type == VariableType::P_String)
		return "P_String";

	if (type == VariableType::P_Float)
		return "P_Float";

	if (type == VariableType::P_Block)
		return "P_Block";

	if (type == VariableType::P_Map)
		return "P_Map";

	return std::string();
}
