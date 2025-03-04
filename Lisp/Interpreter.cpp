#include "Interpreter.h"
#include "Logger.h"


Interpreter::Interpreter(std::vector<Node::Stmt>& stmts) : m_nodes(std::move(stmts)), m_index(0) {}

void Interpreter::interpret_prog()
{
	while (peek().has_value())
	{
		interpret_stmt(peek().value());
		consume();
	}
	// debug...
	LOG_DEBUG("<m_vars>" << std::endl);
	for (const auto& [key, val] : m_vars)
	{
		LOG_DEBUG("  key: " << key << ", val: " << val << "\n");
	}
	LOG_DEBUG("</m_vars>" << std::endl);
	// ...
}

void Interpreter::interpret_stmt(const Node::Stmt& node)
{
	std::visit([this](auto&& stmt) -> void {
		using T = std::decay_t<decltype(stmt)>;
		if constexpr (std::is_same_v<T, Node::StmtAsgn>)
			interpret_asgn(stmt);

		else if constexpr (std::is_same_v<T, Node::StmtIf>)
			interpret_if(stmt);

		else if constexpr (std::is_same_v<T, Node::StmtLoop>)
			interpret_loop(stmt);

		else if constexpr (std::is_same_v<T, Node::Expr>)
			interpret_expr(stmt);

		}, node.stmt);
}

void Interpreter::interpret_asgn(const Node::StmtAsgn& node)
{
	auto value = interpret_expr(node.expr);
	m_vars[std::string(node.id.id)] = value;
}

void Interpreter::interpret_if(const Node::StmtIf& node)
{
	auto cond = interpret_expr(node.cond);
	if (cond != 0)
	{
		interpret_scope(*node.scope);
	}
	else if(node.elif.has_value())
	{
		interpret_if(*node.elif.value());
	}
}

void Interpreter::interpret_loop(const Node::StmtLoop& node)
{
	if (node.init.has_value())
	{
		interpret_asgn(node.init.value());
	}
	for (node.init.has_value() && (interpret_asgn(node.init.value()), true);
		interpret_expr(node.cond);
		node.adv.has_value() && (interpret_asgn(node.adv.value()), true))
	{
		interpret_scope(*node.scope);
	}
}

uint32_t Interpreter::interpret_expr(const Node::Expr& node)
{
	auto res = std::visit([this](auto&& expr) -> uint32_t {
		using T = std::decay_t<decltype(expr)>;
		if constexpr (std::is_same_v<T, Node::Lit>)
		{
			return std::visit([this](auto&& lit) -> uint32_t {
				using T = std::decay_t<decltype(lit)>;
				if constexpr (std::is_same_v<T, Node::LitInt>)
				{
					return lit.val;
				}
				else if constexpr (std::is_same_v<T, Node::LitIdent>)
				{
					return m_vars.at(lit.id);
				}
				}, expr.lit);
		}
		else if constexpr (std::is_same_v<T, Node::BinExpr>)
		{
			return perform_op(interpret_expr(*expr.lhs), interpret_expr(*expr.rhs), expr.op);
		}
		}, node.expr);
	return res;
}

void Interpreter::interpret_scope(const Node::Scope& node)
{
	for (const Node::Stmt& stmt : node.stmts)
		interpret_stmt(stmt);
}

[[nodiscard]] std::optional<Node::Stmt> Interpreter::peek(int offset)
{
	if (m_index + offset < m_nodes.size())
		return m_nodes[m_index + offset];
	return {};
}

Node::Stmt Interpreter::consume(unsigned int amount)
{
	if (amount == 0) [[unlikely]]
	{
		err_exit("Consume called with a value of 0");
	}
	m_index += amount;
	return peek(-1).value(); // return last consumed token
}

void Interpreter::err_exit(const std::string& msg) const
{
	std::cerr << "[ERROR]" << "[INDEX:" << m_index << "]" << " -> " << msg << std::endl;
	exit(EXIT_FAILURE);
}

uint32_t Interpreter::perform_op(uint32_t v1, uint32_t v2, TokenTypes::Operator op) const
{
	switch (op)
	{
		case TokenTypes::Operator::MULT:
			return v1 * v2;
		case TokenTypes::Operator::DIV:
			return v1 / v2;
		case TokenTypes::Operator::ADD:
			return v1 + v2;
		case TokenTypes::Operator::SUB:
			return v1 - v2;
		case TokenTypes::Operator::BW_AND:
			return v1 & v2;
		case TokenTypes::Operator::BW_OR:
			return v1 | v2;
		case TokenTypes::Operator::AND:
			return static_cast<uint32_t>(v1 && v2); // remove warning
		case TokenTypes::Operator::OR:
			return static_cast<uint32_t>(v1 || v2); // remove warning
		case TokenTypes::Operator::LT:
			return v1 < v2;
		case TokenTypes::Operator::GT:
			return v1 > v2;
		case TokenTypes::Operator::LTE:
			return v1 <= v2;
		case TokenTypes::Operator::GTE:
			return v1 >= v2;
		default:
			err_exit("Not a valid operator");
	}
}


