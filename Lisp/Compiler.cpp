#include "Compiler.h"

std::string opcode_to_string(OpCode code)
{
	switch (code)
	{
		case OpCode::MOV: return "MOV";
		case OpCode::PUSH_SF: return "PUSH_SF";
		case OpCode::POP_SF: return "POP_SF";
		case OpCode::PUSH: return "PUSH";
		case OpCode::POP: return "POP";
		case OpCode::ADD: return "ADD";
		case OpCode::SUB: return "SUB";
		case OpCode::MUL: return "MUL";
		case OpCode::DIV: return "DIV";
		case OpCode::BW_OR: return "BW_OR";
		case OpCode::BW_AND: return "BW_AND";
		case OpCode::OR: return "OR";
		case OpCode::AND: return "AND";
		case OpCode::LT: return "LT";
		case OpCode::GT: return "GT";
		case OpCode::GTE: return "GTE";
		case OpCode::LTE: return "LTE";
		case OpCode::EQL: return "EQL";
		case OpCode::JMP: return "JMP";
		case OpCode::JMP_ZERO: return "JMP_ZERO";
		case OpCode::HLT: return "HLT";
		default: return "UNKNOWN";
	}
}

Compiler::Compiler(std::vector<Node::Node>& nodes) : m_nodes(std::move(nodes)), m_curr_env(new Env{ 0, 0, {}, nullptr }) {}

std::vector<Instr> Compiler::compile_prog()
{
	for (const Node::Node& node : m_nodes)
		compile_node(node);

	push_instr(OpCode::HLT, { ValueType::NOT_REQUIRED, -1 });
	return m_bytecode;
}

void Compiler::compile_node(const Node::Node& node)
{
	struct Visitor
	{
		Compiler& compiler;
		void operator()(const Node::Expr& expr)
		{
			compiler.compile_expr(expr);
			compiler.push_instr(OpCode::POP, { ValueType::NOT_REQUIRED, -1 });
		}

		void operator()(const Node::Stmt& stmt)
		{
			compiler.compile_stmt(stmt);
		}

		void operator()(const Node::Struct& strct)
		{
			compiler.compile_struct(strct);
		}

		void operator()(const Node::Scope& scope)
		{
			compiler.compile_scope(scope);
		}
	};
	std::visit(Visitor{ *this }, node.node);
}

void Compiler::compile_asgn(const Node::StmtAsgn& node)
{
	struct Visitor
	{
		Compiler& compiler;
		const std::string& id;
		void operator()(const Node::Expr& expr)
		{
			compiler.compile_expr(expr);
			if (auto it = compiler.m_curr_env->locals.vars.find(id); it == compiler.m_curr_env->locals.vars.end())
				compiler.m_curr_env->locals.vars[id] = compiler.m_curr_env->locals.size();

			else
			{
				int var_loc = static_cast<int>(compiler.m_curr_env->locals.vars[id]);
				compiler.push_instr(OpCode::MOV, { ValueType::VAR, var_loc });
			}
			
		}

		void operator()(const Node::Struct& strct)
		{
			int func_start_loc = static_cast<int>(compiler.m_bytecode.size() + 2);
			compiler.push_instr(OpCode::PUSH, { ValueType::LIT, func_start_loc });
			if (auto it = compiler.m_curr_env->locals.funcs.find(id); it == compiler.m_curr_env->locals.funcs.end())
			{
				compiler.m_curr_env->locals.funcs[id] = compiler.m_curr_env->locals.size();
			}

			else
			{
				int func_loc = static_cast<int>(compiler.m_curr_env->locals.funcs[id]);
				compiler.push_instr(OpCode::MOV, { ValueType::VAR, func_loc });
			}

			compiler.compile_struct(strct);
		}
	};
	std::visit(Visitor{ *this, node.id.id }, node.val);
}

