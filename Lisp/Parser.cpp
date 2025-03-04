#include "Parser.h"
#include "Logger.h"

Parser::Parser(std::vector<Token>& tokens) : m_tokens(std::move(tokens)), m_index(0) {}

std::vector<Node::Stmt> Parser::parse_prog()
{
	std::vector<Node::Stmt> prog;
	while (peek().has_value())
	{
		auto stmt = strict(parse_stmt());
		prog.push_back(stmt);
	}
	return prog;
}

std::optional<Node::Expr> Parser::parse_expr()
{
	if (auto token = peek(); token.has_value() &&
		(token.value() == TokenTypes::Literal::INT || token.value() == TokenTypes::Literal::IDENT))
	{
		consume();
		return Node::Expr{ 
			token.value() == TokenTypes::Literal::INT ?
			Node::Lit{ Node::LitInt{ std::stoi(token.value().value.value()) } } :
			Node::Lit{ Node::LitIdent{ token.value().value.value() } }
		};
	}
	if (!peek().has_value() || peek().value() != TokenTypes::Symbol::OPEN_PAREN) return {};

	consume(); // skip '('
	Node::BinExpr node;
	while (peek().has_value() && peek().value() != TokenTypes::Symbol::CLOSE_PAREN)
	{
		if (
			auto token = peek().value();
			token == TokenTypes::Operator::ADD ||
			token == TokenTypes::Operator::SUB ||
			token == TokenTypes::Operator::MULT ||
			token == TokenTypes::Operator::DIV ||
			token == TokenTypes::Operator::BW_AND ||
			token == TokenTypes::Operator::BW_OR ||
			token == TokenTypes::Operator::GT ||
			token == TokenTypes::Operator::LT
			)
		{
			if (node.op == TokenTypes::Operator::BW_OR && token == TokenTypes::Operator::BW_OR)
				node.op = TokenTypes::Operator::OR;

			else if (node.op == TokenTypes::Operator::BW_AND && token == TokenTypes::Operator::BW_AND)
				node.op = TokenTypes::Operator::AND;

			else if (node.op == TokenTypes::Operator::GT && token == TokenTypes::Operator::ASGN)
				node.op = TokenTypes::Operator::GTE;

			else if (node.op == TokenTypes::Operator::LT && token == TokenTypes::Operator::ASGN)
				node.op = TokenTypes::Operator::LTE;

			else
			{
				if (node.op != TokenTypes::Operator::NONE)
				{
					err_exit("Expected expression");
				}
				node.op = token.extract<TokenTypes::Operator>();
			}
			consume();
		}
		else if (auto token = peek().value(); token == TokenTypes::Literal::INT)
		{
			if (node.lhs == nullptr)
			{
				node.lhs = std::make_shared<Node::Expr>(Node::Expr{ Node::Lit { Node::LitInt{ std::stoi(token.value.value()) } } });
			}
			else if (node.rhs == nullptr)
			{
				node.rhs = std::make_shared<Node::Expr>(Node::Expr{ Node::Lit { Node::LitInt{ std::stoi(token.value.value()) } } });
			}
			consume();
		}
		else if (peek().value() == TokenTypes::Literal::IDENT)
		{
			if (node.lhs == nullptr)
			{
				node.lhs = std::make_shared<Node::Expr>(Node::Expr{ Node::Lit { Node::LitIdent{ token.value.value() } } });
			}
			else if (node.rhs == nullptr)
			{
				node.rhs = std::make_shared<Node::Expr>(Node::Expr{ Node::Lit { Node::LitIdent{ token.value.value() } } });
			}
			consume();
		}
		else if (peek().value() == TokenTypes::Symbol::OPEN_PAREN)
		{
			if (auto expr = parse_expr(); expr.has_value())
			{
				LOG_DEBUG(node_to_string(Node::Stmt{ expr.value() }));
				if (node.lhs == nullptr)
				{
					node.lhs = std::make_shared<Node::Expr>(expr.value());
				}
				else
				{
					node.rhs = std::make_shared<Node::Expr>(expr.value());
				}
			}
			else
			{
				err_exit("Expected sub-expression");
			}


		}
		else
		{
			return {};
			// err_exit("Unrecognized token while parsing expression: " + Tokenizer::tokentype_to_string(peek().value().type));
		}
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
		else err_exit("Expected ')'");

		return Node::Scope{ stmts };
	}
	return {};
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
			{
				node.elif = std::make_shared<Node::StmtIf>(if_chain.value());
			}

			return node;
		}

		Node::StmtIf node;
		node.cond = Node::Expr{ Node::Lit{ Node::LitInt{ 1 } } };
		node.scope = std::make_shared<Node::Scope>(strict(parse_scope()));

		consume(); // skip ')'
		return node;
	}
	return {}; // no elif chain exists
}

std::optional<Node::StmtAsgn> Parser::parse_asgn_stmt()
{
	if (peek().has_value() && peek().value() == TokenTypes::Symbol::OPEN_PAREN &&
		peek(1).has_value() && peek(1).value() == TokenTypes::Operator::ASGN)
	{
		consume(2);
		Node::StmtAsgn node;

		if (auto token = peek(); !token.has_value() || token.value() != TokenTypes::Literal::IDENT) return {}; // expected identifier

		auto ident = consume();
		node.id = Node::LitIdent{ ident.value.value() };
		auto expr = parse_expr();
		if (!expr.has_value()) return {}; // expected expression
		
		node.expr = expr.value();

		consume(); // skip ')'
		return node;

	}
	return {}; // there's no assigment statement
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
		consume(); // skip ')'

		if (auto if_chain = parse_if_chain(); if_chain.has_value())
		{
			node.elif = std::make_shared<Node::StmtIf>(if_chain.value());
		}
		
		return node;
	}
	return {}; // there's no if statement
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
		
		consume(); // skip ')'
		return node;
	}
	return {}; // there's no loop statement
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

	else if (auto node = parse_expr())
		ret_node = Node::Stmt{ node.value() };

	return ret_node;
}

void Parser::err_exit(const std::string& msg)
{
	std::cerr << "[ERROR]" << "[INDEX:" << m_index << "]" << " -> " << msg << std::endl;
	exit(EXIT_FAILURE);
}

[[nodiscard]] std::optional<Token> Parser::peek(int offset)
{
	if (m_index + offset < m_tokens.size())
		return m_tokens[m_index + offset];
	return {};
}

[[nodiscard]] Token Parser::consume(unsigned int amount)
{
	if (amount == 0) [[unlikely]]
	{
		err_exit("Consume called with a value of 0");
	}

	m_index += amount;
	return peek(-1).value(); // return last consumed token
}