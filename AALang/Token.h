#pragma once

#include <string>
#include <vector>

class Token
{
public:
	enum class TokenType {
		T_WhiteSpace = 0,
		T_Identifier,
		T_Number,
		T_AssignmentOperator,
		T_ArithmeticOperator,
		T_OpenParenthesis,
		T_CloseParenthesis,
		T_OpenSquareBracket,
		T_CloseSquareBracket,
		T_Comma,
		T_String,
		T_Block,
		T_EndOfLine,
	};

	Token(std::string value, TokenType type);
	std::string typeToString();

	std::string value;
	TokenType type;
};

typedef std::vector<Token> TokenList;