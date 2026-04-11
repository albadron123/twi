#pragma once
#include"utils.h"

extern char* codeSection;
extern int codeSectionSize;

extern char* staticSection;


enum CodeOp {
	OP_PUSH_VALUE, //value
	OP_PUSH_STRING, //string link
	OP_ADD, 
	OP_SUB,
	OP_MULT,
	OP_DIV,
	OP_NEG,
	OP_NOT,
	OP_OR,
	OP_AND,
	OP_ASSIGN, //lvalue
	OP_DECL_VAR,
	
};


void init_compilation();

bool prepass();

void compile_expression(Expr* expr);
void compile_stmt(Stmt* stmt);

void disassemble();
bool write_bytecode();

