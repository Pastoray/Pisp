#include "Parser.h"
#include "Utils.h"

Parser::Parser(std::vector<Token>& tokens) : m_tokens(std::move(tokens)), m_index(0) {}

std::vector<Node::Node> Parser::parse_prog()
{
	std::vector<Node::Node> prog;
	while (peek().has_value())
	{
		auto node = strict(parse_node());
		prog.push_back(node);
	}
	return prog;
}

std::optional<Node::Node> Parser::parse_node()
{
	if (auto node = parse_stmt())
		return Node::Node{ std::move(node.value()) };

	else if (auto node = parse_expr())
		return Node::Node{ std::move(node.value()) };

	else if (auto node = parse_struct())
		return Node::Node{ std::move(node.value()) };

	else if (auto node = parse_scope())
		return Node::Node{ std::move(node.value()) };
	
	return {};
}

std::optional<Node::Lit> Parser::parse_lit()
{
	if (peek().has_value() && peek().value() == TokenTypes::Literal::INT)
		return Node::Lit{ Node::LitInt{ std::stoi(consume().value.value()) } };

	else if (peek().has_value() && peek().value() == TokenTypes::Literal::IDENT)
		return Node::Lit{ Node::LitIdent{ consume().value.value() } };

	return {};
}

std::optional<Node::Struct> Parser::parse_struct()
{
	if (auto node = parse_func_decl())
		return Node::Struct{ std::move(node.value()) };
	return {};
}

std::optional<Node::StructFuncDecl> Parser::parse_func_decl()
{
	if (!peek().has_value() || peek().value() != TokenTypes::Symbol::OPEN_PAREN ||
		!peek(1).has_value() || peek(1).value() != TokenTypes::Struct::FUNC)
		return {};

	consume(2);

	Node::StructFuncDecl node;
	consume(); // '('

	while (peek().has_value() && peek().value() != TokenTypes::Symbol::CLOSE_PAREN)
	{
		if (peek().value() != TokenTypes::Literal::IDENT)
			ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Expected identifiers in function parameter list");

		node.params.push_back(Node::LitIdent{ consume().value.value()});
	}
	consume(); // ')' (param list)
	node.scope = strict(parse_scope());
	consume(); // ')'
	return node;
}

std::optional<Node::Expr> Parser::parse_expr()
{
	if (auto token = peek(); token.has_value())
	{
		if (auto lit = parse_lit())
			return Node::Expr{ std::move(lit.value()) };

		else if (auto call = parse_call())
			return Node::Expr{ call.value() };

	}
	if (!peek().has_value() || peek().value() != TokenTypes::Symbol::OPEN_PAREN) return {};
	if (auto token = peek(1); !token.has_value() ||
		(
		token.value() != TokenTypes::Operator::ADD &&
		token.value() != TokenTypes::Operator::SUB &&
		token.value() != TokenTypes::Operator::MUL &&
		token.value() != TokenTypes::Operator::DIV &&
		token.value() != TokenTypes::Operator::BW_AND &&
		token.value() != TokenTypes::Operator::BW_OR &&
		token.value() != TokenTypes::Operator::AND &&
		token.value() != TokenTypes::Operator::OR &&
		token.value() != TokenTypes::Operator::GT &&
		token.value() != TokenTypes::Operator::LT &&
		token.value() != TokenTypes::Operator::GTE &&
		token.value() != TokenTypes::Operator::LTE
		)
	) return {};

	consume(); // '('
	Node::BinExpr node;
	while (peek().has_value() && peek().value() != TokenTypes::Symbol::CLOSE_PAREN)
	{
		if (
			auto token = peek().value();
			token == TokenTypes::Operator::ADD ||
			token == TokenTypes::Operator::SUB ||
			token == TokenTypes::Operator::MUL ||
			token == TokenTypes::Operator::DIV ||
			token == TokenTypes::Operator::BW_AND ||
			token == TokenTypes::Operator::BW_OR ||
			token == TokenTypes::Operator::AND ||
			token == TokenTypes::Operator::OR ||
			token == TokenTypes::Operator::GT ||
			token == TokenTypes::Operator::LT ||
			token == TokenTypes::Operator::GTE ||
			token == TokenTypes::Operator::LTE
			)
		{
			if (node.op.has_value())
				ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Expected expression");

			node.op = std::get<TokenTypes::Operator>(token.type);
			consume();
		}
		else if (auto lit = parse_lit())
		{
			if (!node.lhs.has_value())
				node.lhs = std::make_shared<Node::Expr>(Node::Expr{ std::move(lit.value()) });

			else if (!node.rhs.has_value())
				node.rhs = std::make_shared<Node::Expr>(Node::Expr{ std::move(lit.value()) });

		}
		else if (auto call = parse_call())
		{
			if (!node.lhs.has_value())
				node.lhs = std::make_shared<Node::Expr>(Node::Expr{ std::move(call.value()) });

			else if (!node.rhs.has_value())
				node.rhs = std::make_shared<Node::Expr>(Node::Expr{ std::move(call.value()) });
		}
		else if (peek().value() == TokenTypes::Symbol::OPEN_PAREN)
		{
			auto expr = strict(parse_expr()); // "Expected sub-expression"
			LOGGER << (node_to_string(Node::Node{ expr }));

			if (!node.lhs.has_value())
				node.lhs = std::make_shared<Node::Expr>(expr);

			else
				node.rhs = std::make_shared<Node::Expr>(expr);

		}
		else
			return {};
	}
	consume();
	return Node::Expr{ node };
}

