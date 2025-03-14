#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <functional>
#include <optional>
#include <cassert>
#include <variant>

#include <type_traits>
#include <typeinfo>
#include <tuple>

#include "Tokenizer.h"

namespace Node
{
	struct LitInt
	{
		int val;
	};

	struct LitIdent
	{
		std::string id;
	};

	struct Lit
	{
		std::variant<LitInt, LitIdent> lit;
	};

	struct Expr;

	struct BinExpr
	{
		std::shared_ptr<Expr> lhs = nullptr;
		std::shared_ptr<Expr> rhs = nullptr;
		TokenTypes::Operator op = TokenTypes::Operator::NONE;
	};

	struct Expr
	{
		std::variant<Lit, BinExpr> expr;
	};

	struct StmtAsgn
	{
		LitIdent id;
		Expr expr;
	};

	struct Scope;

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

	struct Stmt
	{
		std::variant<StmtAsgn, StmtIf, StmtLoop, Expr> stmt;
	};

	struct Scope
	{
		std::vector<Stmt> stmts;
	};
}

class Parser
{
public:
	Parser(std::vector<Token>& tokens);
	std::vector<Node::Stmt> parse_prog();
	std::optional<Node::Expr> parse_expr();
	std::optional<Node::Stmt> parse_stmt();
	std::optional<Node::Scope> parse_scope();
	std::optional<Node::StmtIf> parse_if_chain();
	std::optional<Node::StmtAsgn> parse_asgn_stmt();
	std::optional<Node::StmtIf> parse_if_stmt();
	std::optional<Node::StmtLoop> parse_loop_stmt();

	static std::string node_to_string(const Node::Stmt& node, const size_t depth = 0)
	{
		return std::visit([depth](auto&& stmt) -> std::string {
			using T = std::decay_t<decltype(stmt)>;
			std::stringstream res{};
			std::string tab = std::string(depth * 2, ' ');
			std::cout << tab.size() << std::endl;
			res << tab << typeid(stmt).name() << " {\n";

			if constexpr (std::is_same_v<T, Node::Expr>)
			{
				res << std::visit([depth](auto&& expr) -> std::string {
					using U = std::decay_t<decltype(expr)>;
					std::stringstream temp{};
					std::string inner_tab = std::string((depth + 1) * 2, ' ');
					std::cout << inner_tab.size() << std::endl;

					

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
						temp << node_to_string(Node::Stmt{*expr.lhs}, depth + 2) + " " +
							Tokenizer::tokentype_to_string(expr.op) + " \n" +
							node_to_string(Node::Stmt{ *expr.rhs }, depth + 2);
					}
					temp << inner_tab << "}\n";
					return temp.str();
				}, stmt.expr);
			}
			else if constexpr (std::is_same_v<T, Node::StmtAsgn>)
			{
				res << typeid(stmt.id).name() << std::endl;
				res << node_to_string(Node::Stmt{ stmt.expr }, depth + 1);
			}
			else if constexpr (std::is_same_v<T, Node::StmtIf>)
			{
				res << node_to_string(Node::Stmt{ stmt.cond }, depth + 1) << std::endl;
				res << "[";
				for (const Node::Stmt& stmt : stmt.scope->stmts)
				{
					res << node_to_string(stmt);
					res << ", ";
				}
				res << "]" << std::endl;
				if (stmt.elif.has_value())
				{
					res << node_to_string(Node::Stmt{ *stmt.elif.value() }) << std::endl;;
				}
			}
			else if constexpr (std::is_same_v<T, Node::StmtLoop>)
			{
				if (stmt.init.has_value())
				{
					res << node_to_string(Node::Stmt{ stmt.init.value() }) << std::endl;;
				}
				res << node_to_string(Node::Stmt{ stmt.cond }) << std::endl;
				if (stmt.adv.has_value())
				{
					res << node_to_string(Node::Stmt{ stmt.adv.value() }) << std::endl;;
				}
				res << "[";
				for (const Node::Stmt& stmt : stmt.scope->stmts)
				{
					res << node_to_string(stmt);
					res << ", ";
				}
				res << "]" << std::endl;
			}

			res << tab << "}\n";
			return res.str();
		}, node.stmt);
	}


private:
	std::optional<Token> peek(int offset = 0);
	Token consume(unsigned int amount = 1);

	template<typename T>
		requires requires { typename T::value_type; }
	auto strict(const T& opt) -> typename T::value_type
	{
		if (opt.has_value())
			return opt.value();
		err_exit("Expected " + typeid(T::value_type).name());
	}

private:
	const std::vector<Token> m_tokens;
	uint16_t m_index;
};

