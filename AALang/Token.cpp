#include "Token.h"

Token::Token(std::string value, TokenType type)
	:value(value), type(type)
{

}

std::string Token::typeToString()
{
	if (type == TokenType::T_AssignmentOperator)
		return "T_AssignmentOperator";
	if (type == TokenType::T_ArithmeticOperator)
		return "T_ArithmeticOperator";
	if (type == TokenType::T_Identifier)
		return "T_Identifier";
	if (type == TokenType::T_Number)
		return "T_Number";
	if (type == TokenType::T_OpenParenthesis)
		return "T_OpenParenthesis";
	if (type == TokenType::T_CloseParenthesis)
		return "T_CloseParenthesis";
	if (type == TokenType::T_OpenSquareBracket)
		return "T_OpenSquareBracket";
	if (type == TokenType::T_CloseSquareBracket)
		return "T_CloseSquareBracket";
	if (type == TokenType::T_Comma)
		return "T_Comma";
	if (type == TokenType::T_String)
		return "T_String";
	if (type == TokenType::T_Block)
		return "T_Block";
	if (type == TokenType::T_WhiteSpace)
		return "T_WhiteSpace";
	if (type == TokenType::T_EndOfLine)
		return "T_EndOfLine";

	return "";
}
