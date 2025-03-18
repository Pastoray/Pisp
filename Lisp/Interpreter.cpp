#include "Interpreter.h"

Interpreter::Interpreter(std::vector<Node::Node>& stmts) : m_nodes(std::move(stmts)), m_index(0) {}

void Interpreter::interpret_prog()
{
	while (peek().has_value())
	{
		interpret_node(peek().value());
		consume();
	}
	// debug...
	logger << "<m_vars>" << std::endl;
	for (const auto& [key, val] : m_vars)
	{
		
		std::visit([key](auto&& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (std::is_same_v<T, Node::LitInt>)
				logger << "  key: " << key.id << ", val: " << val.val << "\n";

			else if constexpr (std::is_same_v<T, Node::Struct>)
				logger << "  key: " << key.id << ", val: " << "function" << "\n";
			}
		, val);

	}
	logger << "</m_vars>" << std::endl;
	// ...
}

std::optional<Node::LitInt> Interpreter::interpret_node(const Node::Node& node)
{
	return std::visit([this](auto&& node) -> std::optional<Node::LitInt> {
		using T = std::decay_t<decltype(node)>;
		if constexpr (std::is_same_v<T, Node::Expr>)
			interpret_expr(node);

		if constexpr (std::is_same_v<T, Node::Stmt>)
		{
			if (auto ret_val = interpret_stmt(node))
				return ret_val;
		}

		else if constexpr (std::is_same_v<T, Node::Struct>)
			interpret_struct(node);

		else if constexpr (std::is_same_v<T, Node::Scope>)
		{
			if (auto ret_val = interpret_scope(node))
				return ret_val;
		}
		return {};
	}, node.node);
}

void Interpreter::interpret_struct(const Node::Struct& node)
{
	std::visit([this](auto&& node) -> void {
		using T = std::decay_t<decltype(node)>;
		if constexpr (std::is_same_v<T, Node::StructFuncDecl>)
			interpret_func_decl(node);

	}, node.strct);
}

void Interpreter::interpret_func_decl(const Node::StructFuncDecl& node)
{
	// this part is not important for now as it is handled by other parts of the interpreter and might get removed
}

std::optional<Node::LitInt> Interpreter::interpret_call(const Node::NodeCall& node)
{
	return std::visit([this, node](auto&& fn) -> std::optional<Node::LitInt> {
		using T = std::decay_t<decltype(fn)>;
		if constexpr (std::is_same_v<T, Node::LitIdent>)
		{
			if (auto it = m_vars.find(fn); it == m_vars.end())
				err_exit(typeid(*this).name(), m_index, "Could not find callable with name: " + fn.id);

			if (std::holds_alternative<Node::LitInt>(m_vars.at(fn)))
				err_exit(typeid(*this).name(), m_index, "Integer literal is not callable");

			const Node::Struct& strct = std::get<Node::Struct>(m_vars.at(fn));
			const Node::StructFuncDecl& fun_decl = std::get<Node::StructFuncDecl>(strct.strct);

			if (fun_decl.params.size() != node.args.size())
				err_exit(typeid(*this).name(), m_index, "parameter/argument mismatch");

			auto m_vars_copy = m_vars;

			for (int i = 0; i < fun_decl.params.size(); i++)
			{
				const Node::LitIdent& ident = fun_decl.params[i];
				const Node::Expr& arg = node.args[i];
				m_vars[ident] = interpret_expr(arg);
			}
			auto ret = interpret_scope(fun_decl.scope);
			m_vars = m_vars_copy;
			return ret;
		}
		else if constexpr (std::is_same_v<T, Node::StructFuncDecl>)
		{
			if (fn.params.size() != node.args.size())
				err_exit(typeid(*this).name(), m_index, "parameter/argument mismatch");

			auto m_vars_copy = m_vars;

			for (int i = 0; i < fn.params.size(); i++)
			{
				const Node::LitIdent& ident = fn.params[i];
				const Node::Expr& arg = node.args[i];
				m_vars[ident] = interpret_expr(arg);
			}
			auto ret = interpret_scope(fn.scope);
			m_vars = m_vars_copy;
			return ret;
		}
	}, node.fn);
	return {};
}

std::optional<Node::LitInt> Interpreter::interpret_ret(const Node::StmtRet& node)
{
	if (!node.ret_val.has_value()) return {};
	return interpret_expr(node.ret_val.value());
}

std::optional<Node::LitInt> Interpreter::interpret_stmt(const Node::Stmt& node)
{
	return std::visit([this](auto&& stmt) -> std::optional<Node::LitInt> {
		using T = std::decay_t<decltype(stmt)>;
		if constexpr (std::is_same_v<T, Node::StmtAsgn>)
			interpret_asgn(stmt);

		else if constexpr (std::is_same_v<T, Node::StmtIf>)
		{

			if (auto ret_val = interpret_if(stmt))
				return ret_val;
		}

		else if constexpr (std::is_same_v<T, Node::StmtLoop>)
		{
			if (auto ret_val = interpret_loop(stmt))
				return ret_val;
		}
		else if constexpr (std::is_same_v<T, Node::StmtRet>)
		{
			if (auto ret_val = interpret_ret(stmt))
				return ret_val;
		}
		return {};
	}, node.stmt);
}

