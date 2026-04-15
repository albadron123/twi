#include"utils.h"
#include"opcodes.h"
#include<vector>
#include<map>
#include<stdio.h>


char* codeSection;
int codeSectionSize = 0;
int codeSectionMaxSize = 0;
int codeSectionCapacity = 0;

int currentSP = 0;
int currentBP = 0;


int currentFunctionArgsSize = 0;

char* staticSection;

struct AddressEnv {
	std::map<std::string, int> vars;
	bool functionFrame;
	int stackPointer;
};
std::vector<AddressEnv> addressEnvs = {};

std::map<std::string, int> nativeFunctionHandlers = {
	{"clock", 0},
	{"int", 1},
};

struct UserFunctionHandler {
	int stmtId;
	int lineInCodeSection;
};
std::map<std::string, UserFunctionHandler> userFunctionHandlers = {};

void init_compilation() {
	codeSectionCapacity = 1024*1024;
	codeSectionSize = 0;
	codeSectionMaxSize = 0;
	currentSP = 0;
	currentBP = 0;
	codeSection = (char*)malloc(codeSectionCapacity);
	addressEnvs.push_back({{}});
}

bool write_bytecode() {
	const char* path = "interpreter/bytecode.b";
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    size_t written = fwrite(codeSection, 1, codeSectionMaxSize, f);
    fclose(f);

    return written == codeSectionMaxSize;
}

int disassemble_index(int i) {
	//skip no-ops in disassembler
	if(codeSection[i] == (char)OP_NO_OP) {	
		++i;
		return i;
	} 
	printf("%4d %20s ", i,  codeOpNames[(int)(*(codeSection+i))]);
	switch(codeSection[i]) {
		case (char)OP_PUSH_VALUE:
			printf("\t%lf", *(double*)(codeSection+i+1));
			i+=8+1;
			break;
		case (char)OP_PUSH_VAR:
		case (char)OP_JMP:
		case (char)OP_JMP_FALSE:
		case (char)OP_ASSIGN:
		case (char)OP_MOVE_SP:
		case (char)OP_CALL_NATIVE:
		case (char)OP_CALL:
		case (char)OP_RET:
			printf("\t%d", *(int*)(codeSection+i+1));
			i+=4+1;
			break;
		default:
			++i;
	}
	printf("\n");
	return i;
}

void disassemble() {
	for(int i = 0; i < codeSectionCapacity; ) {
		i = disassemble_index(i);	
	}
}

bool locate_variable(std::string s, int* res) {
	*res = 0;
	int envId = addressEnvs.size()-1;
	while(envId >= 0) {	
		if(addressEnvs[envId].vars.find(s) != 
		   addressEnvs[envId].vars.end()) {
			*res = addressEnvs[envId].vars[s];
			return true;
		}
		if(addressEnvs[envId].functionFrame)
		{
			break;
		}
		--envId;
	}
	// FOR NOW IT IS IMPOSSIBLE TO USE GLOBALS!!!
	/*
	if(envId >= 0)
	{
		if(addressEnvs[0].vars.find(s) != 
		   addressEnvs[0].vars.end()) {
		return addressEnvs[envId].vars[s];
	}	
	*/
	printf("Unable to locate variable '%s' in the current scope.\n",
			s.c_str());
	return false;
}

void add_code_1(char c) {
	codeSection[codeSectionSize] = c;
	++codeSectionSize;
	if(codeSectionSize>codeSectionMaxSize) {
		codeSectionMaxSize = codeSectionSize;
	}
}

void add_code_8(double d) {
	*(double*)(codeSection+codeSectionSize) = d;
	codeSectionSize += 8;
	if(codeSectionSize>codeSectionMaxSize) {
		codeSectionMaxSize = codeSectionSize;
	}
}

void add_code_4(int i) {
	*(int*)(codeSection+codeSectionSize) = i;
	codeSectionSize += 4;
	if(codeSectionSize>codeSectionMaxSize) {
		codeSectionMaxSize = codeSectionSize;
	}
}

void patch_code_4(int address, int i) {
	*(int*)(codeSection+address) = i;
}
void compile_expression(Expr* expr);

//at this stage we are going to locate all the user functions and define
//place for them in memory
//
//IMPORTANT: this is a temporary solution to this problem of generating code
//			 for functions in fixed sized section with extra pre-analysis
bool prepass() {
	int positionInCodeSection = 500;
	int delta = 500;
	for(int i = 0; i < globalStream.size(); ++i) {	
		if(globalStream[i]->type == FUNCTION_STMT) {
			if(userFunctionHandlers.find(globalStream[i]->lvalue->content) !=
			   userFunctionHandlers.end()) {
				printf("Function '%s' declared more then once\n",
					   globalStream[i]->lvalue->content.c_str());
				return false;
			}
			if(nativeFunctionHandlers.find(globalStream[i]->lvalue->content) !=
			   nativeFunctionHandlers.end()) {
				printf("Function '%s' can't be declared (conficts with native func)\n",
					   globalStream[i]->lvalue->content.c_str());
				return false;
			}
			userFunctionHandlers[globalStream[i]->lvalue->content] = {
				i, //line in stmt stream
				positionInCodeSection, 
			};
			positionInCodeSection += delta;
		}
	}
	return true;
}


