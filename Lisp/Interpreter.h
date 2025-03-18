#pragma once

#include "Parser.h"
#include "Tokenizer.h"
#include "Utils.h"

class Interpreter
{
public:
	Interpreter(std::vector<Node::Node>& stmts);
	void interpret_prog();
	std::optional<Node::LitInt> interpret_node(const Node::Node& node);
	void interpret_asgn(const Node::StmtAsgn& node);
	std::optional<Node::LitInt> interpret_if(const Node::StmtIf& node);
	std::optional<Node::LitInt> interpret_loop(const Node::StmtLoop& node);
	std::optional<Node::LitInt> interpret_ret(const Node::StmtRet& node);
	std::optional<Node::LitInt> interpret_stmt(const Node::Stmt& node);
	std::optional<Node::LitInt> interpret_scope(const Node::Scope& node);

	void interpret_struct(const Node::Struct& node);
	void interpret_func_decl(const Node::StructFuncDecl& node);

	Node::LitInt interpret_expr(const Node::Expr& node);
	std::optional<Node::LitInt> interpret_call(const Node::NodeCall& node);

private:
	struct LitIdentHash
	{
		size_t operator()(const Node::LitIdent& lit) const
		{
			return std::hash<std::string>()(lit.id);
		}
	};

	const std::vector<Node::Node> m_nodes;
	std::unordered_map<Node::LitIdent, std::variant<Node::LitInt, Node::Struct>, LitIdentHash> m_vars;
	uint16_t m_index;

private:
	std::optional<Node::Node> peek(int offset = 0);
	Node::Node consume(unsigned int amount = 1);
	Node::LitInt perform_op(const Node::LitInt& v1, const Node::LitInt& v2, TokenTypes::Operator op) const;

};