void Interpreter::interpret_asgn(const Node::StmtAsgn& node)
{
	std::visit([this, node](auto&& val) {
		using T = std::decay_t<decltype(val)>;
		if constexpr (std::is_same_v<T, Node::Expr>)
		{
			auto value = interpret_expr(val);
			m_vars[node.id] = Node::LitInt{ value };
		}
		else if constexpr (std::is_same_v<T, Node::Struct>)
		{
			m_vars[node.id] = val;
		}
	}
	, node.val);	
}

std::optional<Node::LitInt> Interpreter::interpret_if(const Node::StmtIf& node)
{
	auto cond = interpret_expr(node.cond);
	if (cond.val != 0)
	{
		return interpret_scope(*node.scope);
	}
	else if(node.elif.has_value())
	{
		return interpret_if(*node.elif.value());
	}
	return {};
}

std::optional<Node::LitInt> Interpreter::interpret_loop(const Node::StmtLoop& node)
{
	if (node.init.has_value())
	{
		interpret_asgn(node.init.value());
	}
	for (node.init.has_value() && (interpret_asgn(node.init.value()), true);
		interpret_expr(node.cond).val;
		node.adv.has_value() && (interpret_asgn(node.adv.value()), true))
	{
		if (auto ret_val = interpret_scope(*node.scope))
			return ret_val;
	}
	return {};
}

Node::LitInt Interpreter::interpret_expr(const Node::Expr& node)
{
	auto res = std::visit([this](auto&& expr) -> Node::LitInt {
		using T = std::decay_t<decltype(expr)>;
		if constexpr (std::is_same_v<T, Node::Lit>)
		{
			return std::visit([this](auto&& lit) -> Node::LitInt {
				using T = std::decay_t<decltype(lit)>;
				if constexpr (std::is_same_v<T, Node::LitInt>)
				{
					return lit;
				}
				else if constexpr (std::is_same_v<T, Node::LitIdent>)
				{
					return std::get<Node::LitInt>(m_vars.at(lit));
				}
				else if constexpr (std::is_same_v<T, Node::NodeCall>)
				{
					if (auto ret_val = interpret_call(lit))
						return ret_val.value();

					err_exit(typeid(*this).name(), m_index, "Must return value for expression");
				}
				}, expr.lit);
		}
		else if constexpr (std::is_same_v<T, Node::BinExpr>)
		{
			return perform_op(interpret_expr(*expr.lhs.value()), interpret_expr(*expr.rhs.value()), expr.op.value());
		}
		else if constexpr (std::is_same_v<T, Node::NodeCall>)
		{
			if (auto ret_val = interpret_call(expr))
				return ret_val.value();

			err_exit(typeid(*this).name(), m_index, "Expected return value from call");
		}
		return {}; // never reached
	}, node.expr);
	return res;
}

std::optional<Node::LitInt> Interpreter::interpret_scope(const Node::Scope& node)
{
	for (const Node::Stmt& stmt : node.stmts)
	{
		if (auto ret_val = interpret_stmt(stmt))
			return ret_val;
	}

	return {};
}

[[nodiscard]] std::optional<Node::Node> Interpreter::peek(int offset)
{
	if (m_index + offset < m_nodes.size())
		return m_nodes[m_index + offset];
	return {};
}

Node::Node Interpreter::consume(unsigned int amount)
{
	if (amount == 0) [[unlikely]]
	{
		err_exit(typeid(*this).name(), m_index, "Consume called with a value of 0");
	}
	m_index += amount;
	return peek(-1).value(); // return last consumed token
}

Node::LitInt Interpreter::perform_op(const Node::LitInt& v1, const Node::LitInt& v2, TokenTypes::Operator op) const
{
	switch (op)
	{
		case TokenTypes::Operator::MULT:
			return Node::LitInt{ v1.val * v2.val };
		case TokenTypes::Operator::DIV:
			return Node::LitInt{ v1.val / v2.val };
		case TokenTypes::Operator::ADD:
			return Node::LitInt{ v1.val + v2.val };
		case TokenTypes::Operator::SUB:
			return Node::LitInt{ v1.val - v2.val };
		case TokenTypes::Operator::BW_AND:
			return Node::LitInt{ v1.val & v2.val };
		case TokenTypes::Operator::BW_OR:
			return Node::LitInt{ v1.val | v2.val };
		case TokenTypes::Operator::AND:
			return Node::LitInt{ v1.val && v2.val };
		case TokenTypes::Operator::OR:
			return Node::LitInt{ v1.val || v2.val };
		case TokenTypes::Operator::LT:
			return Node::LitInt{ v1.val < v2.val };
		case TokenTypes::Operator::GT:
			return Node::LitInt{ v1.val > v2.val };
		case TokenTypes::Operator::LTE:
			return Node::LitInt{ v1.val <= v2.val };
		case TokenTypes::Operator::GTE:
			return Node::LitInt{ v1.val >= v2.val };
		default:
			err_exit(typeid(*this).name(), m_index, "Not a valid operator");
			return {};
	}
}


