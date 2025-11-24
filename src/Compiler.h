#pragma once

#include "Parser.h"

enum class OpCode
{
	MOV, // pop the top of the stack and put it at <operand> location in the stack

	PUSH_SF,
	POP_SF,
	PUSH,
	POP,

	ADD,
	SUB,
	MUL,
	DIV,
	BW_OR,
	BW_AND,
	OR,
	AND,
	LT,
	GT,
	GTE,
	LTE,
	EQL,

	JMP,
	JMP_ZERO,

	HLT
};

std::string opcode_to_string(OpCode code);

enum class ValueType
{
	LIT,
	VAR,
	ABS_VAR,
	NIL,
	NOT_REQUIRED
};

struct Locals
{
	std::unordered_map<std::string, size_t> vars{};
	std::unordered_map<std::string, size_t> funcs{};

	size_t size() const
	{
		return vars.size() + funcs.size();
	}
};

struct Env
{
	size_t start;
	size_t stack_idx;
	Locals locals{};
	Env* parent;
};

struct Value
{
	ValueType v_type;
	int operand; // some instructions don't require an operand
};

struct Instr
{
	OpCode code;
	Value val;
};

class Compiler
{
public:
	Compiler(std::vector<Node::Node>& nodes);
	std::vector<Instr> compile_prog();
	void compile_node(const Node::Node& node);
	void compile_asgn(const Node::StmtAsgn& node);
	void compile_if(const Node::StmtIf& node);
	void compile_loop(const Node::StmtLoop& node);
	void compile_ret(const Node::StmtRet& node);
	void compile_stmt(const Node::Stmt& node);
	void compile_scope(const Node::Scope& node);

	void compile_struct(const Node::Struct& node);

	void compile_expr(const Node::Expr& node);
	void compile_call(const Node::Call& node);

private:
	void print_env(const Env* env, int depth = 0);
	void push_instr(OpCode code, Value val);
	Value find_func(const std::string& name);
	Value find_var(const std::string& name);

private:
	const std::vector<Node::Node> m_nodes;
	std::vector<Instr> m_bytecode;
	Env* m_curr_env;
};
