#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Tokenizer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Logger.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./lisp <source.lisp>" << std::endl;
		return EXIT_FAILURE;
	}

	std::stringstream src;
	std::ifstream file(argv[1]);
	
	if (!file || !file.is_open())
	{
		std::cerr << "Could not open file: " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}

	std::string line;
	while (std::getline(file, line))
	{
		src << line;
	}

	LOG_DEBUG("Source: " << src.str() << std::endl);

	file.close();

	Tokenizer tokenizer(src.str());
	std::vector<Token> tokens = tokenizer.tokenize();
	LOG_DEBUG("TOKENS:" << std::endl);
	for (int i = 0; i < tokens.size(); i++)
	{
		Token& token = tokens[i];
		LOG_DEBUG(Tokenizer::tokentype_to_string(token.type));
		if (i != tokens.size() - 1)
		{
			LOG_DEBUG(", ");
		}
	}
	LOG_DEBUG(std::endl);
	LOG_DEBUG("Parsing..." << std::endl);

	Parser parser(tokens);
	std::vector<Node::Stmt> stmts = parser.parse_prog();

	LOG_DEBUG("Statements :" << std::endl);

	for (const Node::Stmt& stmt : stmts)
	{
		LOG_DEBUG(Parser::node_to_string(stmt) << std::endl);
	}

	LOG_DEBUG("Finished Statements" << std::endl);

	LOG_DEBUG("Parsing completed" << std::endl);

	LOG_DEBUG("Interpreting..." << std::endl);
	
	Interpreter interpreter(stmts);
	interpreter.interpret_prog();

	LOG_DEBUG("Interpreting completed" << std::endl);


	return EXIT_SUCCESS;
}