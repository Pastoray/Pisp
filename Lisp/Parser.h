#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <functional>
#include <unordered_map>
#include <optional>
#include <cassert>
#include <variant>
#include "Tokenizer.h"

class Parser
{
public:
	Parser(std::vector<Token>& tokens);
	uint16_t parse_prog();
	uint16_t parse_expr();
	void parse_if_stmt();
	void parse_scope();
	void parse_loop();

private:
	std::optional<Token> peek(int offset = 0);
	void consume(int amount = 1);
	uint16_t perform_op(TokenTypes::Operator op, std::vector<uint16_t>& vals);

private:
	std::vector<Token>& m_tokens;
	std::vector<uint16_t> m_stack;
	std::unordered_map<std::string, Token> m_vars;
	uint16_t m_index;

};

