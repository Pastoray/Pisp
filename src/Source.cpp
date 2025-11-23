#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Tokenizer.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
#include "Utils.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
		ERR_EXIT("Usage: ./lisp <source.lisp>");

	std::stringstream src;
	std::ifstream file(argv[1]);
	
	if (!file || !file.is_open())
		ERR_EXIT("Could not open file: ", argv[1]);

	std::string line;
	while (std::getline(file, line))
	{
		src << line;
	}

	LOGGER << "Source: " << src.str() << std::endl;

	file.close();

	Tokenizer tokenizer(src.str());
	std::vector<Token> tokens = tokenizer.tokenize();
	LOGGER << "TOKENS:" << std::endl;
	for (int i = 0; i < tokens.size(); i++) // debug: tokens
	{
		Token& token = tokens[i];
		LOGGER << Tokenizer::tokentype_to_string(token.type);
		if (i != tokens.size() - 1)
		{
			LOGGER << ", ";
		}
	}
	LOGGER << std::endl;
	LOGGER << "Parsing..." << std::endl;

	Parser parser(tokens);
	std::vector<Node::Node> nodes = parser.parse_prog();

	LOGGER << "Statements :" << std::endl;

	for (const Node::Node& node : nodes) // debug: nodes
	{
		LOGGER << Parser::node_to_string(node) << std::endl;
	}

	LOGGER << "Finished Statements" << std::endl;

	LOGGER << "Parsing completed" << std::endl;

	LOGGER << "Compiling..." << std::endl;

	Compiler compiler(nodes);
	auto vec = compiler.compile_prog();

	LOGGER << "Compilation completed\n" << std::endl;

	for (int i = 0; i < vec.size(); i++) // debug: bytecode
	{
		std::string instr_str = format_instr(vec[i]);
		std::string padding = std::string(32 - instr_str.size(), ' ');
		LOGGER << instr_str << padding << "(" << i << ")" << "\n";
	}
	LOGGER << std::endl;

	VM vm(vec);
	vm.run();

	return EXIT_SUCCESS;
}