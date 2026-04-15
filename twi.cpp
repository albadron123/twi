/*
TODO LIST:
- start parsing while the scanner worked correctly
- show runtime error and crash if something went wrong
- add else clause
- add function calls 
- native functions
 */

#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<map>
#include<string>

#include"utils.h"
#include"scanner.h"
#include"parser.h"
#include"compiler.h"

//Temporal decls
void print_evaluation(EvalResult er);
EvalResult eval_expression(Expr* expr); 
bool eval_statement(Stmt* stmt);


std::string double_to_string(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.15g", value); // adjust precision if needed
    return std::string(buf);
}

EvalResult user_function_call(int functionParamsIndex, 
							  std::vector<EvalResult> results,
							  Stmt* body)
{
	envs.push_back({{},{}});
	if(functionParamsIndex != -1)
	{
		//add arguments as variables to the current env
		for(int i = 0; i < functionParams[functionParamsIndex].size(); ++i)
		{
			envs[envs.size()-1]
				.vars[functionParams[functionParamsIndex][i]->content] = results[i];
		}
	}
	eval_statement(body);
	envs.pop_back();
	if(isReturning == true)
	{	
		isReturning = false;
		return returnResult;
	}
	return {"", NIL_VAL, 0, ""};
}

//returns false if fails
bool eval_statement(Stmt* stmt) {
	switch(stmt->type) {
		case IF_STMT:
		{
			EvalResult conditionEval = eval_expression(stmt->exprs[0]);
			if(conditionEval.type == NUMBER_VAL || conditionEval.type == BOOL_VAL)
			{
				bool doExecuteBody = (bool)conditionEval.value;
				if(doExecuteBody) {
					eval_statement(stmt->stmts[0]);
				}
				else if (stmt->stmts[1] != 0) {
					eval_statement(stmt->stmts[1]);
				}
				return true;
			}
			else {
				printf("EXECUTION : can't evaluate condition as boolean\n");
				return false;
			}
		}
		case WHILE_STMT:
		{
			
			EvalResult conditionEval = eval_expression(stmt->exprs[0]);
			if(conditionEval.type == NUMBER_VAL || conditionEval.type == BOOL_VAL)
			{
				bool doExecuteBody = (bool)conditionEval.value;
				while(doExecuteBody) {
					eval_statement(stmt->stmts[0]);
					conditionEval = eval_expression(stmt->exprs[0]);
					doExecuteBody = (bool)conditionEval.value;
				}
				return true;
			}
			else {
				printf("EXECUTION : can't evaluate condition as boolean\n");
				return false;
			}

		}
		case RETURN_STMT:
		{
			if (stmt->exprs[0]->type == EMPTY)
			{
				returnResult = {"", NIL_VAL, 0, ""};
				isReturning = true;
				return true;
			}
			EvalResult res = eval_expression(stmt->exprs[0]);
			returnResult = res;
			isReturning = true;
			return true;
		}
		case GROUP_STMT:
		{
			envs.push_back({{}, {}});
			for(int i = 0; i < stmt->group.size(); ++i) {	
				bool doSucceed = eval_statement(stmt->group[i]);
				if(!doSucceed) {
					envs.pop_back();
					return false;
				}
				if(isReturning) {
					envs.pop_back();
					return true;
				}
			}
			envs.pop_back();
			return true;
		}
		case PRINT_STMT:
		{
			EvalResult res = eval_expression(stmt->exprs[0]);
			print_evaluation(res);
			return true;
		}
		case EXPR_STMT:
		{
			eval_expression(stmt->exprs[0]);
			return true;
		}
		case VAR_STMT:
		{
			EvalResult res = eval_expression(stmt->exprs[0]);
			res.name = stmt->lvalue->token->content;
			envs[envs.size()-1].vars[stmt->lvalue->token->content] = res;
			return true;
		}
		case FUNCTION_STMT:
		{
			std::string functionDescription = 
				"(func " + 
				stmt->lvalue->token->content +
				" decl :: ";
			if(stmt->extraIndex >= 0) {
				for(int i = 0; i < functionParams[stmt->extraIndex].size()-1; ++i) {
					functionDescription += 
						functionParams[stmt->extraIndex][i]->content + ", ";
				}
				int lastIndex = functionParams[stmt->extraIndex].size()-1;
				functionDescription += 
						functionParams[stmt->extraIndex][lastIndex]->content + ")";	
			}
			else {
				functionDescription += "no args)";
			}
			printf("%s\n", functionDescription.c_str());	
			envs[envs.size()-1].funcs[stmt->lvalue->token->content] = {
				stmt->stmts[0], 
				(stmt->extraIndex == -1)?
					0: 
					(int)functionParams[stmt->extraIndex].size(),
				stmt->extraIndex
			};
			return true;
		}
		default:
			return true;
	}

}