void compile_stmt(Stmt* stmt) {
	if(stmt == nullptr)
	{
		add_code_1((char)OP_HLT);
		return;
	}
	switch(stmt->type) {
		case IF_STMT:
		{
			compile_expression(stmt->exprs[0]);
			add_code_1((char)OP_JMP_FALSE);
			int jmpAddress = codeSectionSize;
			add_code_4(0);
			compile_stmt(stmt->stmts[0]);
			if(stmt->stmts[1] != 0) {
				add_code_1((char)OP_JMP);
				int jmpAddress_2 = codeSectionSize;
				add_code_4(0);
				patch_code_4(jmpAddress, codeSectionSize);	
				compile_stmt(stmt->stmts[1]);
				patch_code_4(jmpAddress_2, codeSectionSize);
			}
			else 
			{	
				patch_code_4(jmpAddress, codeSectionSize);	
			}	
			return;
		}	
		case GROUP_STMT:
		{
			int groupingSP = currentSP;
			addressEnvs.push_back({{}});

			for(int i = 0; i < stmt->group.size(); ++i) {
				compile_stmt(stmt->group[i]);
			}

			currentSP = groupingSP;
			addressEnvs.pop_back();

			add_code_1((char)OP_MOVE_SP);
			add_code_4(currentSP);
			return;
		}
		case WHILE_STMT:
		{
			int jmpAddress = codeSectionSize;
			compile_expression(stmt->exprs[0]);
			add_code_1((char)OP_JMP_FALSE);
			int patchPlace = codeSectionSize;
			add_code_4(0);
			compile_stmt(stmt->stmts[0]);
			add_code_1((char)OP_JMP);
			add_code_4(jmpAddress);	
			patch_code_4(patchPlace, codeSectionSize);	
			return;
		}
		case PRINT_STMT:
		{		
			compile_expression(stmt->exprs[0]);
			add_code_1((char)OP_PRINT);
			currentSP -= 8;
			return;
		}
		case EXPR_STMT:
		{
			compile_expression(stmt->exprs[0]);
			return;
		}
		case VAR_STMT:
		{
			//we know before the expression is evaluated that
			//at this point we are expecting the result
			addressEnvs[addressEnvs.size()-1]
				.vars[stmt->lvalue->content] = currentSP;

			compile_expression(stmt->exprs[0]);	
			return;
		}
		case FUNCTION_STMT:
		{
			int savepoint = codeSectionSize;
			printf("savepoint:%d\n", savepoint);
			int savepointSP  = currentSP;
			currentSP = 0;
			codeSectionSize = userFunctionHandlers[stmt->lvalue->content]
				.lineInCodeSection;	
			
			addressEnvs.push_back({{}, true});
			
			//NOTE: later we may need to change this to get params of different size
			currentFunctionArgsSize = 0;
			int arity = 0;
			if(stmt->extraIndex >= 0)
			{
				arity = functionParams[stmt->extraIndex].size();
				currentFunctionArgsSize = 8 * arity;
				for(int i = 0; i < arity; ++i) {
					addressEnvs[addressEnvs.size()-1]
						.vars[functionParams[stmt->extraIndex][i]->content] = 
						-8 - 8 * (arity - i);
				}	
			}

			compile_stmt(stmt->stmts[0]);


			//push the default return value & return 
			add_code_1((char)OP_PUSH_VALUE);
			add_code_8(0);	
			add_code_1((char)OP_RET);
			add_code_4(currentFunctionArgsSize);

			addressEnvs.pop_back();
			codeSectionSize = savepoint;
			currentSP = savepointSP;
			return;
		}
		case RETURN_STMT:
		{
			if(stmt->exprs[0]->type == EMPTY) {
				add_code_1((char)OP_PUSH_VALUE);
				add_code_8(0);	
				add_code_1((char)OP_RET);
				add_code_4(currentFunctionArgsSize);
				return;
			}
			compile_expression(stmt->exprs[0]);
			add_code_1((char)OP_RET);	
			add_code_4(currentFunctionArgsSize);
			return;
		}
		default:
			return;
	}

}

