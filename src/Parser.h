#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <functional>
#include <optional>
#include <cassert>
#include <variant>
#include <memory>

#include <type_traits>
#include <typeinfo>
#include <tuple>

#include "Tokenizer.h"
#include "Utils.h"

namespace Node
{
	struct LitInt
	{
		int val;
	};

	struct LitIdent
	{
		std::string id;
		bool operator==(const LitIdent& other) const
		{
			return id == other.id;
		}
	};

	struct Lit
	{
		std::variant<LitInt, LitIdent> lit;
	};

	struct Expr;

	struct BinExpr
	{
		std::optional<std::shared_ptr<Expr>> lhs;
		std::optional<std::shared_ptr<Expr>> rhs;
		std::optional<TokenTypes::Operator> op;
	};

	struct Stmt;

	struct Scope
	{
		std::vector<Stmt> stmts;
	};

	struct StructFuncDecl
	{
		std::vector<LitIdent> params;
		Scope scope;
	};

	struct Struct
	{
		std::variant<StructFuncDecl> strct;
	};


	struct Call
	{
		std::variant<LitIdent, StructFuncDecl> fn;
		std::vector<Expr> args;
	};

	struct Expr
	{
		std::variant<BinExpr, Lit, Call> expr;
	};

	struct StmtAsgn
	{
		LitIdent id;
		std::variant<Expr, Struct> val;
	};

	struct StmtIf
	{
		Expr cond; // else will just be and elif with cond set to True
		std::shared_ptr<Scope> scope;
		std::optional<std::shared_ptr<StmtIf>> elif{};
	};

	struct StmtLoop
	{
		std::optional<StmtAsgn> init{};
		Expr cond;
		std::optional<StmtAsgn> adv{};
		std::shared_ptr<Scope> scope;
	};

	struct StmtRet
	{
		std::optional<Expr> ret_val;
	};

	struct Stmt
	{
		std::variant<StmtAsgn, StmtIf, StmtLoop, StmtRet> stmt;
	};

	struct Node
	{
		std::variant<Expr, Stmt, Struct, Scope> node;
	};
}

class Parser
{
public:
	Parser(std::vector<Token>& tokens);
	std::vector<Node::Node> parse_prog();
	std::optional<Node::Node> parse_node();
	std::optional<Node::Expr> parse_expr();
	std::optional<Node::Stmt> parse_stmt();
	std::optional<Node::Scope> parse_scope();
	std::optional<Node::Struct> parse_struct();
	std::optional<Node::Lit> parse_lit();

	std::optional<Node::StructFuncDecl> parse_func_decl();

	std::optional<Node::StmtIf> parse_if_chain();
	std::optional<Node::StmtAsgn> parse_asgn_stmt();
	std::optional<Node::StmtIf> parse_if_stmt();
	std::optional<Node::StmtLoop> parse_loop_stmt();
	std::optional<Node::Call> parse_call();
	std::optional<Node::StmtRet> parse_ret_stmt();
	

	static std::string node_to_string(const Node::Node& node, const size_t depth = 0)
	{
		return std::visit([depth](auto&& stmt) -> std::string {
			using T = std::decay_t<decltype(stmt)>;
			std::stringstream res{};
			std::string tab = std::string(depth * 2, ' ');
			LOGGER << tab.size() << std::endl;
			res << tab << typeid(stmt).name() << " {\n";

			if constexpr (std::is_same_v<T, Node::Expr>)
			{
				res << std::visit([depth](auto&& expr) -> std::string {
					using U = std::decay_t<decltype(expr)>;
					std::stringstream temp{};
					std::string inner_tab = std::string((depth + 1) * 2, ' ');
					LOGGER << inner_tab.size() << std::endl;

					

					if constexpr (std::is_same_v<U, Node::Lit>)
					{
						temp << inner_tab << typeid(expr).name() << " {\n";
						temp << std::visit([depth](auto&& lit) -> std::string {
							std::string inner_tab = std::string((depth + 2) * 2, ' ');
							std::stringstream ss;
							ss << inner_tab << typeid(lit).name() << "\n";
							return ss.str();
							}, expr.lit);
					}
					else if constexpr (std::is_same_v<U, Node::BinExpr>)
					{
						temp << inner_tab << typeid(expr).name() << " {\n";
						temp << node_to_string(Node::Node{ *expr.lhs.value() }, depth + 2) + " " +
							Tokenizer::tokentype_to_string(expr.op.value_or(TokenTypes::Operator::NONE)) + " \n" +
							node_to_string(Node::Node{ *expr.rhs.value() }, depth + 2);
					}
					temp << inner_tab << "}\n";
					return temp.str();
				}, stmt.expr);
			}
			else if constexpr (std::is_same_v<T, Node::StmtAsgn>)
			{
				res << typeid(stmt.id).name() << std::endl;
				res << node_to_string(Node::Node{ Node::Stmt{ stmt.expr }}, depth + 1);
			}
			else if constexpr (std::is_same_v<T, Node::StmtIf>)
			{
				res << node_to_string(Node::Node{ Node::Stmt{ stmt.cond }}, depth + 1) << std::endl;
				res << "[";
				for (const Node::Stmt& stmt : stmt.scope->stmts)
				{
					res << node_to_string(Node::Node{ stmt });
					res << ", ";
				}
				res << "]" << std::endl;
				if (stmt.elif.has_value())
				{
					res << node_to_string(Node::Node{ Node::Stmt{ *stmt.elif.value() }}) << std::endl;
				}
			}
			else if constexpr (std::is_same_v<T, Node::StmtLoop>)
			{
				if (stmt.init.has_value())
				{
					res << node_to_string(Node::Node{ Node::Stmt{ stmt.init.value() }}) << std::endl;;
				}
				res << node_to_string(Node::Node{ Node::Stmt{ stmt.cond }}) << std::endl;
				if (stmt.adv.has_value())
				{
					res << node_to_string(Node::Node{ Node::Stmt{ stmt.adv.value() }}) << std::endl;;
				}
				res << "[";
				for (const Node::Stmt& stmt : stmt.scope->stmts)
				{
					res << node_to_string(Node::Node{ stmt });
					res << ", ";
				}
				res << "]" << std::endl;
			}

			res << tab << "}\n";
			return res.str();
		}, node.node);
	}


private:
	[[nodiscard]] std::optional<Token> peek(int offset = 0);
	Token consume(unsigned int amount = 1);

	template<typename T>
		requires requires { typename T::value_type; }
	auto strict(const T& opt) -> typename T::value_type
	{
		if (opt.has_value())
			return opt.value();
		ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Expected ", std::string(typeid(typename T::value_type).name()));
	}

private:
	const std::vector<Token> m_tokens;
	uint16_t m_index;
};

