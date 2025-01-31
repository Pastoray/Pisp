#include "Tokenizer.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Token& token)
{
	os << Tokenizer::tokentype_to_string(token.type);
	return os;
}

Tokenizer::Tokenizer(const std::string& src) : m_src(src), m_index(0) {}

std::vector<Token> Tokenizer::tokenize()
{
	std::vector<Token> tokens;
	std::string buffer;
	while (peek().has_value())
	{
		// std::cout << "[TOKENIZER]" << "[" << m_index << "]" << " " << "\'" << m_src[m_index] << "\'" << std::endl;
		if (std::isdigit(peek().value()))
		{
			while (peek().has_value() && std::isdigit(peek().value()))
			{
				std::cout << "digit index: " << m_index << std::endl;
				buffer += m_src[m_index];
				m_index++;
			}
			Token token{};
			token.type = TokenTypes::Literal::INT;
			token.value = buffer;
			tokens.push_back(token);

			buffer.clear();
		}
		else if (std::isalpha(peek().value()))
		{
			buffer += peek().value();
			m_index++;
			while (peek().has_value() && std::isalnum(peek().value()))
			{
				buffer += m_src[m_index];
				m_index++;
			}
			Token token{};
			token.type = TokenTypes::Literal::ID;
			token.value = buffer;
			tokens.push_back(token);
			buffer.clear();
		}
		else if (peek().value() == '=')
		{
			Token token{};
			token.type = TokenTypes::Operator::ASGN;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '(')
		{
			Token token{};
			token.type = TokenTypes::Symbol::OPEN_PAREN;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == ')')
		{
			Token token{};
			token.type = TokenTypes::Symbol::CLOSE_PAREN;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '+')
		{
			Token token{};
			token.type = TokenTypes::Operator::ADD;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '-')
		{
			Token token{};
			token.type = TokenTypes::Operator::SUB;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '*')
		{
			Token token{};
			token.type = TokenTypes::Operator::MULT;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '/')
		{
			Token token{};
			token.type = TokenTypes::Operator::DIV;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '>')
		{
			Token token{};
			token.type = TokenTypes::Operator::GT;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '<')
		{
			Token token{};
			token.type = TokenTypes::Operator::LT;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '&')
		{
			Token token{};
			token.type = TokenTypes::Operator::BW_AND;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '|')
		{
			Token token{};
			token.type = TokenTypes::Operator::BW_OR;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '?')
		{
			Token token{};
			token.type = TokenTypes::Statement::IF;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == '!')
		{
			Token token{};
			token.type = TokenTypes::Statement::ELSE;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == ':')
		{
			Token token{};
			token.type = TokenTypes::Statement::LOOP;
			tokens.push_back(token);
			m_index++;
		}
		else if (peek().value() == ' ' || peek().value() == '\n' || peek().value() == '\t')
		{
			m_index++;
		}

	}
	return tokens;
}

std::optional<char> Tokenizer::peek(int offset)
{
	if (m_index + offset < m_src.size())
		return m_src[m_index + offset];
	return {};
}

std::string Tokenizer::tokentype_to_string(TokenType type)
{
	return std::visit([](auto&& arg) -> std::string {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same<T, TokenTypes::Symbol>::value)
		{
			switch (arg)
			{
				case TokenTypes::Symbol::OPEN_PAREN:
					return "(";
				case TokenTypes::Symbol::CLOSE_PAREN:
					return ")";
				case TokenTypes::Symbol::NONE:
					return "symbol none";
			}

		}
		else if constexpr (std::is_same<T, TokenTypes::Literal>::value)
		{
			switch (arg)
			{
				case TokenTypes::Literal::INT:
					return "int";
				case TokenTypes::Literal::ID:
					return "identifier";
				case TokenTypes::Literal::NONE:
					return "literal none";
			}
		}
		else if constexpr (std::is_same<T, TokenTypes::Operator>::value)
		{
			switch (arg)
			{
				case TokenTypes::Operator::MULT:
					return "*";
				case TokenTypes::Operator::DIV:
					return "/";
				case TokenTypes::Operator::ADD:
					return "+";
				case TokenTypes::Operator::SUB:
					return "-";
				case TokenTypes::Operator::ASGN:
					return "=";
				case TokenTypes::Operator::GT:
					return ">";
				case TokenTypes::Operator::LT:
					return "<";
				case TokenTypes::Operator::BW_AND:
					return "&";
				case TokenTypes::Operator::BW_OR:
					return "|";
				case TokenTypes::Operator::NONE:
					return "operator none";
			}
		}
		else if constexpr (std::is_same<T, TokenTypes::Statement>::value)
		{
			switch (arg)
			{
				case TokenTypes::Statement::IF:
					return "if";
				case TokenTypes::Statement::ELSE:
					return "else";
				case TokenTypes::Statement::LOOP:
					return "loop";
				case TokenTypes::Statement::NONE:
					return "statement none";
			}
		}
		return "Unknown";
		}, type
	);
}
