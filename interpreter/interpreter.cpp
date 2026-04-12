#include"../opcodes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


char* codeSection;

char* varStackSection;
char* opStack;

int instructionPointer = 0;
int opStackPointer = 0;
int varSP = 0;
int varBP = 0;



inline void push_in_op_stack_8(double d) {
	*((double*)(opStack+opStackPointer)) = d;
	opStackPointer += 8;
}

inline double pop_from_op_stack_8() {
	opStackPointer -= 8;
	return *((double*)(opStack+opStackPointer));
}

inline double peek_op_stack_8() {
	return *((double*)(opStack+opStackPointer-8));
}

inline void push_in_var_stack_8(double d) {
	*((double*)(varStackSection+varBP+varSP)) = d;
	varSP += 8;
}

inline void push_in_var_stack_4(int i) {
	*((int*)(varStackSection+varBP+varSP)) = i;
	varSP += 4;
}


inline double read_next_8() {
	instructionPointer += 8;
	return *(double*)(codeSection+instructionPointer-8);
}

inline int read_next_4() {
	instructionPointer += 4;
	return *(int*)(codeSection+instructionPointer-4);
}

inline double read_var_from_stack_8(int globalAddress) {
	return *((double*)(varStackSection+globalAddress));
}

inline int read_var_from_stack_4(int globalAddress) {
	printf("read int* from %d\n", globalAddress);
	return *((int*)(varStackSection+globalAddress));
}

inline void write_var_on_stack_8(int globalAddress, double d) {
	*((double*)(varStackSection+globalAddress)) = d;
}


bool read_bytecode_from_file(const char* src) {
    FILE* f = fopen(src, "rb");
    if (!f) return false;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    int size = ftell(f);
    if (size < 0) {
        fclose(f);
        return false;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return false;
    }

    codeSection = (char*)malloc((size_t)size + 1);

    size_t nread = fread(codeSection, 1, (size_t)size, f);
    fclose(f);

    if (nread != (size_t)size) {
        return false;
    }

    codeSection[size] = '\0'; 
	return true;
}


void twi_clock() {
	printf(":: not implemented\n");
	push_in_op_stack_8(10);
}

void twi_int() {	
	printf(":: not implemented\n");
	push_in_op_stack_8(10);
}


typedef void (*NativeFunction)();
NativeFunction funcs[] = 
{
	twi_clock,
	twi_int,	
};


