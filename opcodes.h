#pragma once


enum CodeOp {
	OP_NO_OP, // for debug purposes (it is just the default 0s in op)
	OP_PUSH_VALUE, //value
	OP_PUSH_VAR, //value stack address 
	OP_PUSH_STRING, //string link
	OP_ADD, 
	OP_SUB,
	OP_MULT,
	OP_DIV,
	OP_NEG,
	OP_NOT,
	OP_OR,
	OP_AND,
	OP_EQ,
	OP_LT,
	OP_LE,
	OP_ASSIGN, //value stack address
	OP_DECL_VAR, 
	OP_JMP_FALSE, //address
	OP_JMP, //address
	OP_MOVE_VAR_SP, //address
	OP_SAVE_BP,
	OP_CALL, //address
	OP_CALL_NATIVE, //some handle?
	OP_RET,
	OP_PRINT,
	OP_HLT,
};

extern const char* codeOpNames[];