inline EvalResult EvalResult_True() {
	return {"", BOOL_VAL, 1, ""};
}

inline EvalResult EvalResult_False() {
	return {"", BOOL_VAL, 0, ""};
}

//eval only arithmetics
EvalResult eval_expression(Expr* expr) {

	if(expr->token != 0 && expr->token->type == NUMBER) {
		return {"", NUMBER_VAL, expr->token->value, ""};
	}
	else if (expr->token != 0 && expr->token->type == STRING) {
		return {"", STRING_VAL, 0, expr->token->content};
	}
	else if (expr->token != 0 && expr->token->type == TRUE) {
		return EvalResult_True();
	}
	else if (expr->token != 0 && expr->token->type == FALSE) {
		return EvalResult_False();
	}
	else if (expr->token != 0 && expr->token->type == NIL) {
		return {"", NIL_VAL, 0, ""};
	}
	else if (expr->token != 0 && expr->token->type == IDENTIFIER) {
		int envId = envs.size()-1;
		// search for variable name
		while(envId >= 0)
		{
			
			if(envs[envId].vars.find(expr->token->content) != 
			   envs[envId].vars.end()) {
				return envs[envId].vars[expr->token->content];
			}
			if(envs[envId].funcs.find(expr->token->content) != 
			   envs[envId].funcs.end()) {
				return {"", CALEE_VAL, (double)envId, expr->token->content};
			}
			--envId;
		}
		if(nativeFunctions.find(expr->token->content) != nativeFunctions.end()) {
			return {"", CALEE_VAL, -1, expr->token->content};	
		}
		printf("found unassigned identifier: %s\n", expr->token->content.c_str());
		return {"", NOT_A_VALUE, 0, ""};
	}
	else if (expr->type == GROUPING) {
		return eval_expression(expr->children[0]);
	}
	else if (expr->type == UNARY) {
		EvalResult res = eval_expression(expr->children[0]);
		if(expr->token->type == BANG) {
			if(res.type == NUMBER_VAL)
			{
				res.type = BOOL_VAL;
				res.value = !res.value;
			}
			else if (res.type == BOOL_VAL) {
				res.value = !res.value;
			}
			else {
				res = {"", NOT_A_VALUE, 0, ""};
			}
			return res;
		}
		else {
			if(res.type == NUMBER_VAL)
			{
				res.value = -res.value;
			}
			else if (res.value == BOOL_VAL) {
				res.type = NUMBER_VAL;
				res.value = -res.value;
			}
			else {
				res = {"", NOT_A_VALUE, 0, ""};
			}
			return res;
		}
	}
	else if(expr->type == CALL) {
		EvalResult childRes = eval_expression(expr->children[0]);
		if(childRes.type == CALEE_VAL) {
			//this means that we've passed an environment of the 'thing'
			//in the value
			if(childRes.value != -1) {
				if(childRes.value < envs.size() && 
				   envs[(int)childRes.value].funcs.find(childRes.str_value) !=
				   envs[(int)childRes.value].funcs.end()) {
					int arity = envs[(int)childRes.value]
									.funcs[childRes.str_value]
									.arity;
					int provided;
					if(expr->extraIndex != -1) {	
						provided = functionArgs[expr->extraIndex].size();
					}
					else {
						provided = 0;
					}
					//check for arity
					if(arity != provided) {
						printf("you provided %d arguments to %s,but %d was expected\n",
								provided, 
								childRes.str_value.c_str(),
								arity);
						return {"", NOT_A_VALUE, 0, ""};
					}
					//evaluate all the arguments
					if(arity > 0) {
						std::vector<EvalResult> evalArgs = {};
						for(int i = 0; i < provided; ++i) {
							evalArgs.push_back(
								eval_expression(functionArgs[expr->extraIndex][i]));
						}
						
						EvalResult functionReturn = user_function_call(
								envs[(int)childRes.value]
									.funcs[childRes.str_value]
									.paramsId,
								evalArgs,
								envs[(int)childRes.value]
									.funcs[childRes.str_value]
									.body);
						return functionReturn;
					}	
					EvalResult functionReturn = user_function_call(
								-1,
								{},
								envs[(int)childRes.value]
									.funcs[childRes.str_value]
									.body);
					return functionReturn;
				}
				else {
					printf("function not found!!! error\n");
					return {"", NOT_A_VALUE, 0, ""};
				}

			}
			else if(nativeFunctions.find(childRes.str_value) !=
			   nativeFunctions.end()) {
				int arity = nativeFunctions[childRes.str_value].arity;
				int provided;
				if(expr->extraIndex != -1) {	
					provided = functionArgs[expr->extraIndex].size();
				}
				else {
					provided = 0;
				}
				//check for arity
				if(arity != provided) {
					printf("you provided %d arguments to %s,but %d was expected\n",
							provided, 
							childRes.str_value.c_str(),
							arity);
					return {"", NOT_A_VALUE, 0, ""};
				}
				//evaluate all the arguments
				if(arity > 0) {
					std::vector<EvalResult> evalArgs = {};
					for(int i = 0; i < provided; ++i) {
						evalArgs.push_back(
							eval_expression(functionArgs[expr->extraIndex][i]));
					}
					
					EvalResult functionReturn = 
						nativeFunctions[childRes.str_value].func(evalArgs);
					return functionReturn;
				}
				else 
				{
					return nativeFunctions[childRes.str_value].func({});
				}
			}
			else {
				printf("function not found!!! error\n");
				return {"", NOT_A_VALUE, 0, ""};
			}
		}
		else {
			printf("you try to call something that cant be called\n");
			return {"", NOT_A_VALUE, 0, ""};	
		}
	}
	else if(expr->type == BINARY) {
		EvalResult a = eval_expression(expr->children[0]);
		EvalResult b = eval_expression(expr->children[1]);
		switch(expr->token->type) {
			case PLUS:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL)) {
					return {"", NUMBER_VAL, a.value+b.value, ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {"", STRING_VAL, 0, a.str_value + b.str_value};
				}	
				else if (a.type == STRING_VAL && b.type == NUMBER_VAL) {
					return {"", STRING_VAL, 0, a.str_value + double_to_string(b.value)};
				}
				else if (b.type == STRING_VAL && a.type == NUMBER_VAL) {
					return {"", STRING_VAL, 0, double_to_string(a.value) + b.str_value};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case MINUS:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL))
				{
					return {"", NUMBER_VAL, a.value-b.value, ""};
				}
				else
				{
					return {"", NOT_A_VALUE, 0, ""};
				}	
			case STAR:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL))
				{
					return {"", NUMBER_VAL, a.value*b.value, ""};
				}
				else
				{
					return {"", NOT_A_VALUE, 0, ""};
				}	
			case SLASH:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL)) {
					return {"", NUMBER_VAL, a.value/b.value, ""};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case GREATER:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value>b.value), ""};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case GREATER_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value>=b.value), ""};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case LESS:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value<b.value), ""};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case LESS_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value<=b.value), ""};
				}
				else {
					return {"", NOT_A_VALUE, 0, ""};
				}
			case EQUAL_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value==b.value), ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.str_value==b.str_value), ""};
				}
				else if (a.type == BOOL_VAL && b.type == BOOL_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value==b.value), ""};
				}
				else {
					return EvalResult_False();
				}
			case BANG_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value!=b.value), ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.str_value!=b.str_value), ""};
				}
				else if (a.type == BOOL_VAL && b.type == BOOL_VAL) {
					return {"", BOOL_VAL, (double)(int)(a.value!=b.value), ""};
				}
				else if (a.type == NIL_VAL || b.type == NIL_VAL) {
					return EvalResult_False();
				}
				else {
					return EvalResult_True();
				}
			/*
			 * and / or are short circuit
			 * first we try to evaluate for the lhs 
			 * if it is convertable to bool we then eval it
			 * based on that we decide if we need to convert to bool or 
			 * evaluate the rhs.
			 */
			case AND:
				if(a.type == NUMBER_VAL || a.type == BOOL_VAL)
				{
					if((bool)a.value == false) {
						return EvalResult_False();
					}
					if(b.type == NUMBER_VAL || b.type == BOOL_VAL) {
						if((bool)b.value == false) {
							return EvalResult_False();
						}
						else {
							return EvalResult_True();	
						}
					}
					else {
						printf("EXECUTION: can't convert rhs of 'and' to bool\n");
						return {"", NOT_A_VALUE, 0, ""};
					}
				}
				else 
				{
					printf("EXECUTION: can't convert lhs of 'and' to bool\n");
					return {"", NOT_A_VALUE, 0, ""}; 
				}
			case OR:
				if(a.type == NUMBER_VAL || a.type == BOOL_VAL)
				{
					if((bool)a.value == true) {
						return EvalResult_True();
					}
					if(b.type == NUMBER_VAL || b.type == BOOL_VAL) {
						if((bool)b.value == true) {
							return EvalResult_True();
						}
						else {
							return EvalResult_False();	
						}
					}
					else {
						printf("EXECUTION: can't convert rhs of 'or' to bool\n");
						return {"", NOT_A_VALUE, 0, ""};
					}
				}
				else 
				{
					printf("EXECUTION: can't convert lhs of 'or' to bool\n");
					return {"", NOT_A_VALUE, 0, ""}; 
				}
			case EQUAL:
			{
				if(a.name == "") {
					printf("can't assign to rvalue\n");
					return {"", NOT_A_VALUE, 0, ""};
				}
				// we will assign the right side with the left side name to the left
				// var
				b.name = a.name;
				int envId = envs.size()-1;
				while(envId >= 0) {
					
					if(envs[envId].vars.find(a.name) != 
					   envs[envId].vars.end()) {
						envs[envId].vars[a.name] = b;
						return b;
					}
					--envId;
				}
				//Unreachable
				printf("unreachable area: should be the bug in logic of compiler\n");
				return {"", NOT_A_VALUE, 0, ""};
			}
			default: 
				printf("some binary operation is not supported\n");
				return {"", NOT_A_VALUE, 0, ""};
		}
	}	
	else if (expr->type == EMPTY) {
		return {"", NO_VALUE, 0, ""}; 
	}
	else 
	{
		// should be unreachable
		return  {"", NOT_A_VALUE, 0, ""};	
	}

	printf("Shouldnt reach here\n");
	return {"", NOT_A_VALUE, 0, ""};
}


