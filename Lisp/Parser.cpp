#include "Parser.h"

Parser::Parser(std::vector<Token>& tokens) : m_tokens(tokens), m_index(0), m_stack() {}

void Parser::parse_loop()
{
	consume(3);
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
	{
		if (peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN) // optional assignment
		{
			consume(2);
			std::vector<Token> vars;
			while (peek().has_value() && !(peek().value() == TokenTypes::Symbol::CLOSE_PAREN))
			{
				if (peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
					peek(1).has_value() && (
						peek(1).value() == TokenTypes::Operator::MULT ||
						peek(1).value() == TokenTypes::Operator::DIV ||
						peek(1).value() == TokenTypes::Operator::ADD ||
						peek(1).value() == TokenTypes::Operator::SUB ||
						peek(1).value() == TokenTypes::Operator::GT ||
						peek(1).value() == TokenTypes::Operator::LT ||
						peek(1).value() == TokenTypes::Operator::BW_AND ||
						peek(1).value() == TokenTypes::Operator::BW_OR
						))
				{
					uint16_t ret_val = parse_expr();
					Token token{};
					token.type = TokenTypes::Literal::INT;
					token.value = std::to_string(ret_val);
					vars.push_back(token);

				}
				else
				{
					vars.push_back(peek().value());
					consume();
				}
			}
			consume(); // skip ')'
			if (vars.size() % 2 != 0)
			{
				std::cerr << "Variable-Value number mismatch" << std::endl;
				exit(EXIT_FAILURE);
			}
			for (int i = 0; i < vars.size() / 2; i++)
			{
				m_vars[vars[i].value.value()] = vars[i + vars.size() / 2];
			}
			// debug...
			std::cout << "[";
			for (int i = 0; i < vars.size(); i++)
			{
				std::cout << vars[i].value.value();
				if (i != vars.size() - 1)
				{
					std::cout << ", ";
				}
			}
			std::cout << "]" << std::endl;
			std::cout << "Value to convert: " << m_vars[vars[0].value.value()].value.value() << std::endl;
			std::cout << "after" << std::endl;
			// ...
		}
		parse_expr(); // condition
		if (peek().value() == TokenTypes::Symbol::OPEN_PAREN && // optional iter expr
			peek(1).has_value() && (
				peek(1).value() == TokenTypes::Operator::MULT ||
				peek(1).value() == TokenTypes::Operator::DIV ||
				peek(1).value() == TokenTypes::Operator::ADD ||
				peek(1).value() == TokenTypes::Operator::SUB ||
				peek(1).value() == TokenTypes::Operator::GT ||
				peek(1).value() == TokenTypes::Operator::LT ||
				peek(1).value() == TokenTypes::Operator::BW_AND ||
				peek(1).value() == TokenTypes::Operator::BW_OR
				))
		{
			parse_expr();
		}

	}
}

// @return evaluated expression
Parser::parse_expr()
{
	consume(); // skip '('
	std::cout << "ZA WARUDO" << std::endl;
	std::vector<uint16_t> vals{};
	TokenTypes::Operator op = TokenTypes::Operator::NONE;
	while (peek().has_value() && !(peek().value() == TokenTypes::Symbol::CLOSE_PAREN))
	{
		std::cout << m_tokens[m_index] <<  m_index << std::endl;
		if (peek().value() == TokenTypes::Symbol::OPEN_PAREN)
		{
			if (op != TokenTypes::Operator::NONE)
			{
				vals.push_back(parse_expr());
			}
			else
			{
				m_index += 2;
			}
		}
		else if (
			peek().value() == TokenTypes::Operator::MULT ||
			peek().value() == TokenTypes::Operator::DIV ||
			peek().value() == TokenTypes::Operator::ADD ||
			peek().value() == TokenTypes::Operator::SUB
		)
		{
			op = peek().value().extract<TokenTypes::Operator>();
			consume();
		}
		else if (peek().value() == TokenTypes::Operator::BW_AND)
		{
			consume();
			if (op == TokenTypes::Operator::BW_AND)
			{
				op = TokenTypes::Operator::AND;
			}
			else
			{
				op = TokenTypes::Operator::BW_AND;
			}
			consume();
		}
		else if (peek().value() == TokenTypes::Operator::BW_OR)
		{
			consume();
			if (op == TokenTypes::Operator::BW_OR)
			{
				op = TokenTypes::Operator::OR;
			}
			else
			{
				op = TokenTypes::Operator::BW_OR;
			}

			consume();
		}
		else if (
			peek().value() == TokenTypes::Operator::GT ||
			peek().value() == TokenTypes::Operator::LT
			)
		{
			op = peek().value().extract<TokenTypes::Operator>();
			if (peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN)
			{
				op = (peek().value() == TokenTypes::Operator::GT) ? TokenTypes::Operator::GTE : TokenTypes::Operator::LTE;
				consume();
			}
			consume();
		}
		else if (peek().value() == TokenTypes::Literal::INT || peek().value() == TokenTypes::Literal::ID)
		{
			if (peek().value() == TokenTypes::Literal::ID)
			{
				if (m_vars.find(peek().value().value.value()) == m_vars.end())
				{
					std::cerr << "Variable not found.." << std::endl;
					exit(EXIT_FAILURE);
				}
				vals.push_back(
					static_cast<uint16_t>(
						std::stoi(m_vars[peek().value().value.value()].value.value())
						)
				);
			}
			else
			{
				vals.push_back(static_cast<uint16_t>(std::stoi(peek().value().value.value())));
			}
			consume();
		}
	}
	consume(); // skip ')'
	return perform_op(op, vals);
}

