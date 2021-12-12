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
#include <sstream>
#include <math.h>

#include "Variable.h"
#include "Token.h"
#include "CallStack.h"
#include "Function.h"

typedef std::vector<std::string> Program;

void loadProgram(std::filesystem::path filepath, Program* p);
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
			new Function("while", 2, [this](CallStack* p) {
				Variable* eval = p->top();
				p->pop();

				if (eval->type != Variable::VariableType::P_Block)
				{
					std::cout << "Runtime Error: 1st parameter of while() must be a block!" << std::endl;
					return new Variable();
				}

				Variable* block = p->top();
				p->pop();

				if (eval->type != Variable::VariableType::P_Block)
				{
					std::cout << "Runtime Error: 2nd parameter of while() must be a block!" << std::endl;
					return new Variable();
				}

				while ((int)(executeBlock(eval->sValue)->fValue) != 0)
				{
					executeBlock(block->sValue);
				}
				return new Variable();
			}
		));
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
			new Function("ifelse", 3, [this](CallStack* p) {
				int eval = p->top()->fValue;
				p->pop();
				if (!eval)
					p->pop();

				Variable* block = p->top();
				executeBlock(block->sValue);

				if (eval)
					p->pop();

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
			new Function("equals", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 == v2);
				}
		));
		registerFunction(
			new Function("lt", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 < v2);
			}
		));
		registerFunction(
			new Function("gt", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 > v2);
				}
		));
		registerFunction(
			new Function("lte", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 <= v2);
				}
		));
		registerFunction(
			new Function("gte", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(v1 >= v2);
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
			new Function("abs", 1, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				return new Variable((std::abs(v1)));
				}
		));
		registerFunction(
			new Function("and", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(((int)v1 && (int)v2));
			}
		));
		registerFunction(
			new Function("or", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return new Variable(((int)v1 || (int)v2));
			}
		));
		registerFunction(
			new Function("pop", 0, [](CallStack* p) {
				Variable* temp = new Variable();
				temp->sValue = p->top()->sValue;
				temp->fValue = p->top()->fValue;
				temp->type = p->top()->type;
				p->pop();
				return temp;
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
		registerFunction(
			new Function("getFileContents", 1, [this](CallStack* p) {
				std::string path = p->top()->sValue;

				std::ifstream file(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
				if (!file)
				{
					std::cout << "getFileContents(" << path << ") Error: Unable to open file" << std::endl;
					return new Variable();
				}

				char* data = nullptr;
				size_t size = 0;

				size = file.tellg();
				file.seekg(0, file.beg);
				data = new char[size];
				file.read(data, size);
				file.close();

				return new Variable(std::string(data, size));
			}
		));
		registerFunction(
			new Function("exit", 0, [this](CallStack* p) {
				exit(0);
				return new Variable();
			}
		));
		registerFunction(
			new Function("include", 1, [this](CallStack* p) {
				std::string path = p->top()->sValue;
				p->pop();

				Program program;
				loadProgram(path, &program);
				for (auto& i : program)
				{
					Variable* result = executeLine(i);
				}
				return new Variable();
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
		else if(variables.find(identifier) != variables.end())
		{
			return executeBlock(variables.at(identifier)->sValue);
		}
		else
		{
			//return null
			return new Variable();
		}

		std::cout << "Error: Call(" << identifier << ") returning nullptr" << std::endl;
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
				{
					blockCount++;
					if(blockCount == 1)
						continue;
				}
				else if (v == '}')
				{
					blockCount--;
					if (blockCount == 0)
					{
						list->push_back(Token(currentValue, Token::TokenType::T_Block));
						currentValue = "";
						continue;
					}
				}
				
				if (blockCount > 0)
				{
					currentValue += v;
					continue;
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
		if (blockCount != 0)
		{
			std::cout << "Parse Error: Block mismatch" << std::endl;
		}
	}

	Variable* executeBlock(std::string block)
	{
		Variable* ret = nullptr;
		Program t;
		preParse(block.c_str(), block.size(), &t);

		for (auto& i : t)
		{
			ret = executeLine(i);
		}

		if (ret == nullptr)
		{
			std::cout << "Error: executeBlock(" << block << ") returning nullptr." << std::endl;
		}

		return ret;
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
				immediate = executeBlock(in.value);
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

		if (immediate == nullptr)
		{
			std::cout << "Error: processImmediate(" << in.value << " [" << in.typeToString() << "]) returning nullptr." << std::endl;
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
						if (!subList.empty())
						{
							stack.push(evaluateExpression(&subList));
							subList.clear();
						}

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

	std::string TokenListToString(TokenList* list)
	{
		std::ostringstream buf;
		for (auto& i : *list)
			buf << i.value << "[" << i.typeToString() << "]" << std::endl;

		return buf.str();
	}

	Variable* executeTokens(TokenList* list)
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

		if (lParamV == nullptr)
		{
			std::cout << "Error: executeTokens(" << std::endl << TokenListToString(list) << ") returning nullptr." << std::endl;
		}
		return lParamV;
	}

	Variable* executeLine(std::string line)
	{
		TokenList tokens;
		tokenizeLine(line, &tokens);

		Variable* ret = executeTokens(&tokens);
		
		if (ret == nullptr)
			std::cout << "Error: executeLine(" << line << ") returning nullptr." << std::endl;

		//std::cout << std::endl;
		/*for (auto& i : tokens)
		{
			std::cout << "" << i.value << "" << "\t(" << i.typeToString() <<  ")" << std::endl;
		}*/
		return ret;
	}

	CallStack callStack;
	std::map<std::string, Function*> functions;
	std::map<std::string, Variable*> variables;
};

void preParse(const char *data, size_t size, Program* p)
{
	/*p->push_back("");
	for (int i = 0; i < size; ++i)
	{
		if(data[i] != '\n' && data[i] != '\r')
			p->back() += data[i];

		if (data[i] == '\n')
			p->push_back("");
	}*/

	int blockCount = 0;
	int inQuote = 0;
	p->push_back("");
	for (int i = 0; i < size; ++i)
	{
		if (!inQuote)
		{
			if (data[i] == '{')
				blockCount++;
			else if (data[i] == '}')
				blockCount--;
		}
		if (blockCount == 0)
		{
			if (data[i] == '"')
				inQuote = !inQuote;
		}
		if (data[i] != '\n' && data[i] != '\r')
			p->back() += data[i];
		if (data[i] == ';' && !inQuote && blockCount == 0)
			p->push_back("");
	}
	p->pop_back();
	if (blockCount != 0)
	{
		std::cout << "PreParse Error: Block mismatch" << std::endl;
	}
}

void loadProgram(std::filesystem::path filepath, Program *p)
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
	loadProgram("test.aal", &program);

	int lineNum = 1;
	for (auto& i : program)
	{
		Variable* result = aaLang->executeLine(i);
		if (result != nullptr)
		{
			std::cout << ">> " <<  result->toString() << std::endl;
		}
		else
		{
			std::cout << "Error: nullptr returned on line " << lineNum << std::endl;
		}
		lineNum++;
	}
	std::string cmd;
	while (true)
	{
		std::cout << ">> ";
		std::getline(std::cin, cmd);
		if (cmd == "quit" || cmd == "exit")
			break; 

		Variable* result = aaLang->executeLine(cmd);
		if (result != nullptr)
		{
			std::cout << ">> " << result->toString() << std::endl;
		}
		else
		{
			std::cout << "Error: nullptr returned on line " << lineNum << std::endl;
		}
	}
}