void print_evaluation(EvalResult er)
{
	switch(er.type) {
		case NUMBER_VAL:
			printf("::%g\n", er.value);
			break;
		case STRING_VAL:
			printf("::%s\n", er.str_value.c_str());
			break;
		case BOOL_VAL:
			if(er.value == 0) {
				printf("::false\n");
			}
			else {
				printf("::true\n");
			}
			break;
		case NIL_VAL:
			printf("::nil\n");
			break;
		case NO_VALUE:
			printf("::*nothing*\n");
			break;
		case NOT_A_VALUE:
			printf("::*not a value*\n");
			break;
		default:
			printf("::*output unemplemented*\n");
	}	
}



void run(const char* src, bool fromRepl) {	
	//scan tokens 
	tokens.clear();
	scan(src, tokens); 

#ifdef VERBOSE
	//print info about scanned tokens:
	printf("LINE  %-15s  %s\n", "TOKEN", "VALUE");	
	for(Token tk : tokens) {
		if(tk.type == NUMBER) {
			printf("%-4d::%-15s::%s::%lf\n",
				   tk.line,
				   tokenNames[tk.type], 
				   tk.content.c_str(),
				   tk.value);
		}
		else {
			printf("%-4d::%-15s::%s\n",
				   tk.line, 
				   tokenNames[tk.type], 
				   tk.content.c_str());
		}
	}
#endif


	int stmtsCount = parse_all();

	if(stmtsCount > 0) {
		if(!fromRepl) {
			//create an execution environment
			envs.push_back({{},{}});
		}
		else {
			// in repl mode we should create an execution environment only once.
			// after we've executed the first line we want the repl to remember the
			// variables that we created before
		 	if(envs.size() == 0) {
				envs.push_back({{},{}});
			}		
		}

		for(int i = 0; i < globalStream.size(); ++i) {	
			eval_statement(globalStream[i]);
		}
	}
}

