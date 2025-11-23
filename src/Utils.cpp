#include "Utils.h"
#include "Compiler.h"

Logger& Logger::operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
Logger& Logger::operator<<(std::ios& (*)(std::ios&)) { return *this; }

template <typename T>
Logger& Logger::operator<<(const T&) { return *this; }

Logger g_logger;
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