std::optional<Node::Scope> Parser::parse_scope()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN)
	{
		consume();
		std::vector<Node::Stmt> stmts;
		while (peek().has_value() && peek().value() != TokenTypes::Symbol::CLOSE_PAREN)
		{
			auto stmt = strict(parse_stmt());
			stmts.push_back(stmt);
		}
		if (peek().has_value()) consume(); // skip ')'
		else ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Expected ')'");

		return Node::Scope{ stmts };
	}
	return {};
}

std::optional<Node::Call> Parser::parse_call()
{
	if (!peek().has_value() || peek().value() != TokenTypes::Symbol::OPEN_PAREN ||
		!peek(1).has_value() || peek(1).value() != TokenTypes::Statement::CALL)
		return {};

	consume(2);

	Node::Call node;
	if (peek().has_value() && peek().value() == TokenTypes::Literal::IDENT)
		node.fn = Node::LitIdent{ consume().value.value() };

	else if (auto fn = parse_func_decl())
		node.fn = fn.value();

	else
		return {};

	consume(); // '('
	while (peek().has_value() && peek().value() != TokenTypes::Symbol::CLOSE_PAREN)
		node.args.push_back(strict(parse_expr()));
	
	consume(); // ')' (arg list)
	consume(); // ')' (stmt end)

	return node;
}

std::optional<Node::StmtIf> Parser::parse_if_chain()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
		peek(1).has_value() && peek(1).value() == TokenTypes::Statement::ELSE)
	{
		consume(2);
		if (peek().has_value() && peek().value() == TokenTypes::Statement::IF)
		{
			consume();
			Node::StmtIf node;
			auto expr = strict(parse_expr());

			node.cond = expr;
			node.scope = std::make_shared<Node::Scope>(strict(parse_scope()));
			consume();
			if (auto if_chain = parse_if_chain(); if_chain.has_value())
				node.elif = std::make_shared<Node::StmtIf>(if_chain.value());

			return node;
		}

		Node::StmtIf node;
		node.cond = Node::Expr{ Node::Lit{ Node::LitInt{ 1 } } };
		node.scope = std::make_shared<Node::Scope>(strict(parse_scope()));

		consume(); // ')'
		return node;
	}
	return {}; // no elif
}

std::optional<Node::StmtAsgn> Parser::parse_asgn_stmt()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
		peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN)
	{
		consume(2);
		Node::StmtAsgn node;

		if (auto token = peek(); !token.has_value() || token.value() != TokenTypes::Literal::IDENT) return {};

		auto ident = consume();
		node.id = Node::LitIdent{ ident.value.value() };
		
		if (auto expr = parse_expr())
			node.val = expr.value();

		else if (auto strct = parse_struct())
			node.val = strct.value();

		else
			ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Expected expression after assignment");

		consume(); // ')'
		return node;

	}
	return {}; // no assigment
}

std::optional<Node::StmtIf> Parser::parse_if_stmt()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
		peek(1).has_value() && peek(1).value() == TokenTypes::Statement::IF)
	{
		consume(2);
		Node::StmtIf node;
		auto expr = strict(parse_expr());

		node.cond = expr;
		node.scope = std::make_shared<Node::Scope>(strict(parse_scope()));
		consume(); // ')'

		if (auto if_chain = parse_if_chain(); if_chain.has_value())
		{
			node.elif = std::make_shared<Node::StmtIf>(if_chain.value());
		}
		
		return node;
	}
	return {}; // no if
}

std::optional<Node::StmtLoop> Parser::parse_loop_stmt()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
		peek(1).has_value() && peek(1).value() == TokenTypes::Statement::LOOP)
	{
		consume(2);
		Node::StmtLoop node;
		node.init = parse_asgn_stmt();

		auto expr = strict(parse_expr());
		node.cond = expr;
		node.adv = parse_asgn_stmt();
		node.scope = std::make_shared<Node::Scope>(strict(parse_scope()));
		
		consume(); // ')'
		return node;
	}
	return {}; // no loop
}

std::optional<Node::StmtRet> Parser::parse_ret_stmt()
{
	if (!peek().has_value() || peek().value() != TokenTypes::Symbol::OPEN_PAREN ||
		!peek(1).has_value() || peek(1).value() != TokenTypes::Statement::RET)

		return {};

	consume(2);
	Node::StmtRet ret_node{ parse_expr() };
	consume();
	return ret_node;
}

std::optional<Node::Stmt> Parser::parse_stmt()
{
	std::optional<Node::Stmt> ret_node{};
	if (auto node = parse_asgn_stmt())
		ret_node = Node::Stmt{ node.value() };

	else if (auto node = parse_if_stmt())
		ret_node = Node::Stmt{ node.value() };

	else if (auto node = parse_loop_stmt())
		ret_node = Node::Stmt{ node.value() };

	else if (auto node = parse_ret_stmt())
		ret_node = Node::Stmt{ node.value() };

	return ret_node;
}

[[nodiscard]] std::optional<Token> Parser::peek(int offset)
{
	if (m_index + offset < m_tokens.size())
		return m_tokens[m_index + offset];
	return {};
}

Token Parser::consume(unsigned int amount)
{
	if (amount == 0) [[unlikely]]
		ERR_EXIT("[INDEX: ", std::to_string(m_index), "] ", "Consume called with a value of 0");

	m_index += amount;
	return peek(-1).value(); // return last consumed token
}