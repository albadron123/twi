#include"../opcodes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// COMPILATION FLAGS
bool verbose;
bool traceStack;


////////////////////

char* codeSection;
int IP = 0;

char* stack;
int SP = 0;
int BP = 0;

char* heap;
const int HEAP_SIZE = 1024*1024*1024;
int heapPtr = 0;


//stack section manipulations

int alloc_memory_heap(int size) {
	*(double*)(heap+heapPtr) = (double)size;
	heapPtr += (size+1)*8;
	return heapPtr - (size)*8;
}

inline int get_heap(int pos) {
	return *(double*)(heap+pos);
}

inline void set_heap(int pos, double value) {
	*(double*)(heap+pos) = value;
}

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


void print_stack_trace() {
	printf("===STACK TRACE===\n");
	for(int i = 0; i < BP + SP; ++i)
	{
		printf("%04x ", (int)stack[i]);
	}
	printf("\n");
}


void run(const char* src) {
	if(!read_bytecode_from_file(src)) {
		printf("panic: coudn't read the bytecode\n");
		return;
	}

	stack = (char*)malloc(2*1024*1024*sizeof(char));
	heap = (char*)malloc(HEAP_SIZE*sizeof(char));
	IP = 0;
	SP = 0;
	BP = 0;
	heapPtr = 0;

	while(((char*)codeSection)[IP] != OP_HLT) {
		char op = ((char*)codeSection)[IP];
		if(traceStack) {	
			print_stack_trace();
		}
		if(verbose) {
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
				//this offset is the size in stack dedicated for local vars
				int offsetSize = read_next_4();

				double returnValue = pop_stack_8();

				int newBP = read_stack_4(BP-4);
				int address = read_stack_4(BP-8);
				
				SP = (BP-8)-newBP-offsetSize;
				BP = newBP;	
								
				IP = address;	
				
				push_stack_8(returnValue);

				break;
			}

			case OP_ALLOC:
			{
				double size = pop_stack_8();
				double address = alloc_memory_heap((int)size);	
				push_stack_8(address);
				break;
			}

			case OP_INDEX:
			{
				double base = pop_stack_8();
				double index = pop_stack_8();
				int address = (int)base + ((int)index)*8;
				double value = get_heap(address);	
				push_stack_8(value);
				break;
			}

			case OP_ASSIGN_HEAP:
			{
				double value = pop_stack_8();
				double index = pop_stack_8();
				double base = pop_stack_8();
				set_heap((int) base + ((int)index)*8, value);
				break;
			}
		}	
	}

}


int main(int argc, char** argv) {
	verbose = false;
	traceStack = false;
	if(argc == 2) {
		run(argv[1]);
	}
	else if (argc > 2) {
		for(int i = 2; i < argc; ++i) {
			if(strcmp(argv[i],"-v")==0) {	
				verbose = true;
			}
			else if (strcmp(argv[i], "-t")==0) {
				traceStack = true;	
			}
			else {
				printf("%d-th argument (%s) was incorrect.\n", i-1, argv[i]);
				printf("supported flags:\n");
				printf("-v -- verbose\n");
				printf("-t -- enable stack tracing\n");	
				return 0;
			}
		}
		run(argv[1]);
	}	
	else {
		printf("1 arg: src file with bytecode\n");
		printf("supported flags:\n");
		printf("-v -- verbose\n");
		printf("-t -- enable stack tracing\n");	
	}
	return 0;
}
