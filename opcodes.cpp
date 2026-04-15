#include"opcodes.h"

const char* codeOpNames[] = 
{
	"OP_NO_OP",
	"OP_PUSH_VALUE",
	"OP_PUSH_VAR",
	"OP_PUSH_STRING",

	"OP_ADD",
	"OP_SUB",
	"OP_MULT",
	"OP_DIV",
	"OP_NEG",
	"OP_NOT",
	"OP_OR",
	"OP_AND",
	"OP_EQ",
	"OP_LT",
	"OP_LE",

	"OP_ALLOC",
	"OP_INDEX",
	"OP_ASSIGN_HEAP",

	"OP_ASSIGN",
	"OP_JMP_FALSE",
	"OP_JMP",
	"OP_MOVE_SP",
	"OP_CALL",
	"OP_CALL_NATIVE",
	"OP_RET",
	"OP_PRINT",
	"OP_HLT",
};