bool init_compiler = false;
void run_compile(const char* src) {	
	//scan tokens 
	tokens.clear();
	scan(src, tokens); 

#ifdef VERBOSE
	//print info about scanned tokens:
	printf("LINE  %-15s  %s\n", "TOKEN", "VALUE");	
	for(Token tk : tokens) {
		if(tk.type == NUMBER) {
			printf("%-4d::%-15s::%s::%lf\n",
				   tk.line,
				   tokenNames[tk.type], 
				   tk.content.c_str(),
				   tk.value);
		}
		else {
			printf("%-4d::%-15s::%s\n",
				   tk.line, 
				   tokenNames[tk.type], 
				   tk.content.c_str());
		}
	}
#endif
	int stmtsCount = parse_all();



	if(!init_compiler) {
		init_compilation();
	}
	codeSectionSize = 0;

	bool success = prepass();
	if(!success) {
		return;
	}

	for(int i = 0; i < globalStream.size(); ++i) {
		compile_stmt(globalStream[i]);
	}
	compile_stmt(nullptr);
	disassemble();	
	write_bytecode();
}

int read_file(const char* filename, char** buffer) {
	FILE* file = fopen(filename, "r");
	if(file == NULL) {
		printf("Source file doesn't exit!\n");
		return -1;
	}

	//getting the file size
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);

	//allocating the buffers
	*buffer = (char*)malloc(size);

	//copy the program into the buffer and clear the memory part
	fread(*buffer, sizeof(char), size, file);

	fclose(file);	

	return size;
}

void work_from_file(const char* filename) {
	char* buf;
	int bufSize = read_file(filename, &buf);	
	if(bufSize<0) {
		return;
	}
#ifdef VERBOSE
	printf("BUFFER CONTENTS:\n%s", buf);
#endif
	run_compile(buf);
}

void work_from_repl() {
	char line[200];
	while(true) {	
		printf(">");
		fflush(stdout);
		if(scanf("%199[^\n]", line)) {
			getchar();
#ifdef VERBOSE
			printf("%s\n", line);
#endif
			run_compile(line);
			//run(line, true);
		}
		else {
			break;	
		}
	}
}

int main(int argc, char** argv) {
	if(argc==1) {
		work_from_repl();	
	} 
	else if(argc==2) {
		work_from_file(argv[1]);
	} 
	else {
		printf("Call without agruments to work with REPL or provide a file\n");
	}
	return 0;
}
