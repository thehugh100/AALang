#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <filesystem>
#include <fstream>

struct Variable
{
	enum class VariableType {
		P_String = 0,
		P_Float
	};

	Variable(std::string value)
	{
		type = VariableType::P_String;
		sValue = value;
		fValue = 0;
	}
	Variable(float value)
	{
		type = VariableType::P_Float;
		sValue = "";
		fValue = value;
	}

	std::string toString()
	{
		if (type == VariableType::P_String)
			return sValue;

		if (type == VariableType::P_Float)
			return std::to_string(fValue);

		return "";
	}

	VariableType type;
	std::string sValue;
	float fValue;
};

struct Token
{
	enum class TokenType {
		T_WhiteSpace = 0,
		T_Identifier,
		T_Number,
		T_AssignmentOperator,
		T_OpenParenthesis,
		T_CloseParenthesis,
		T_Comma,
		T_String,
		T_EndOfLine,
	};

	Token(std::string value, TokenType type)
		:value(value), type(type)
	{

	}

	std::string typeToString()
	{
		if (type == TokenType::T_AssignmentOperator)
			return "T_AssignmentOperator";
		if (type == TokenType::T_Identifier)
			return "T_Identifier";
		if (type == TokenType::T_Number)
			return "T_Number";
		if (type == TokenType::T_OpenParenthesis)
			return "T_OpenParenthesis";
		if (type == TokenType::T_CloseParenthesis)
			return "T_CloseParenthesis";
		if (type == TokenType::T_Comma)
			return "T_Comma";
		if (type == TokenType::T_String)
			return "T_String";
		if (type == TokenType::T_WhiteSpace)
			return "T_WhiteSpace";
		if (type == TokenType::T_EndOfLine)
			return "T_EndOfLine";

		return "";
	}

	std::string value;
	TokenType type;
};

typedef std::vector<Token> TokenList;

struct CallStack
{
	void push(Variable* v)
	{
		cs.push(v);
	}

	Variable* top()
	{
		return cs.top();
	}

	size_t size()
	{
		return cs.size();
	}

	bool empty()
	{
		return cs.empty();
	}

	void pop()
	{
		delete cs.top();
		cs.pop();
	}

	std::stack<Variable*> cs;
};

typedef std::function<void(CallStack*)> Action;

struct Function
{
	Function(std::string identifier, int parameterCount, Action action)
		:identifier(identifier), parameterCount(parameterCount), action(action)
	{
	}

	void execute(CallStack *p)
	{
		if (p->size() >= parameterCount)
		{
			action(p);
		}
		else
		{
			std::cout << "Runtime Error: Call to " << identifier << "() failed. Too few stack items for call." << std::endl;
		}
	}

	std::string identifier;
	int parameterCount;
	Action action;
};

typedef std::vector<std::string> Program;

bool isIdentifierChar(char v)
{
	return ((v >= 'A' && v <= 'Z') || (v >= 'a' && v <= 'z') || v == '_');
}
bool isNumericChar(char v)
{
	return ((v >= '0' && v <= '9') || v == '.');
}

struct AALang
{
	AALang()
	{
		registerSTDLib();
	}

	void registerSTDLib()
	{
		registerFunction(
			new Function("print", 1, [](CallStack* p) {
				std::cout << p->top()->toString() << std::endl;

				p->pop();
			}
		));
		registerFunction(
			new Function("printv", 3, [](CallStack* p) {

				int params = p->top()->fValue;
				p->pop();

				for (int i = 0; i < params; ++i)
				{
					std::cout << p->top()->toString() << std::endl;
					p->pop();

				}

			}
		));
	}

	void call(std::string identifier)
	{
		if (functions.find(identifier) != functions.end())
		{
			if (!callStack.empty())
			{
				functions.at(identifier)->execute(&callStack);
			}
			else
			{
				std::cout << "Runtime Error: Callstack is empty" << std::endl;
			}
		}
	}

	Function* registerFunction(Function* newFunc)
	{
		std::cout << "Registered Function: " << newFunc->identifier << "()" << std::endl;
		functions[newFunc->identifier] = newFunc;

		return newFunc;
	}

	Variable* assignVariable(std::string identifier, Variable* newVar)
	{
		if (variables.find(identifier) != variables.end())
		{
			std::cout << identifier << " = " << newVar->toString() << std::endl;

			delete variables[identifier];
			variables[identifier] = newVar;
		}
		else
		{
			variables[identifier] = newVar;
		}

		return newVar;
	}

	void tokenizeLine(std::string line, TokenList* list)
	{
		std::string currentValue;
		bool foundIdentifier = 0;
		bool foundNumber = 0;
		bool foundString = 0;
		bool inQuote = 0;

		for (int i = 0; i < line.size(); ++i)
		{
			char v = line[i];
			if (v == '"')
			{
				inQuote = !inQuote;
				continue;
			}

			if (isNumericChar(v) && !foundIdentifier && !inQuote)
			{
				currentValue += v;
				foundNumber = true;
			}
			if (isIdentifierChar(v) && !inQuote)
			{
				currentValue += v;
				foundIdentifier = true;
				foundNumber = false;
			}
			if (inQuote)
			{
				currentValue += v;
				foundString = true;
				foundIdentifier = false;
				foundNumber = false;
			}

			if (v == '=' || v == ';' || v == '(' || v == ')' || v == ',')
			{
				if (foundIdentifier)
				{
					list->push_back(Token(currentValue, Token::TokenType::T_Identifier));
				}
				if (foundNumber)
				{
					list->push_back(Token(currentValue, Token::TokenType::T_Number));
				}
				if (foundString)
				{
					list->push_back(Token(currentValue, Token::TokenType::T_String));
				}

				foundIdentifier = false;
				foundNumber = false;
				foundString = false;

				currentValue = "";

				if(v == '=')
					list->push_back(Token("=", Token::TokenType::T_AssignmentOperator));
				if (v == ';')
					list->push_back(Token(";", Token::TokenType::T_EndOfLine));
				if (v == '(')
					list->push_back(Token("(", Token::TokenType::T_OpenParenthesis));
				if (v == ')')
					list->push_back(Token(")", Token::TokenType::T_CloseParenthesis));
				if (v == ',')
					list->push_back(Token(",", Token::TokenType::T_Comma));
			}
		}
	}