uint16_t Parser::parse_prog()
{
	std::cout << "Parsing..." << std::endl;
	std::cout << m_tokens.size() << std::endl;
	while (peek().has_value())
	{
		std::cout << "[PARSER]" << "[" << m_index << "]" << " " <<
			"\'" << m_tokens[m_index] << "\'" << std::endl;
		if (peek().value() == TokenTypes::Symbol::OPEN_PAREN)
		{
			// std::cout << "hi ?" << std::endl;
			// std::cout << (peek(1).has_value() ? static_cast<uint16_t>(peek(1).value().type) : 0) << std::endl;
			if (peek(1).has_value() && (
				peek(1).value() == TokenTypes::Operator::MULT ||
				peek(1).value() == TokenTypes::Operator::DIV ||
				peek(1).value() == TokenTypes::Operator::ADD ||
				peek(1).value() == TokenTypes::Operator::SUB ||
				peek(1).value() == TokenTypes::Operator::GT ||
				peek(1).value() == TokenTypes::Operator::LT ||
				peek(1).value() == TokenTypes::Operator::BW_AND ||
				peek(1).value() == TokenTypes::Operator::BW_OR
				)
				)
			{
				uint16_t value = parse_expr();
				std::cout << "Value from expression: " << value << std::endl;
			}
			else if (peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN)
			{
				consume(2);
				std::vector<Token> vars;
				while (peek().has_value() && !(peek().value() == TokenTypes::Symbol::CLOSE_PAREN))
				{
					if (peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
						peek(1).has_value() && (
							peek(1).value() == TokenTypes::Operator::MULT ||
							peek(1).value() == TokenTypes::Operator::DIV ||
							peek(1).value() == TokenTypes::Operator::ADD ||
							peek(1).value() == TokenTypes::Operator::SUB ||
							peek(1).value() == TokenTypes::Operator::GT ||
							peek(1).value() == TokenTypes::Operator::LT ||
							peek(1).value() == TokenTypes::Operator::BW_AND ||
							peek(1).value() == TokenTypes::Operator::BW_OR
						))
					{
						uint16_t ret_val = parse_expr();
						Token token{};
						token.type = TokenTypes::Literal::INT;
						token.value = std::to_string(ret_val);
						vars.push_back(token);

					}
					else
					{
						vars.push_back(peek().value());
						consume();
					}
				}
				consume(); // skip ')'
				if (vars.size() % 2 != 0)
				{
					std::cerr << "Variable-Value number mismatch" << std::endl;
					exit(EXIT_FAILURE);
				}
				for (int i = 0; i < vars.size() / 2; i++)
				{
					m_vars[vars[i].value.value()] = vars[i + vars.size() / 2];
				}
				// debug...
				std::cout << "[";
				for (int i = 0; i < vars.size(); i++)
				{
					std::cout << vars[i].value.value();
					if (i != vars.size() - 1)
					{
						std::cout << ", ";
					}
				}
				std::cout << "]" << std::endl;
				std::cout << "Value to convert: " << m_vars[vars[0].value.value()].value.value() << std::endl;
				std::cout << "after" << std::endl;
				// ...
			}
			else if (peek(1).has_value() && peek(1).value() == TokenTypes::Statement::IF)
			{
				parse_if_stmt();
			}
		}
	}
	return EXIT_SUCCESS;
}

