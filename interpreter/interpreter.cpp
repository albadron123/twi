#include"../opcodes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


char* codeSection;
int IP = 0;

char* stack;
int SP = 0;
int BP = 0;

//stack section manipulations

inline void push_stack_8(double d) {
	*((double*)(stack+BP+SP)) = d;
	SP += 8;
}

inline double pop_stack_8() {
	SP -= 8;
	return *((double*)(stack+BP+SP));
}

inline double peek_stack_8() {
	return *((double*)(stack+SP+BP-8));
}

inline void push_stack_4(int i) {
	*((int*)(stack+SP+BP)) = i;
	SP += 4;
}

inline double read_stack_8(int globalAddress) {
	return *((double*)(stack+globalAddress));
}

inline int read_stack_4(int globalAddress) {
	return *((int*)(stack+globalAddress));
}

inline void write_stack_8(int globalAddress, double d) {
	*((double*)(stack+globalAddress)) = d;
}

//code section maniputations

inline double read_next_8() {
	IP += 8;
	return *(double*)(codeSection+IP-8);
}

inline int read_next_4() {
	IP += 4;
	return *(int*)(codeSection+IP-4);
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
	push_stack_8(10);
}

void twi_int() {	
	printf(":: not implemented\n");
	push_stack_8(10);
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

	stack = (char*)malloc(2*1024*1024*sizeof(char));

	IP = 0;
	SP = 0;
	BP = 0;

	while(((char*)codeSection)[IP] != OP_HLT) {
		char op = ((char*)codeSection)[IP];
		if(runVerbose) {
			printf("%4d : %s\n", 
					IP,
					codeOpNames[(int)op]);
		}
		++IP;
		switch(op) {
			case OP_NO_OP:
			{
				printf("no op hit\n");
				return;
			}
			case OP_PUSH_VALUE:
			{
				double val = read_next_8();
				push_stack_8(val);
				break;
			}
			case OP_PUSH_VAR:
			{
				int localAddress = read_next_4();
				double val = read_stack_8(BP + localAddress);	
				push_stack_8(val);				
				break;
			}	
			case OP_ASSIGN:
			{
				int localAddress = read_next_4();
				double val = peek_stack_8();
				write_stack_8(BP + localAddress, val);	
				break;
			}	
			case OP_MOVE_SP:
			{
				int address = read_next_4();
				SP = BP + address;
				break;
			}
			case OP_ADD:
			{
				SP -= 8;
				*((double*)(stack+BP+SP-8)) +=
					*((double*)(stack+SP+BP));	
				break;
			}
			case OP_SUB:
			{
				SP -= 8;
				*((double*)(stack+BP+SP-8)) -=
					*((double*)(stack+SP+BP));	
				break;
			}
			case OP_MULT:
			{
				SP -= 8;
				*((double*)(stack+BP+SP-8)) *=
					*((double*)(stack+SP+BP));		
				break;
			}
			case OP_DIV:
			{
				SP -= 8;
				*((double*)(stack+BP+SP-8)) /=
					*((double*)(stack+SP+BP));	
				break;
			}
			case OP_NEG:
			{
				double a = pop_stack_8();
				push_stack_8(-a);
				break;
			}
			case OP_AND:
			{
				double a = pop_stack_8();
				double b = pop_stack_8();
				push_stack_8((double)((bool)(int)(a) && (bool)(int)(b)));
				break;
			}
			case OP_OR:
			{
				double a = pop_stack_8();	
				double b = pop_stack_8();
				push_stack_8((double)((bool)(int)(a) || (bool)(int)(b)));
				break;	
			}
			case OP_NOT:
			{
				double a = pop_stack_8();
				push_stack_8(!(bool)(int)a);
				break;
			}
			case OP_EQ:
			{
				double a = pop_stack_8();
				double b = pop_stack_8();
				push_stack_8((double)(a == b));
				break;
			}
			case OP_LT:
			{
				double b = pop_stack_8();
				double a = pop_stack_8();
				push_stack_8(a < b);
				break;
			}
			case OP_LE:
			{
				double b = pop_stack_8();
				double a = pop_stack_8();
				push_stack_8(a <= b);
				break;
			}
			/*
			case OP_DECL_VAR: 
			{
				double val = pop_from_op_stack_8();
				push_in_var_stack_8(val);
				break;
			}
			*/
			case OP_JMP_FALSE:
			{
				bool val = (bool)(int)pop_stack_8();
				if(!val) {
					int address = read_next_4();
					IP = address;
				}
				else
				{
					IP += 4;
				}
				break;
			}
			case OP_JMP:
			{
				int address = read_next_4();
				IP = address;
				break;
			}
			case OP_PRINT:
			{
				double val = pop_stack_8();
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

				push_stack_4(IP);
				push_stack_4(BP);

				BP = BP + SP;
				SP = 0;

				IP = address;
				break;
			}
			case OP_RET:
			{
				int newBP = read_stack_4(BP-4);
				int address = read_stack_4(BP-8);
				
				SP = (BP-8)-newBP;
				BP = newBP;	
								
				IP = address;	
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
