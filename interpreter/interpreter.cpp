#include"../opcodes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


char* codeSection;

char* varStackSection;
char* opStack;

int instructionPointer = 0;
int opStackPointer = 0;
int varStackTop = 0;



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
	*((double*)(varStackSection+varStackTop)) = d;
	varStackTop += 8;
}

inline double read_next_8() {
	instructionPointer += 8;
	return *(double*)(codeSection+instructionPointer-8);
}

inline int read_next_4() {
	instructionPointer += 4;
	return *(int*)(codeSection+instructionPointer-4);
}

inline double read_var_from_stack_8(int address) {
	return *((double*)(varStackSection+address));
}

inline void write_var_on_stack_8(int address, double d) {
	*((double*)(varStackSection+address)) = d;
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
	varStackTop = 0;

	while(((char*)codeSection)[instructionPointer] != OP_HLT) {
		char op = ((char*)codeSection)[instructionPointer];
		if(runVerbose) {
			printf("%4d : %s\n", 
					instructionPointer,
					codeOpNames[(int)op]);
		}
		++instructionPointer;
		switch(op) {
			case OP_PUSH_VALUE:
			{
				double val = read_next_8();
				push_in_op_stack_8(val);
				break;
			}
			case OP_PUSH_VAR:
			{
				int address = read_next_4();
				double val = read_var_from_stack_8(address);	
				push_in_op_stack_8(val);				
				break;
			}
			case OP_ASSIGN:
			{
				int address = read_next_4();
				double val = peek_op_stack_8();
				write_var_on_stack_8(address, val);	
				break;
			}
			case OP_MOVE_VAR_SP:
			{
				int address = read_next_4();
				varStackTop = address;
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