void Parser::parse_if_stmt()
{
	consume();
	if (peek().has_value() && peek().value() == TokenTypes::Statement::IF)
	{
		consume();
		if (uint16_t return_val = parse_expr()) // condition
		{
			std::cout << "reached: " << return_val << std::endl;
			parse_scope();
			consume(); // skip the if's ')'
			std::cout << "cleaning: " << peek().value() << "\n"
				<< peek(1).value() << std::endl;
			while (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
				peek(1).has_value() && peek(1).value() == TokenTypes::Statement::ELSE)
			{
				consume();
				int count = 1;
				while (peek().has_value() && count != 0)
				{
					if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
						count++;
					if (peek().has_value() && peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
						count--;
					consume();
				}
			}
			std::cout << "skipped to: " << m_index << std::endl;
		}
		else
		{
			if (!(peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN))
			{
				std::cerr << "fuck u" << std::endl;
				exit(EXIT_FAILURE);
			}
			consume();
			int count = 1;
			while (peek().has_value() && count != 0)
			{
				if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
					count++;
				if (peek().has_value() && peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
					count--;
				consume();
			}
			consume(); // the if's ')'
			parse_if_stmt();
		}
	}
	else if (peek().has_value() && peek().value() == TokenTypes::Statement::ELSE)
	{
		if (peek(1).has_value() && peek(1).value() == TokenTypes::Statement::IF)
		{
			consume();
			consume();
			if (uint16_t return_val = parse_expr()) // condition
			{
				std::cout << "reached: " << return_val << std::endl;
				parse_scope();
				consume(); // skip the if's ')'
				while (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
					peek(1).has_value() && peek(1).value() == TokenTypes::Statement::ELSE)
				{
					consume();
					int count = 1;
					while (peek().has_value() && count != 0)
					{
						if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
							count++;
						if (peek().has_value() && peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
							count--;
						consume();
					}
				}
			}
			else
			{
				if (!(peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN))
				{
					std::cerr << "fuck u" << std::endl;
					exit(EXIT_FAILURE);
				}
				consume();
				int count = 1;
				while (peek().has_value() && count != 0)
				{
					if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
						count++;
					if (peek().has_value() && peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
						count--;
					consume();
				}
				consume(); // the if's ')'
				parse_if_stmt();
			}
		}
		else
		{
			consume();
			parse_scope();
			consume();
		}
	}
}

void Parser::parse_scope()
{
	if (!(peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN))
	{
		std::cerr << "fuck u, this is a scope" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "index we starting with: " << m_index << std::endl;
	std::cout << "element we starting with: " << m_tokens[m_index] << std::endl;

	consume();
	int count = 1;
	while (peek().has_value() && count != 0)
	{
		std::cout << m_index << std::endl;
		if (peek().value() == TokenTypes::Symbol::OPEN_PAREN)
		{
			if (peek(1).has_value() && (
				peek(1).value() == TokenTypes::Operator::MULT ||
				peek(1).value() == TokenTypes::Operator::DIV ||
				peek(1).value() == TokenTypes::Operator::ADD ||
				peek(1).value() == TokenTypes::Operator::SUB ||
				peek(1).value() == TokenTypes::Operator::GT ||
				peek(1).value() == TokenTypes::Operator::LT ||
				peek(1).value() == TokenTypes::Operator::BW_AND ||
				peek(1).value() == TokenTypes::Operator::BW_OR
				)
				)
			{
				uint16_t value = parse_expr();
				std::cout << "Value from expression: " << value << std::endl;
			}
			else if (peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN)
			{
				m_index += 2;
				std::vector<Token> vars;
				while (peek().has_value() && !(peek().value() == TokenTypes::Symbol::CLOSE_PAREN))
				{
					vars.push_back(peek().value());
					consume();
				}
				consume(); // skip ')'
				if (vars.size() % 2 != 0)
				{
					std::cerr << "Variable-Value number mismatch" << std::endl;
					exit(EXIT_FAILURE);
				}
				for (int i = 0; i < vars.size() / 2; i++)
				{
					m_vars[vars[i].value.value()] = vars[i + vars.size() / 2];
				}
				// debug...
				std::cout << "[";
				for (int i = 0; i < vars.size(); i++)
				{
					std::cout << vars[i].value.value();
					if (i != vars.size() - 1)
					{
						std::cout << ", ";
					}
				}
				std::cout << "]" << std::endl;
				std::cout << "Value to convert: " << m_vars[vars[0].value.value()].value.value() << std::endl;
				std::cout << "after" << std::endl;
				// ...
			}
			else if (peek(1).has_value() && peek(1).value() == TokenTypes::Statement::IF)
			{
				consume();
				consume();
				if (uint16_t return_val = parse_expr()) // condition
				{
					std::cout << "reached: " << return_val << std::endl;
					parse_scope();
				}
				else
				{
					if (!(peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN))
					{
						std::cerr << "fuck u" << std::endl;
						exit(EXIT_FAILURE);
					}
					consume();
					int count = 1;
					while (peek().has_value() && count != 0)
					{
						if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
							count++;
						if (peek().has_value() && peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
							count--;
						consume();
					}
				}
				consume();
			}
			else
			{
				consume();
				count++;
			}
		}
		else if (peek().value() == TokenTypes::Symbol::CLOSE_PAREN)
		{
			consume();
			count--;
		}
	}
	std::cout << "index we leaving with: " << m_index << std::endl;
	std::cout << "element we leaving with: " << m_tokens[m_index] << std::endl;
}

// @returns result of a certain operation between number of elements 
// e.g: (+, [1, 2, 3]) -> 1 + 2 + 3
uint16_t Parser::perform_op(TokenTypes::Operator op, std::vector<uint16_t>& vals)
{
	// debug...
	std::cout << "[";
	for (int i = 0; i < vals.size(); i++)
	{
		std::cout << vals[i];
		if (i != vals.size() - 1)
		{
			std::cout << ", ";
		}
	}
	std::cout << "]" << std::endl;
	// ...
	switch (op)
	{
		case TokenTypes::Operator::MULT:
			return std::accumulate(vals.begin(), vals.end(), 1, [](uint16_t a, uint16_t b) { return a * b; });
		case TokenTypes::Operator::DIV:
			return std::accumulate(vals.begin() + 1, vals.end(), vals.front(), [](uint16_t a, uint16_t b) { return a / b; });
		case TokenTypes::Operator::ADD:
			return std::accumulate(vals.begin(), vals.end(), 0, [](uint16_t a, uint16_t b) { return a + b; });
		case TokenTypes::Operator::SUB:
			return std::accumulate(vals.begin() + 1, vals.end(), vals.front(), [](uint16_t a, uint16_t b) { return a - b; });
		case TokenTypes::Operator::BW_AND:
			return std::accumulate(vals.begin(), vals.end(), 0, [](uint16_t a, uint16_t b) { return a & b; });
		case TokenTypes::Operator::AND:
			return std::accumulate(vals.begin() + 1, vals.end(), vals.front(), [](uint16_t a, uint16_t b) { return a && b; });
		case TokenTypes::Operator::BW_OR:
			return std::accumulate(vals.begin(), vals.end(), 0, [](uint16_t a, uint16_t b) { return a | b; });
		case TokenTypes::Operator::OR:
			return std::accumulate(vals.begin() + 1, vals.end(), vals.front(), [](uint16_t a, uint16_t b) { return a || b; });
		case TokenTypes::Operator::LT:
			assert(vals.size() == 2);
			return static_cast<uint16_t>(vals[0] < vals[1]);
		case TokenTypes::Operator::GT:
			assert(vals.size() == 2);
			return static_cast<uint16_t>(vals[0] > vals[1]);
		case TokenTypes::Operator::LTE:
			assert(vals.size() == 2);
			return static_cast<uint16_t>(vals[0] <= vals[1]);
		case TokenTypes::Operator::GTE:
			assert(vals.size() == 2);
			return static_cast<uint16_t>(vals[0] >= vals[1]);
		default:
			std::cerr << "i fr: " << m_index << std::endl;
			// std::cerr << "Not a valid operator: " << op << std::endl;
			exit(EXIT_FAILURE);
	}
}

std::optional<Token> Parser::peek(int offset)
{
	if (m_index + offset < m_tokens.size())
		return m_tokens[m_index + offset];
	return {};
}

void Parser::consume(int amount)
{
	if (!peek(amount - 1).has_value())
	{
		std::cerr << "Consume out of range" << std::endl;
		exit(EXIT_FAILURE);
	}
	m_index += amount;
}