void compile_expression(Expr* expr) {

	if(expr->token != 0 && expr->token->type == NUMBER) {
		add_code_1((char)OP_PUSH_VALUE);
		add_code_8(expr->token->value);		
		currentSP += 8;
		return;
	}
	else if (expr->token != 0 && expr->token->type == STRING) {
		return;
	}
	else if (expr->token != 0 && expr->token->type == TRUE) {
		add_code_1((char)OP_PUSH_VALUE);
		add_code_8(1);		
		currentSP += 8;
		return;
	}
	else if (expr->token != 0 && expr->token->type == FALSE) {
		add_code_1((char)OP_PUSH_VALUE);
		add_code_8(0);	
		currentSP += 8;
		return;
	}
	else if (expr->token != 0 && expr->token->type == NIL) {
		return;
	}
	else if (expr->token != 0 && expr->token->type == IDENTIFIER) {
		int address;			
		bool located = locate_variable(expr->token->content, 
									   &address);
		if(located) {
			add_code_1((char)OP_PUSH_VAR);
			add_code_4(address);	
			currentSP += 8;
			return;
		}
		return;
	}
	else if (expr->type == GROUPING) {
		compile_expression(expr->children[0]);
		return;
	}
	else if (expr->type == UNARY) {
		compile_expression(expr->children[0]);
		if(expr->token->type == BANG) {	
			add_code_1((char)OP_NOT);
			return;
		}
		else {
			add_code_1((char)OP_NEG);	
			return;
		}
	}
	else if(expr->type == CALL) {
		// -- check if it is a native or a real call
		if(expr->children[0]->token != 0 && 
		   expr->children[0]->token->type != IDENTIFIER)
		{
			printf("non identifier calees are not supported for now!\n");	
			return;
		}
		bool isNativeFunction = false;
		bool isUserFunction = false;
		if(nativeFunctionHandlers.find(expr->children[0]->token->content) != 
		   nativeFunctionHandlers.end()) {
			isNativeFunction = true;
		}
		if(userFunctionHandlers.find(expr->children[0]->token->content) != 
		   userFunctionHandlers.end()) {
			isUserFunction = true;
		}

		if(!isNativeFunction && !isUserFunction) {
			printf("trying to call a functon '%s' that is never defined\n",
				   expr->children[0]->token->content.c_str());
			return;
		}
				
		//this will push args in stack
		if(expr->extraIndex != -1)
		{
			for(int i = 0; i < functionArgs[expr->extraIndex].size(); ++i) {
				compile_expression(functionArgs[expr->extraIndex][i]);
			}
		}

		if(isUserFunction)
		{		
			add_code_1((char)OP_CALL);
			add_code_4(userFunctionHandlers[expr->children[0]->token->content]
				.lineInCodeSection);	
			//after the call we expect the return value to be pushed on the stack
			currentSP += 8;
			return;
		}
		if(isNativeFunction)
		{
			add_code_1((char)OP_CALL_NATIVE);
			add_code_4(nativeFunctionHandlers[expr->children[0]->token->content]);
			//after the call we expect the return value to be pushed on the stack
			currentSP += 8;
			return;
		}
		printf("unreachable area!\n");
		//Unreachable
		return;
	}
	else if(expr->type == BINARY) {
		if(expr->token->type == EQUAL) {
			int address = 0;
			bool located = locate_variable(expr->children[0]->token->content, 
										   &address);
			if(located)
			{	
				compile_expression(expr->children[1]);	
				add_code_1((char)OP_ASSIGN);
				add_code_4(address);
				return;
			}
			return;
		}
		else if (expr->token->type == GREATER) {
			compile_expression(expr->children[1]);
			compile_expression(expr->children[0]);
			add_code_1((char)OP_LT);
			currentSP -= 16;
			return;
		}
		else if (expr->token->type == GREATER_EQUAL) {
			compile_expression(expr->children[1]);
			compile_expression(expr->children[0]);
			add_code_1((char)OP_LE);
			currentSP -= 16;
			return;
		}
		else {
			compile_expression(expr->children[0]);
			compile_expression(expr->children[1]);
			switch(expr->token->type) {
				case PLUS:	
					add_code_1((char)OP_ADD);
					currentSP -= 16;
					return;
				case MINUS:	
					add_code_1((char)OP_SUB);
					currentSP -= 16;
					return;
				case STAR:
					add_code_1((char)OP_MULT);			
					currentSP -= 16;
					return;
				case SLASH:
					add_code_1((char)OP_DIV);	
					currentSP -= 16;
					return;	
				case AND:
					add_code_1((char)OP_AND);
					currentSP -= 16;
					return;
				case OR:
					add_code_1((char)OP_OR);
					currentSP -= 16;
					return;
				case LESS:
					add_code_1((char)OP_LT);
					currentSP -= 16;
					return;
				case LESS_EQUAL:
					add_code_1((char)OP_LE);
					currentSP -= 16;
					return;
				case EQUAL_EQUAL:
					add_code_1((char)OP_EQ);
					currentSP -= 16;
					return;
				case BANG_EQUAL:
					add_code_1((char)OP_EQ);
					add_code_1((char)OP_NOT);
					currentSP -= 16;
					return;
				default: 
					printf("some binary operation is not supported\n");
					return;
			}
		}
	}	
	else if (expr->type == EMPTY) {
		return; 
	}

	printf("Shouldnt reach here%d\n", expr->type);
	return;
}