void run(const char* src, bool runVerbose) {
	if(!read_bytecode_from_file(src)) {
		printf("panic: coudn't read the bytecode\n");
		return;
	}

	varStackSection = (char*)malloc(1024*1024*sizeof(char));
	opStack = (char*)malloc(1024*1024*sizeof(char));

	instructionPointer = 0;
	opStackPointer = 0;
	varSP = 0;

	while(((char*)codeSection)[instructionPointer] != OP_HLT) {
		char op = ((char*)codeSection)[instructionPointer];
		if(runVerbose) {
			printf("%4d : %s\n", 
					instructionPointer,
					codeOpNames[(int)op]);
		}
		++instructionPointer;
		switch(op) {
			case OP_NO_OP:
			{
				printf("no op hit\n");
				return;
			}
			case OP_PUSH_VALUE:
			{
				double val = read_next_8();
				push_in_op_stack_8(val);
				break;
			}
			case OP_PUSH_VAR:
			{
				int address = read_next_4();
				double val = read_var_from_stack_8(varBP + address);	
				push_in_op_stack_8(val);				
				break;
			}
			case OP_ASSIGN:
			{
				int address = read_next_4();
				double val = peek_op_stack_8();
				write_var_on_stack_8(varBP + address, val);	
				break;
			}
			case OP_MOVE_VAR_SP:
			{
				int address = read_next_4();
				varSP = varBP + address;
				break;
			}
			case OP_ADD:
			{
				//tried to optimize the +:
				opStackPointer -= 8;
				*((double*)(opStack+opStackPointer-8)) +=
					*((double*)(opStack+opStackPointer));
				/*
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a+b);
				*/
				break;
			}
			case OP_SUB:
			{
				//tried to optimize the -:
				opStackPointer -= 8;
				*((double*)(opStack+opStackPointer-8)) -=
					*((double*)(opStack+opStackPointer));
				/*
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a-b);
				*/
				break;
			}
			case OP_MULT:
			{
				//tried to optimize the *:
				opStackPointer -= 8;
				*((double*)(opStack+opStackPointer-8)) *=
					*((double*)(opStack+opStackPointer));
				/*
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a*b);	
				*/
				break;
			}
			case OP_DIV:
			{
				//tried to optimize the /:
				opStackPointer -= 8;
				*((double*)(opStack+opStackPointer-8)) /=
					*((double*)(opStack+opStackPointer));
				/*
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a/b);
				*/
				break;
			}
			case OP_NEG:
			{
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(-a);
				break;
			}
			case OP_AND:
			{
				double a = pop_from_op_stack_8();
				double b = pop_from_op_stack_8();
				push_in_op_stack_8((double)((bool)(int)(a) && (bool)(int)(b)));
				break;
			}
			case OP_OR:
			{
				double a = pop_from_op_stack_8();	
				double b = pop_from_op_stack_8();
				push_in_op_stack_8((double)((bool)(int)(a) || (bool)(int)(b)));
				break;	
			}
			case OP_NOT:
			{
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(!(bool)(int)a);
				break;
			}
			case OP_EQ:
			{
				double a = pop_from_op_stack_8();
				double b = pop_from_op_stack_8();
				push_in_op_stack_8((double)(a == b));
				break;
			}
			case OP_LT:
			{
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a < b);
				break;
			}
			case OP_LE:
			{
				double b = pop_from_op_stack_8();
				double a = pop_from_op_stack_8();
				push_in_op_stack_8(a <= b);
				break;
			}
			case OP_DECL_VAR: 
			{
				double val = pop_from_op_stack_8();
				push_in_var_stack_8(val);
				break;
			}
			case OP_JMP_FALSE:
			{
				bool val = (bool)(int)pop_from_op_stack_8();
				if(!val) {
					int address = read_next_4();
					instructionPointer = address;
				}
				else
				{
					instructionPointer += 4;
				}
				break;
			}
			case OP_JMP:
			{
				int address = read_next_4();
				instructionPointer = address;
				break;
			}
			case OP_PRINT:
			{
				double val = pop_from_op_stack_8();
				printf(":: %g\n", val);
				break;
			}
			case OP_CALL_NATIVE:
			{
				int handle = read_next_4();
				funcs[handle]();
				break;
			}
			case OP_CALL:
			{
				int address = read_next_4();	

				printf("address: %d\n", address);

				//add the stack frame (IP, BP)
				printf("IP:%d\n", instructionPointer);
				printf("BP:%d\n", varBP);

				push_in_var_stack_4(instructionPointer);
				push_in_var_stack_4(varBP);
				varBP = varBP + varSP;
				varSP = 0;
				//setup the jump

				instructionPointer = address;
				break;
			}
			case OP_RET:
			{
				int newBP = read_var_from_stack_4(varBP-4);
				int address = read_var_from_stack_4(varBP-8);
				printf("BP:%d", varBP);
				printf("NEWBP:%d", newBP);
				varSP = (varBP-8)-newBP;
				varBP = newBP;	
				printf("newBP:%d\nnewSP:%d\nnewAddress:%d\n",
						varBP,
						varSP,
						address);
				instructionPointer = address;	
				break;
			}
		}	
	}

}


int main(int argc, char** argv) {
	if(argc == 2) {
		run(argv[1], false);
	}
	else if(argc == 3) {
		if(strcmp(argv[2],"-v")==0)
		{
			run(argv[1], true);
		}
		else 
		{
			printf("1 arg: src file with bytecode\n");
			printf("2 arg: -v -- for verbose execution\n");
		}
	}
	else {
		printf("1 arg: src file with bytecode\n");
		printf("2 arg: -v -- for verbose execution\n");
	}
	return 0;
}
