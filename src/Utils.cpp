#include "Utils.h"
#include "Compiler.h"

NullStream g_null_logger;
std::string format_instr(const Instr& instr)
{
	std::stringstream ss;
	switch (instr.val.v_type)
	{
		case ValueType::NIL: ss << opcode_to_string(instr.code) << " NIL"; break;
		case ValueType::LIT: ss << opcode_to_string(instr.code) << " " << instr.val.operand; break;
		case ValueType::VAR: ss << opcode_to_string(instr.code) << " [" << instr.val.operand << "]"; break;
		case ValueType::ABS_VAR: ss << opcode_to_string(instr.code) << " (" << instr.val.operand << ")"; break;
		case ValueType::NOT_REQUIRED: ss << opcode_to_string(instr.code); break;
	}
	return ss.str();
}