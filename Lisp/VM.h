#pragma once

#include "Compiler.h"

class VM
{
public:
	VM(std::vector<Instr>& bytecode);
	void run();

private:
	void exec_next();

	void push(Value val);
	void pop();
	void move(Value val);

	void push_sf();
	void pop_sf();

	void add();
	void sub();
	void mul();
	void div();

	void bw_or();
	void bw_and();
	void or_();
	void and_();

	void lt();
	void gt();
	void gte();
	void lte();
	void eql();

	void jmp(Value val);
	void jmp_zero(Value val);

	void hlt();

private:
	const std::vector<Instr> m_bytecode;
	std::vector<Value> m_stack;
	size_t m_bp;
	size_t m_ip;
};