void Compiler::compile_if(const Node::StmtIf& node)
{
	compile_expr(node.cond);

	size_t false_idx = m_bytecode.size();
	push_instr(OpCode::JMP_ZERO, { ValueType::LIT, -1 });

	compile_scope(*node.scope);

	size_t end_idx = m_bytecode.size();
	push_instr(OpCode::JMP, { ValueType::LIT, -1 });

	if (node.elif.has_value())
	{
		m_bytecode[false_idx].val.operand = m_bytecode.size();
		compile_if(*node.elif.value());
		m_bytecode[end_idx].val.operand = m_bytecode.size();
	}
	else
	{
		m_bytecode[false_idx].val.operand = m_bytecode.size() - 1;
		m_bytecode.pop_back();
	}

	
}

void Compiler::compile_loop(const Node::StmtLoop& node)
{
	if (node.init.has_value())
		compile_asgn(node.init.value());

	size_t cond_start = m_bytecode.size();
	compile_expr(node.cond);

	size_t idx = m_bytecode.size();
	push_instr(OpCode::JMP_ZERO, Value{ ValueType::LIT, -1 });

	compile_scope(*node.scope);

	if (node.adv.has_value())
		compile_asgn(node.adv.value());

	push_instr(OpCode::JMP, { ValueType::LIT, static_cast<int>(cond_start) });

	m_bytecode[idx].val.operand = m_bytecode.size();
}

void Compiler::compile_ret(const Node::StmtRet& node)
{
	if (node.ret_val.has_value())
		compile_expr(node.ret_val.value());

	push_instr(OpCode::MOV, { ValueType::VAR, -1 }); // bp - 1 because stack[bp] == return address

	for (int i = 0; i < m_curr_env->locals.size(); i++)
		push_instr(OpCode::POP, { ValueType::NOT_REQUIRED, -1 });

	push_instr(OpCode::POP_SF, { ValueType::NOT_REQUIRED, -1 });
}

void Compiler::compile_stmt(const Node::Stmt& node)
{
	struct Visitor
	{
		Compiler& compiler;
		void operator()(const Node::StmtAsgn& asgn)
		{
			compiler.compile_asgn(asgn);
		}

		void operator()(const Node::StmtIf& if_stmt)
		{
			compiler.compile_if(if_stmt);
		}

		void operator()(const Node::StmtLoop& loop)
		{
			compiler.compile_loop(loop);
		}

		void operator()(const Node::StmtRet& ret)
		{
			compiler.compile_ret(ret);
		}
	};
	std::visit(Visitor{ *this }, node.stmt);
}

void Compiler::compile_scope(const Node::Scope& node)
{
	for (const Node::Stmt& stmt : node.stmts)
		compile_stmt(stmt);
}

void Compiler::compile_struct(const Node::Struct& node)
{
	size_t jmp_idx = m_bytecode.size();
	push_instr(OpCode::JMP, { ValueType::LIT, -1 });

	struct Visitor
	{
		Compiler& compiler;
		void operator()(const Node::StructFuncDecl& fn_decl)
		{
			compiler.push_instr(OpCode::PUSH_SF, { ValueType::NOT_REQUIRED, -1 });
			Env* new_env = new Env();
			
			new_env->parent = compiler.m_curr_env;
			new_env->start = compiler.m_bytecode.size();
			new_env->stack_idx = new_env->parent->stack_idx + new_env->parent->locals.size();

			for (int i = 0; i < fn_decl.params.size(); i++)
			{
				const auto& param = fn_decl.params[i];
				new_env->locals.vars[param.id] = i;
			}

			compiler.m_curr_env = new_env;

			compiler.compile_scope(fn_decl.scope);
			for (int i = 0; i < new_env->locals.size(); i++)
				compiler.push_instr(OpCode::POP, { ValueType::NOT_REQUIRED, -1 });

			compiler.m_curr_env = new_env->parent;
			compiler.push_instr(OpCode::POP_SF, { ValueType::NOT_REQUIRED, -1 });
		}
	};
	std::visit(Visitor{ *this }, node.strct);

	m_bytecode[jmp_idx].val.operand = m_bytecode.size();
}

