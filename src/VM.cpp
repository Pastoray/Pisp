#include "VM.h"

VM::VM(std::vector<Instr>& bytecode) : m_bytecode(std::move(bytecode)), m_bp(0), m_ip(0) {}

void VM::run()
{
	while (m_ip < m_bytecode.size())
	{
		exec_next();
	}
}

void VM::exec_next()
{
	const auto& instr = m_bytecode[m_ip];
	switch (instr.code)
	{
		case (OpCode::PUSH): push(instr.val); break;
		case (OpCode::POP): pop(); break;
		case (OpCode::MOV): move(instr.val); break;

		case (OpCode::PUSH_SF): push_sf(); break;
		case (OpCode::POP_SF): pop_sf(); break;

		case (OpCode::ADD): add(); break;
		case (OpCode::SUB): sub(); break;
		case (OpCode::MUL): mul(); break;
		case (OpCode::DIV): div(); break;

		case (OpCode::BW_OR): bw_or(); break;
		case (OpCode::BW_AND): bw_and(); break;
		case (OpCode::OR): or_(); break;
		case (OpCode::AND): and_(); break;

		case (OpCode::LT): lt(); break;
		case (OpCode::GT): gt(); break;
		case (OpCode::GTE): gte(); break;
		case (OpCode::LTE): lte(); break;
		case (OpCode::EQL): eql(); break;

		case (OpCode::JMP): jmp(instr.val); break;
		case (OpCode::JMP_ZERO): jmp_zero(instr.val); break;

		case (OpCode::HLT): hlt(); break;

		default: ERR_EXIT("Unknown opcode");
	}
}

void VM::move(Value val)
{
	Value stack_top = m_stack.back();
	m_stack.pop_back();
	m_stack[val.operand + m_bp] = stack_top;
	m_ip++;
}

void VM::push_sf()
{

	m_stack.push_back({ ValueType::LIT, static_cast<int>(m_bp) });
	size_t move_to = m_stack.size();

	m_ip++;
	while (m_ip < m_bytecode.size() && m_bytecode[m_ip].code != OpCode::JMP)
	{
		exec_next();
	}
	exec_next();
	m_bp = move_to;
}

void VM::pop_sf()
{
	Value old_bp = m_stack.back();
	m_stack.pop_back();

	Value ret_addr = m_stack.back();
	m_stack.pop_back();

	m_bp = old_bp.operand;
	m_ip = ret_addr.operand;
}

void VM::add()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 + val1 });
	m_ip++;
}

void VM::sub()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 - val1 });
	m_ip++;
}

void VM::mul()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 * val1 });
	m_ip++;
}

void VM::div()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 / val1 });
	m_ip++;
}

void VM::bw_or()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 | val1 });
	m_ip++;
}

void VM::bw_and()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 & val1 });
	m_ip++;
}

void VM::or_()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 || val1 });
	m_ip++;
}

void VM::and_()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 && val1 });
	m_ip++;
}

void VM::lt()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 < val1 });
	m_ip++;
}

void VM::gt()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 > val1 });
	m_ip++;
}

void VM::gte()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 >= val1 });
	m_ip++;
}

void VM::lte()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 <= val1 });
	m_ip++;
}

void VM::eql()
{
	Value v1 = m_stack.back();
	m_stack.pop_back();

	Value v2 = m_stack.back();
	m_stack.pop_back();

	int val1 = v1.v_type == ValueType::LIT ? v1.operand : m_stack[v1.operand + m_bp].operand;
	int val2 = v2.v_type == ValueType::LIT ? v2.operand : m_stack[v2.operand + m_bp].operand;

	m_stack.push_back({ ValueType::LIT, val2 == val1 });
	m_ip++;
}

void VM::jmp(Value val)
{
	if (val.v_type == ValueType::LIT)
	{
		m_ip = val.operand;
	}

	else if (val.v_type == ValueType::VAR)
	{
		m_ip = m_stack[val.operand + m_bp].operand;
	}

	else if (val.v_type == ValueType::ABS_VAR)
	{
		m_ip = m_stack[val.operand].operand;
	}
}

void VM::jmp_zero(Value val)
{
	Value cond = m_stack.back();
	m_stack.pop_back();

	if (cond.operand == 0)
		m_ip = val.operand;
	else
		m_ip++;
}

void VM::hlt()
{
	LOGGER << "*Program Finished..*" << std::endl;
	std::cin.get();
	m_ip++;
}

void VM::push(Value val)
{
	m_stack.push_back(val);
	m_ip++;
}

void VM::pop()
{
	m_stack.pop_back();
	m_ip++;
}

