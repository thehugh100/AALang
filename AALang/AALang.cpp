#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>


struct Variable
{
	enum class VariableType {
		P_NULL = 0,
		P_String,
		P_Float,
		P_Block,
	};

	Variable()
	{
		type = VariableType::P_NULL;
		sValue = "";
		fValue = 0;
	}
	Variable(std::string value, bool block = false)
	{
		type = VariableType::P_String;
		sValue = value;
		fValue = 0;

		if (block)
		{
			type = VariableType::P_Block;
		}
	}
	Variable(float value)
	{
		type = VariableType::P_Float;
		sValue = "";
		fValue = value;
	}

	std::string toString()
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
		T_Block,
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
		if (type == TokenType::T_Block)
			return "T_Block";
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

typedef std::function<Variable*(CallStack*)> Action;

struct Function
{
	Function(std::string identifier, int parameterCount, Action action)
		:identifier(identifier), parameterCount(parameterCount), action(action)
	{
	}

	Variable* execute(CallStack *p)
	{
		if (p->size() >= parameterCount)
		{
			return action(p);
		}
		else
		{
			std::cout << "Runtime Error: Call to " << identifier << "() failed. Too few stack items for call." << std::endl;
		}

		return nullptr;
	}

	std::string identifier;
	int parameterCount;
	Action action;
};

typedef std::vector<std::string> Program;
void parseDocument(std::filesystem::path filepath, Program* p);
void preParse(const char* data, size_t size, Program* p);

bool isIdentifierChar(char v)
{
	return ((v >= 'A' && v <= 'Z') || (v >= 'a' && v <= 'z') || v == '_');
}
bool isNumericChar(char v)
{
	return ((v >= '0' && v <= '9') || v == '.' || v == '-');
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
			new Function("if", 2, [this](CallStack* p) {
				int eval = p->top()->fValue;
				p->pop();

				Variable* block = p->top();
				
				if (eval)
				{
					executeBlock(block->sValue);
				}

				p->pop();
				return new Variable();
			}
		));
		registerFunction(
			new Function("print", 1, [](CallStack* p) {
				std::cout << p->top()->toString() << std::endl;
				p->pop();
				return new Variable();
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
				return new Variable();
			}
		));
		registerFunction(
			new Function("add", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 + v2);
			}
		));
		registerFunction(
			new Function("sub", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 - v2);
			}
		));
		registerFunction(
			new Function("mul", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 * v2);
			}
		));
		registerFunction(
			new Function("div", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p ->pop();
				return new Variable(v1 / v2);
			}
		));
		registerFunction(
			new Function("mod", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(((int)v1 % (int)v2));
			}
		));
		registerFunction(
			new Function("cmd", 1, [](CallStack* p) {
				std::string cmd = p->top()->sValue;
				p->pop();

				std::array<char, 128> buffer;
				std::string result;
				std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
				if (!pipe) {
					throw std::runtime_error("popen() failed!");
				}
				while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
					result += buffer.data();
				}

				return new Variable(result);
			}
		));
	}

	Variable* call(std::string identifier)
	{
		if (functions.find(identifier) != functions.end())
		{
			if (!callStack.empty())
			{
				return functions.at(identifier)->execute(&callStack);
			}
			else
			{
				std::cout << "Runtime Error: Callstack is empty" << std::endl;
			}
		}
		return nullptr;
	}

	Function* registerFunction(Function* newFunc)
	{
		//std::cout << "Registered Function: " << newFunc->identifier << "()" << std::endl;
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
		int blockCount = 0;

		for (int i = 0; i < line.size(); ++i)
		{
			char v = line[i];
			if (v == '"' && blockCount == 0)
			{
				inQuote = !inQuote;
				continue;
			}
			if (!inQuote)
			{
				if (v == '{')
					blockCount++;
				else if (v == '}')
				{
					blockCount--;
					list->push_back(Token(currentValue, Token::TokenType::T_Block));
					currentValue = "";
				}
				else
				{
					if (blockCount == 1)
					{
						currentValue += v;
						continue;
					}
				}
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
				continue;
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
		if (inQuote)
		{
			std::cout << "Parse Error: Quote mismatch." << std::endl;
		}
	}

	void executeBlock(std::string block)
	{
		Program t;
		preParse(block.c_str(), block.size(), &t);

		for (auto& i : t)
		{
			std::cout << i << std::endl;
			std::cout << executeLine(i) << std::endl;
		}
	}

	Variable* processImmediate(Token in, bool createIfNotExists = false)
	{
		Variable* immediate = nullptr;
		if (in.type == Token::TokenType::T_Identifier)
		{
			if (variables.find(in.value) != variables.end())
			{
				immediate = variables[in.value];
			}
			else
			{
				if (createIfNotExists)
				{
					immediate = assignVariable(in.value, new Variable());
				}
				else
				{
					//return NULL
					immediate = new Variable();
					std::cout << "Parse Error: Unknown Identifier '" << in.value << "'" << std::endl;
				}
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
		else if (in.type == Token::TokenType::T_Block)
		{
			if (createIfNotExists)
			{
				executeBlock(in.value);
			}
			else
			{
				immediate = new Variable(in.value, true);
			}
		}
		else
		{
			std::cout << "Parse Error: Unexpected Token '" << in.value << " " << in.typeToString() << "'" << std::endl;
		}

		return immediate;
	}

	Variable* evaluateExpression(TokenList* list, bool createIfNotExists = false)
	{
		// return null if expression is empty
		if (list->empty())
		{
			return new Variable();
		}
		else
		{
			bool isFunction = 0;
			// is immediate value ?
			if (list->size() == 1)
			{
				return processImmediate(list->at(0), createIfNotExists);
			}
			else
			{
				if (list->size() >= 3)
				{
					if (list->at(0).type == Token::TokenType::T_Identifier && list->at(1).type == Token::TokenType::T_OpenParenthesis)
					{
						isFunction = true;
						int parenthesisCount = 0;
						std::string identifier = list->at(0).value;

						TokenList subList;
						std::stack<Variable*> stack;

						for (int i = 2; i < list->size()-1; ++i)
						{
							auto currentType = list->at(i).type;

							if (currentType == Token::TokenType::T_OpenParenthesis)
							{
								parenthesisCount++;
							}
							else if (currentType == Token::TokenType::T_CloseParenthesis)
							{
								parenthesisCount--;
							}
							

							if (parenthesisCount == 0)
							{
								if (currentType == Token::TokenType::T_Comma)
								{
									stack.push(evaluateExpression(&subList));
									subList.clear();
								}
								else
								{
									subList.push_back(list->at(i));
								}
							}
							else
							{
								subList.push_back(list->at(i));
							}
						}

						stack.push(evaluateExpression(&subList));
						subList.clear();

						if (parenthesisCount == 0)
						{
							while (!stack.empty())
							{
								callStack.push(stack.top());
								stack.pop();
							}
							return call(identifier);
						}
						else
						{
							std::cout << "Parse Error: Parenthesis mismatch" << std::endl;
							return nullptr;
						}
					}
				}
			}
		}
		return nullptr;
	}

	void executeTokens(TokenList* list)
	{
		TokenList lParam;
		TokenList rParam;
		bool foundlParam = false;
		bool assignment = false;
		for (auto& i : *list)
		{
			if (i.type == Token::TokenType::T_AssignmentOperator)
			{
				foundlParam = true;
				assignment = true;
			}
			else
			{
				if (i.type == Token::TokenType::T_EndOfLine)
					continue;

				if (!foundlParam)
					lParam.push_back(i);
				else
					rParam.push_back(i);
			}
		}

		Variable* lParamV = evaluateExpression(&lParam, true);
		Variable* rParamV = evaluateExpression(&rParam);

		if (assignment)
		{
			lParamV->type = rParamV->type;
			lParamV->fValue = rParamV->fValue;
			lParamV->sValue = rParamV->sValue;
		}
		else
		{
			if (lParamV)
			{
				if (lParamV->type == Variable::VariableType::P_Block)
					executeBlock(lParamV->sValue);
			}
		}
	}

	std::string executeLine(std::string line)
	{
		TokenList tokens;
		tokenizeLine(line, &tokens);

		executeTokens(&tokens);

		//std::cout << std::endl;
		/*for (auto& i : tokens)
		{
			std::cout << "" << i.value << "" << "\t(" << i.typeToString() <<  ")" << std::endl;
		}*/

		return "";
	}

	CallStack callStack;
	std::map<std::string, Function*> functions;
	std::map<std::string, Variable*> variables;
};

void preParse(const char *data, size_t size, Program* p)
{
	p->push_back("");
	for (int i = 0; i < size; ++i)
	{
		if(data[i] != '\n' && data[i] != '\r')
			p->back() += data[i];

		if (data[i] == '\n')
			p->push_back("");
	}
}

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

	preParse(data, size, p);
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