	Variable* processImmediate(Token in)
	{
		Variable* immediate = nullptr;
		if (in.type == Token::TokenType::T_Identifier)
		{
			if (functions.find(in.value) != functions.end())
			{
				std::cout << "Parse Error: immiate functions not yet supported..." << std::endl;
			}
			else if (variables.find(in.value) != variables.end())
			{
				immediate = variables[in.value];
			}
			else
			{
				std::cout << "Parse Error: Unknown Identifier '" << in.value << "'" << std::endl;
			}
		}
		else if (in.type == Token::TokenType::T_Number)
		{
			immediate = new Variable(std::stof(in.value));
		}
		else if (in.type == Token::TokenType::T_String)
		{
			immediate = new Variable(in.value);
		}
		else
		{
			std::cout << "Parse Error: Unexpected Token '" << in.value << " " << in.typeToString() << "'" << std::endl;
		}

		return immediate;
	}

	void executeTokens(TokenList* list)
	{
		std::string lParamIdentifier = "";
		Variable* lParamV = nullptr;
		bool lParamIsVar = false;
		bool lParamIsFunc = false;
		bool lParam = 0;
		bool assignment = false;

		Variable* rParamV = nullptr;
		bool rParamIsFunc = false;
		bool rParam = 0;

		std::vector<Variable*> params;

		bool pushing = false;

		for (auto& i : *list)
		{
			if (!lParam)
			{
				if (i.type == Token::TokenType::T_Identifier)
				{
					lParamIdentifier = i.value;

					if (functions.find(i.value) != functions.end())
					{
						lParamIsFunc = true;
						lParam = true;
						continue;
					}
					if (variables.find(i.value) != variables.end())
					{
						lParamV = variables[i.value];
						lParamIsVar = true;
						lParam = true;
						continue;
					}
					else
					{
						lParamV = assignVariable(i.value, new Variable(0));
						lParamIsVar = true;
						lParam = true;
					}
				}
			}
			else
			{
				if (i.type == Token::TokenType::T_AssignmentOperator)
				{
					assignment = true;
					continue;
				}
				else if (i.type == Token::TokenType::T_EndOfLine)
				{
					break;
				}
				else if (i.type == Token::TokenType::T_OpenParenthesis)
				{
					if (!assignment && !lParamIsFunc)
					{
						std::cout << "Parse Error: Unexpected Token '(', " << lParamIdentifier << " is not a function." << std::endl;
						break;
					}
					pushing = true;
					continue;
				}
				else if (i.type == Token::TokenType::T_Comma)
				{
					continue;
				}
				else if (i.type == Token::TokenType::T_CloseParenthesis)
				{
					
					for (int e = params.size() - 1; e >= 0; e--)
					{
						callStack.push(params[e]);
					}

					pushing = false;
					continue;
				}
				else
				{
					rParam = true;
					rParamV = processImmediate(i);
					if (pushing)
					{
						params.push_back(rParamV);
					}
				}
			}
		}

		if (assignment)
		{
			if (lParam && rParam)
			{
				lParamV->type = rParamV->type;
				lParamV->fValue = rParamV->fValue;
				lParamV->sValue = rParamV->sValue;
			}
		}
		else
		{
			if (lParam)
			{
				if (lParamIsFunc)
				{
					call(lParamIdentifier);
				}
				else
				{
					if (lParamV)
					{
						std::cout << lParamV->toString() << std::endl;
					}
				}
			}
		}
	}

	std::string executeLine(std::string line)
	{
		TokenList tokens;
		tokenizeLine(line, &tokens);

		executeTokens(&tokens);

		std::cout << std::endl;
		for (auto& i : tokens)
		{
			std::cout << "" << i.value << "" << "\t(" << i.typeToString() <<  ")" << std::endl;
		}

		return "";
	}

	CallStack callStack;
	std::map<std::string, Function*> functions;
	std::map<std::string, Variable*> variables;
};

void parseDocument(std::filesystem::path filepath, Program *p)
{
	if (!std::filesystem::exists(filepath))
	{
		std::cout << "Path does not exist: " << filepath << std::endl;
		return;
	}

	if(!std::filesystem::is_regular_file(filepath))
	{
		std::cout << "Path is not a file: " << filepath << std::endl;
		return;
	}

	std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::cout << "Unable to open file: " << filepath << std::endl;
		return;
	}

	char* data = nullptr;
	size_t size = 0;

	size = file.tellg();
	file.seekg(0, file.beg);
	data = new char[size];
	file.read(data, size);
	file.close();

	p->push_back("");
	for (int i = 0; i < size; ++i)
	{
		if(data[i] != '\n' && data[i] != '\r')
			p->back() += data[i];

		if (data[i] == ';')
			p->push_back("");
	}
	p->pop_back();
}

int main()
{
	AALang* aaLang = new AALang();

	Program program;
	parseDocument("test.aal", &program);

	int lineNum = 1;
	for (auto& i : program)
	{
		std::cout << i << std::endl;
		std::cout << aaLang->executeLine(i) << std::endl;
	}

}