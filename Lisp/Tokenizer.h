#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <optional>
#include <variant>
#include <iostream>

#include "Utils.h"

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
		RET, // return
		CALL, // function call
		NONE, // temporary place holder
	};

	enum class Struct
	{
		FUNC, // function
	};
}

using TokenType = std::variant<TokenTypes::Symbol, TokenTypes::Operator, TokenTypes::Literal,
	TokenTypes::Statement, TokenTypes::Struct>;

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
	char consume(unsigned int amount = 1);

private:
	const std::string m_src;
	uint16_t m_index;
};

