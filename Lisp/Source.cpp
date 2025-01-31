#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Tokenizer.h"
#include "Parser.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./lisp <source.lisp>";
		return EXIT_FAILURE;
	}

	std::stringstream src;
	std::ifstream file(argv[1]);
	
	if (!file || !file.is_open())
	{
		std::cerr << "Could not open file" << std::endl;
		return EXIT_FAILURE;
	}

	std::string line;
	while (std::getline(file, line))
	{
		src << line;
	}
	std::cout << "Source: " << src.str() << std::endl;

	file.close();

	Tokenizer tokenizer(src.str());
	std::vector<Token> tokens = tokenizer.tokenize();
	std::cout << "TOKENS:" << std::endl;
	for (int i = 0; i < tokens.size(); i++)
	{
		Token& token = tokens[i];
		std::cout << Tokenizer::tokentype_to_string(token.type);
		if (i != tokens.size() - 1)
		{
			std::cout << ", ";
		}
	}
	std::cout << std::endl;

	Parser parser(tokens);
	uint16_t return_val = parser.parse_prog();

	std::cout << "return value calculated: " << return_val << std::endl;
	return EXIT_SUCCESS;
}