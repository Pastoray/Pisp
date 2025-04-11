#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Tokenizer.h"
#include "Parser.h"
#include "Compiler.h"
#include "Utils.h"

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

	logger << "Source: " << src.str() << std::endl;

	file.close();

	Tokenizer tokenizer(src.str());
	std::vector<Token> tokens = tokenizer.tokenize();
	logger << "TOKENS:" << std::endl;
	for (int i = 0; i < tokens.size(); i++)
	{
		Token& token = tokens[i];
		logger << Tokenizer::tokentype_to_string(token.type);
		if (i != tokens.size() - 1)
		{
			logger << ", ";
		}
	}
	logger << std::endl;
	logger << "Parsing..." << std::endl;

	Parser parser(tokens);
	std::vector<Node::Node> nodes = parser.parse_prog();

	logger << "Statements :" << std::endl;

	for (const Node::Node& node : nodes)
	{
		logger << Parser::node_to_string(node) << std::endl;
	}

	logger << "Finished Statements" << std::endl;

	logger << "Parsing completed" << std::endl;

	logger << "Compiling..." << std::endl;

	Compiler compiler(nodes);
	const auto vec = compiler.compile_prog();

	logger << "Compilation completed\n" << std::endl;

	for (int i = 0; i < vec.size(); i++)
	{
		std::string instr_str = format_instr(vec[i]);
		std::string padding = std::string(32 - instr_str.size(), ' ');
		logger << instr_str << padding << "(" << i << ")" << "\n";
	}
	std::cout << std::endl;


	return EXIT_SUCCESS;
}