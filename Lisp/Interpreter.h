#pragma once

#include "Parser.h"
#include "Tokenizer.h"

class Interpreter
{
public:
	Interpreter(std::vector<Node::Stmt>& stmts);
	void interpret_prog();
	void interpret_asgn(const Node::StmtAsgn& node);
	void interpret_if(const Node::StmtIf& node);
	void interpret_loop(const Node::StmtLoop& node);
	void interpret_stmt(const Node::Stmt& node);
	void interpret_scope(const Node::Scope& node);
	uint32_t interpret_expr(const Node::Expr& node);

private:
	const std::vector<Node::Stmt> m_nodes;
	std::unordered_map<std::string, uint32_t> m_vars;
	uint16_t m_index;

private:
	std::optional<Node::Stmt> peek(int offset = 0);
	Node::Stmt consume(unsigned int amount = 1);
	void err_exit(const std::string& msg) const;
	uint32_t perform_op(uint32_t v1, uint32_t v2, TokenTypes::Operator op) const;

};
