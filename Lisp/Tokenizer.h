#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <optional>
#include <variant>
#include <iostream>

namespace TokenTypes
{
	enum class Symbol
	{
		OPEN_PAREN, // (
		CLOSE_PAREN, // )
		NONE, // temporary place holder

	};

	enum class Operator
	{
		ASGN, // =
		MULT, // *
		DIV, // /
		ADD, // +
		SUB, // -
		GT, // >
		LT, // <
		GTE, // >=
		LTE, // <=
		AND, // &
		OR, // |
		BW_AND, // bitwise &
		BW_OR, // bitwise |
		RET, // return
		NONE, // temporary place holder
	};

	enum class Literal
	{
		INT, // 123
		IDENT, // x
		NONE, // temporary place holder
	};

	enum class Statement
	{
		IF, // if
		ELSE, // else
		LOOP, // for
		FUNC, // function
		NONE, // temporary place holder
	};
}

using TokenType = std::variant<TokenTypes::Symbol, TokenTypes::Operator, TokenTypes::Literal, TokenTypes::Statement>;

struct Token
{
	TokenType type;
	std::optional<std::string> value{};

	template<typename T>
	bool operator==(const T& type_enum)
	{
		if (auto* op = std::get_if<T>(&type))
			return *op == type_enum;
		return false;
	}
	template<typename T>
	T extract()
	{
		if (auto* op = std::get_if<T>(&type))
		{
			return *op;
		}
		// compare before this
		std::cerr << "Extraction failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

class Tokenizer
{
public:
	Tokenizer(const std::string& src);
	std::vector<Token> tokenize();
	static std::string tokentype_to_string(TokenType type);

private:
	std::optional<char> peek(int offset = 0);
	char consume(unsigned int amount);
	void err_exit(const std::string& msg);

private:
	const std::string m_src;
	uint16_t m_index;
};

