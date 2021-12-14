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

#include <regex>

#include <chrono>

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
		null = std::make_shared<Variable>(); //default null
		isInForeach = false;
		value = nullptr;
		registerSTDLib();
		startTime = std::chrono::high_resolution_clock::now();
	}

	void registerSTDLib()
	{
		registerFunction(
			new Function("timeMS", 0, [this](CallStack* p) {
				return std::make_shared<Variable>((std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count()));
			}
		));

		registerFunction(
			new Function("while", 2, [this](CallStack* p) {
				std::shared_ptr<Variable> eval = p->top();
				p->pop();

				if (eval->type != Variable::VariableType::P_Block)
				{
					std::cout << "Runtime Error: 1st parameter of while() must be a block!" << std::endl;
					return null;
				}

				std::shared_ptr<Variable> block = p->top();
				p->pop();

				if (eval->type != Variable::VariableType::P_Block)
				{
					std::cout << "Runtime Error: 2nd parameter of while() must be a block!" << std::endl;
					return null;
				}

				while ((int)(executeBlock(eval->sValue)->fValue) != 0)
				{
					executeBlock(block->sValue);
				}
				return null;
			}
		));

		registerFunction(
			new Function("foreach", 4, [this](CallStack* p) {
			std::shared_ptr<Variable> v = p->top();
			p->pop();

			if (v->type != Variable::VariableType::P_Map)
			{
				std::cout << "Runtime Error: 1st parameter of foreach() must be a Map!" << std::endl;
				return null;
			}

			std::shared_ptr<Variable> key = p->top();
			p->pop();
			std::shared_ptr<Variable> val = p->top();
			p->pop();

			std::shared_ptr<Variable> block = p->top();
			p->pop();


			if (block->type != Variable::VariableType::P_Block)
			{
				std::cout << "Runtime Error: 2nd parameter of foreach() must be a block!" << std::endl;
				return null;
			}

			isInForeach = true;
			for(auto &i : v->mValue)
			{
				key->sValue = i.first;
				key->type = Variable::VariableType::P_String;

				val->sValue = i.second->sValue;
				val->fValue = i.second->fValue;
				val->mValue = i.second->mValue;
				val->type = i.second->type;

				executeBlock(block->sValue);
			}
			isInForeach = false;

			return null;
		}
		));
		registerFunction(
			new Function("value", 0, [this](CallStack* p) {
				if (!isInForeach)
				{
					std::cout << "Runtime Error: value() cannot be called outside of foreach()" << std::endl;
					return null;
				}
				return value;
			}
		));
		registerFunction(
			new Function("setMap", 3, [](CallStack* p) {
				std::shared_ptr<Variable> lVal = p->top();
				p->pop();
				std::string key = p->top()->toString();
				p->pop();
				std::shared_ptr<Variable> rVal = p->top();
				p->pop();

				lVal->type = Variable::VariableType::P_Map;
				lVal->mValue[key] = rVal;

				return lVal;
			}
		));
		registerFunction(
			new Function("getMap", 2, [](CallStack* p) {
				std::shared_ptr<Variable> lVal = p->top();
				p->pop();
				std::string key = p->top()->toString();
				p->pop();

				return lVal->mValue[key];
			}
		));
		registerFunction(
			new Function("if", 2, [this](CallStack* p) {
				int eval = p->top()->fValue;
				p->pop();

				std::shared_ptr<Variable> block = p->top();
				
				if (eval)
				{
					executeBlock(block->sValue);
				}

				p->pop();
				return null;
			}
		));
		registerFunction(
			new Function("ifelse", 3, [this](CallStack* p) {
				int eval = p->top()->fValue;
				p->pop();
				if (!eval)
					p->pop();

				std::shared_ptr<Variable> block = p->top();
				executeBlock(block->sValue);

				if (eval)
					p->pop();

				p->pop();
				return null;
				}
		));
		registerFunction(
			new Function("print", 1, [this](CallStack* p) {
				std::cout << p->top()->toString();
				p->pop();
				return null;
			}
		));
		//registerFunction(
		//	new Function("printv", 3, [](CallStack* p) {

		//		int params = p->top()->fValue;
		//		p->pop();

		//		for (int i = 0; i < params; ++i)
		//		{
		//			std::cout << p->top()->toString() << std::endl;
		//			p->pop();
		//		}
		//		null;
		//	}
		//));
		registerFunction(
			new Function("equals", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 == v2);
				}
		));
		registerFunction(
			new Function("lt", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 < v2);
			}
		));
		registerFunction(
			new Function("gt", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 > v2);
				}
		));
		registerFunction(
			new Function("lte", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 <= v2);
				}
		));
		registerFunction(
			new Function("gte", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 >= v2);
				}
		));
		registerFunction(
			new Function("add", 2, [](CallStack* p) {
				bool isStringV1 = p->top()->type == Variable::VariableType::P_String;
				std::string v1s = p->top()->sValue;
				float v1 = p->top()->fValue;
				p->pop();

				bool isStringV2 = p->top()->type == Variable::VariableType::P_String;
				std::string v2s = p->top()->sValue;
				float v2 = p->top()->fValue;
				p->pop();

				if (isStringV1 && isStringV2)
					return std::make_shared<Variable>(v1s + v2s);

				return std::make_shared<Variable>(v1 + v2);
			}
		));
		registerFunction(
			new Function("sub", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 - v2);
			}
		));
		registerFunction(
			new Function("mul", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(v1 * v2);
			}
		));
		registerFunction(
			new Function("div", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p ->pop();
				return std::make_shared<Variable>(v1 / v2);
			}
		));
		registerFunction(
			new Function("mod", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>(((int)v1 % (int)v2));
			}
		));
		registerFunction(
			new Function("abs", 1, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>((std::abs(v1)));
				}
		));
		registerFunction(
			new Function("and", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>((int)v1 && (int)v2);
			}
		));
		registerFunction(
			new Function("or", 2, [](CallStack* p) {
				float v1 = p->top()->fValue;
				p->pop();
				float v2 = p->top()->fValue;
				p->pop();
				return std::make_shared<Variable>((int)v1 || (int)v2);
			}
		));
		registerFunction(
			new Function("pop", 0, [](CallStack* p) {
				std::shared_ptr<Variable> temp = std::make_shared<Variable>();
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

				return std::make_shared<Variable>(result);
			}
		));
		registerFunction(
			new Function("getFileContents", 1, [this](CallStack* p) {
				std::string path = p->top()->sValue;

				std::ifstream file(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
				if (!file)
				{
					std::cout << "getFileContents(" << path << ") Error: Unable to open file" << std::endl;
					return null;
				}

				char* data = nullptr;
				size_t size = 0;

				size = file.tellg();
				file.seekg(0, file.beg);
				data = new char[size];
				file.read(data, size);
				file.close();

				return std::shared_ptr<Variable>(new Variable(std::string(data, size)));
			}
		));
		registerFunction(
			new Function("exit", 0, [this](CallStack* p) {
				exit(0);
				return null;
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
					std::shared_ptr<Variable> result = executeLine(i);
				}
				return null;
			}
		));
	}

	std::shared_ptr<Variable> call(std::string identifier)
	{
		if (functions.find(identifier) != functions.end())
		{
			if (!callStack.empty() || functions.at(identifier)->parameterCount == 0)
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
			return null;
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

	std::shared_ptr<Variable> assignVariable(std::string identifier, std::shared_ptr<Variable> newVar)
	{
		newVar->registered = true;
		if (variables.find(identifier) != variables.end())
		{
			//TODO: delete variables[identifier];
			variables[identifier] = newVar;
		}
		else
		{
			variables[identifier] = newVar;
		}

		return newVar;
	}

	//TODO: cache this
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
			if (v == '=' || v == ';' || v == '(' || v == ')' || v == '[' || v == ']' || v == ',' || v == '+' || v == '-' || v == '*' || v == '/')
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
					if (currentValue.find('\\') != -1)
					{
						currentValue = std::regex_replace(currentValue, std::regex("\\\\n"), "\n");
						currentValue = std::regex_replace(currentValue, std::regex("\\\\r"), "\r");
						currentValue = std::regex_replace(currentValue, std::regex("\\\\t"), "\t");
					}
					list->push_back(Token(currentValue, Token::TokenType::T_String));
				}

				foundIdentifier = false;
				foundNumber = false;
				foundString = false;

				currentValue = "";

				if(v == '=')
					list->push_back(Token("=", Token::TokenType::T_AssignmentOperator));
				if (v == '+')
					list->push_back(Token("+", Token::TokenType::T_ArithmeticOperator));
				if (v == '-')
					list->push_back(Token("-", Token::TokenType::T_ArithmeticOperator));
				if (v == '*')
					list->push_back(Token("*", Token::TokenType::T_ArithmeticOperator));
				if (v == '/')
					list->push_back(Token("/", Token::TokenType::T_ArithmeticOperator));
				if (v == ';')
					list->push_back(Token(";", Token::TokenType::T_EndOfLine));
				if (v == '(')
					list->push_back(Token("(", Token::TokenType::T_OpenParenthesis));
				if (v == ')')
					list->push_back(Token(")", Token::TokenType::T_CloseParenthesis));
				if (v == '[')
					list->push_back(Token("[", Token::TokenType::T_OpenSquareBracket));
				if (v == ']')
					list->push_back(Token("]", Token::TokenType::T_CloseSquareBracket));
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

	std::shared_ptr<Variable> executeBlock(std::string block)
	{
		std::shared_ptr<Variable> ret = nullptr;
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

	std::shared_ptr<Variable> processImmediate(Token in, bool createIfNotExists = false)
	{
		std::shared_ptr<Variable> immediate;
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
					immediate = assignVariable(in.value, std::make_shared<Variable>());
				}
				else
				{
					//return NULL
					immediate = null;
					std::cout << "Parse Error: Unknown Identifier '" << in.value << "'" << std::endl;
				}
			}
		}
		else if (in.type == Token::TokenType::T_Number)
		{
			immediate = std::shared_ptr<Variable>(new Variable(std::stof(in.value)));
		}
		else if (in.type == Token::TokenType::T_String)
		{
			immediate = std::shared_ptr<Variable>(new Variable(in.value));
		}
		else if (in.type == Token::TokenType::T_Block)
		{
			if (createIfNotExists)
			{
				immediate = executeBlock(in.value);
			}
			else
			{
				immediate = std::shared_ptr<Variable>(new Variable(in.value, true));
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

	std::shared_ptr<Variable> evaluateExpression(TokenList* list, bool createIfNotExists = false)
	{
		// return null if expression is empty
		if (list->empty())
		{
			return null;
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
				if (list->at(1).type == Token::TokenType::T_OpenSquareBracket) //must be an array index
				{
					bool foundMatchingSquareBracket = false;
					TokenList subList;
					for (int i = 2; i < list->size(); ++i)
					{
						auto currentType = list->at(i).type;
						if (currentType == Token::TokenType::T_CloseSquareBracket)
						{
							foundMatchingSquareBracket = true;
							break;
						}
						subList.push_back(list->at(i));
					}
					if (foundMatchingSquareBracket)
					{
						std::shared_ptr<Variable> index = evaluateExpression(&subList);
						std::shared_ptr<Variable> val = processImmediate(list->at(0), createIfNotExists);
						if (val)
						{
							if (createIfNotExists)
							{
								val->mValue[index->toString()] = null;
								val->type = Variable::VariableType::P_Map;
							}
						}

						return val->mValue[index->toString()];
					}
					else
					{
						std::cout << "Parse Error: square bracket mismatch, returning nullptr" << std::endl;
						return nullptr;
					}
				}
				if (list->size() >= 3)
				{
					if (list->at(0).type == Token::TokenType::T_Identifier && list->at(1).type == Token::TokenType::T_OpenParenthesis)
					{
						isFunction = true;
						int parenthesisCount = 0;
						std::string identifier = list->at(0).value;

						TokenList subList;
						std::stack<std::shared_ptr<Variable>> stack;

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
							std::cout << "Parse Error: Parenthesis mismatch, returning nullptr" << std::endl;
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

	std::shared_ptr<Variable> executeTokens(TokenList* list)
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

		std::shared_ptr<Variable> lParamV = evaluateExpression(&lParam, true);
		std::shared_ptr<Variable> rParamV = evaluateExpression(&rParam);

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

	std::shared_ptr<Variable> executeLine(std::string line)
	{
		TokenList *tokens;
		if (tokenCache.find(line) == tokenCache.end())
		{
			tokens = new TokenList();
			tokenizeLine(line, tokens);
			tokenCache[line] = tokens;
		}
		else
		{
			tokens = tokenCache[line];
		}

		std::shared_ptr<Variable> ret = executeTokens(tokens);
		
		if (ret == nullptr)
			std::cout << "Error: executeLine(" << line << ") returning nullptr." << std::endl;

		//std::cout << std::endl;
		/*for (auto& i : tokens)
		{
			std::cout << "" << i.value << "" << "\t(" << i.typeToString() <<  ")" << std::endl;
		}*/
		return ret;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

	bool isInForeach;
	std::shared_ptr<Variable> value;
	std::shared_ptr<Variable> null;

	std::map<std::string, TokenList*> tokenCache;
	CallStack callStack;
	std::map<std::string, Function*> functions;
	std::map<std::string, std::shared_ptr<Variable>> variables;
};

void preParse(const char *data, size_t size, Program* p)
{
	int blockCount = 0;
	int inQuote = 0;

	p->push_back("");
	for (int i = 0; i < size; ++i)
	{
		if (!inQuote)
		{
			if (size - i > 1)
			{
				if (data[i] == '/' && data[i + 1] == '/')
				{
					while (data[++i] != '\n');
				}
			}

			if (data[i] == '{')
				blockCount++;
			else if (data[i] == '}')
				blockCount--;
		}

		if (data[i] == '"')
			inQuote = !inQuote;
		
		if (data[i] != '\n' && data[i] != '\r')
		{
			p->back() += data[i];
		}

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
		std::shared_ptr<Variable> result = aaLang->executeLine(i);
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

		std::shared_ptr<Variable> result = aaLang->executeLine(cmd);
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