void Compiler::compile_expr(const Node::Expr& node)
{
	struct Visitor
	{
		Compiler& compiler;
		void operator()(const Node::BinExpr& bin_expr)
		{
			compiler.compile_expr(*bin_expr.lhs.value());
			compiler.compile_expr(*bin_expr.rhs.value());

			switch (bin_expr.op.value())
			{
				case TokenTypes::Operator::EQL:
					compiler.push_instr(OpCode::EQL, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::ADD:
					compiler.push_instr(OpCode::ADD, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::SUB:
					compiler.push_instr(OpCode::SUB, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::DIV:
					compiler.push_instr(OpCode::DIV, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::MUL:
					compiler.push_instr(OpCode::MUL, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::OR:
					compiler.push_instr(OpCode::OR, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::AND:
					compiler.push_instr(OpCode::AND, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::BW_OR:
					compiler.push_instr(OpCode::BW_OR, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::BW_AND:
					compiler.push_instr(OpCode::BW_AND, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::LT:
					compiler.push_instr(OpCode::LT, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::LTE:
					compiler.push_instr(OpCode::LTE, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::GT:
					compiler.push_instr(OpCode::GT, { ValueType::NOT_REQUIRED, -1 });
					break;

				case TokenTypes::Operator::GTE:
					compiler.push_instr(OpCode::GTE, { ValueType::NOT_REQUIRED, -1 });
					break;

				default:
					err_exit("Operator not allowed", typeid(*this).name());

			}
		}
		void operator()(const Node::Lit& lit)
		{
			struct LitVisitor
			{
				Compiler& compiler;
				void operator()(const Node::LitIdent& ident)
				{
					auto loc = compiler.find_var(ident.id);
					if (loc != -1)
					{
						Value val = { ValueType::VAR, static_cast<int>(loc)};
						compiler.push_instr(OpCode::PUSH, val);
					}

					else
					{
						err_exit("Undefined variable: \"" + ident.id + "\"", "Symbol table");
					}

				}

				void operator()(const Node::LitInt& integer)
				{
					Value val = { ValueType::LIT, integer.val };
					compiler.push_instr(OpCode::PUSH, val);
				}
			};
			std::visit(LitVisitor{ compiler }, lit.lit);
		}
		void operator()(const Node::Call& call)
		{
			compiler.compile_call(call);
		}
	};
	std::visit(Visitor{ *this }, node.expr);
}

void Compiler::compile_call(const Node::Call& node)
{
	struct Visitor
	{
		Compiler& compiler;
		const std::vector<Node::Expr>& args;
		void operator()(const Node::LitIdent& ident)
		{
			auto loc = compiler.find_func(ident.id);
			if (loc != -1)
			{
				compiler.push_instr(OpCode::PUSH, { ValueType::NIL, -1 }); // return value

				size_t push_idx = compiler.m_bytecode.size();
				compiler.push_instr(OpCode::PUSH, { ValueType::LIT, -1 }); // return address

				for (const Node::Expr& arg : args)
					compiler.compile_expr(arg);

				int func_loc = static_cast<int>(loc);
				compiler.push_instr(OpCode::JMP, { ValueType::VAR, func_loc });
				compiler.m_bytecode[push_idx].val.operand = compiler.m_bytecode.size(); // return address
			}
			else
			{
				std::cerr << "Warning: Couldn't find function " << "\"" << ident.id << "\"" << std::endl;
			}
		}

		void operator()(const Node::StructFuncDecl& func)
		{
			// currently this uses the current stack frame since it doesn't need an explicit one
			// once garbage collection is introduce this will be reworked
			// (to be tested..)

			for (const Node::Expr& arg : args)
				compiler.compile_expr(arg);

			compiler.compile_scope(func.scope);
		}

	};
	std::visit(Visitor{ *this, node.args }, node.fn);
}

void Compiler::push_instr(OpCode code, Value val)
{
	m_bytecode.emplace_back(code, val);
}


int Compiler::find_func(const std::string& name)
{
	Env* curr = m_curr_env;

	while (curr)
	{
		auto it = curr->locals.funcs.find(name);
		if (it != curr->locals.funcs.end())
			return it->second;

		curr = curr->parent;
	}
	return -1;
}

int Compiler::find_var(const std::string& name)
{
	Env* curr = m_curr_env;

	while (curr)
	{
		auto it = curr->locals.vars.find(name);
		if (it != curr->locals.vars.end())
			return it->second;

		curr = curr->parent;
	}
	return -1;
}
