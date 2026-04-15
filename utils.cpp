#include"utils.h"

#include<vector>
#include<map>

#include<time.h>

const char* tokenNames[] = {
	"(", 
	")", 
	"{", 
	"}", 
	"[",
	"]",
	",", 
	".",
	"-",
	"+",
	";",
	":",
	"/",
	"*",
	"!",
	"!=",
	"=",
	"==",
	">",
	">=",
	"<",
	"<=",
	"identifier",
	"string",
	"number",
	//keywords
	"and", "class", "else", "false", "fun", "for", "if", "nil", "or", "print", "return",
	"this", "true", "var", "while",
};

std::map<std::string, TokenType> keywords = {
	{"and", AND},
	{"class", CLASS},
	{"else", ELSE},
	{"false", FALSE},
	{"fun", FUN},
	{"for", FOR},
	{"if", IF},
	{"nil", NIL},
	{"or", OR},
	{"print", PRINT},
	{"return", RETURN},
	{"this", THIS},
	{"true", TRUE},
	{"var", VAR},
	{"while", WHILE},
};

std::map<TokenType, float> opPower = {
	{STAR, 100},
	{SLASH, 100},
	{PLUS, 90},
   	{MINUS, 90},
	{AND, 80},
	{OR, 70},
	{GREATER, 60},
	{GREATER_EQUAL, 60},
	{LESS, 60},
	{LESS_EQUAL, 60},
	{BANG_EQUAL, 50},
	{EQUAL_EQUAL, 50},
	{EQUAL, 40},
};

std::vector<Environment> envs;

//NATIVE FUNCTIONS

EvalResult twi_clock(std::vector<EvalResult> args) {
	EvalResult res = {"", NUMBER_VAL, 0, ""};
	res.value = ((double)clock()/(double)CLOCKS_PER_SEC);
	return res;
}

EvalResult twi_getfunc(std::vector<EvalResult> args) {
	return {"", CALEE_VAL, 0, "clock"};
}

EvalResult twi_int(std::vector<EvalResult> args) {
	if(args[0].type == NUMBER_VAL) {
		return {"", NUMBER_VAL, (double)(int)(args[0].value), ""};
	}
	return {"", NIL_VAL, 0, ""};
}

std::map<std::string, Func> nativeFunctions = {
	{"clock", {twi_clock, 0}},
	{"get_func", {twi_getfunc, 0}},
	{"int", {twi_int, 1}},
};
//END OF NATIVE FUNCTIONS




std::vector<Stmt*> globalStream;
std::vector<Token> tokens{};

std::vector<std::vector<Expr*>> functionArgs;
std::vector<std::vector<Token*>> functionParams;


EvalResult returnResult;
bool isReturning;

std::string print_expr(Expr* expr) {
	std::string res = "";
	switch(expr->type) {
		case CALL:
			{
				res = "(call " + print_expr(expr->children[0]) + " :: ";
				for(int i = 0; i < functionArgs[expr->extraIndex].size()-1; ++i) {
					res += print_expr(functionArgs[expr->extraIndex][i]) + ", ";
				}
				int lastIndex = functionArgs[expr->extraIndex].size()-1;
				res += print_expr(functionArgs[expr->extraIndex][lastIndex]) + ")";
			}
			break;
		case LITERAL:
			res = expr->token->content;
			break;
		case BINARY:
			res = "(" + std::string(tokenNames[expr->token->type]) + " " + 
				  print_expr(expr->children[0]) + " " + 
				  print_expr(expr->children[1]) + ")";
			break;
		case UNARY:
			res = "(" + std::string(tokenNames[expr->token->type]) + " " +
			   	 print_expr(expr->children[0]) + ")";	
			break;
		case GROUPING:
			res = "(group" + print_expr(expr->children[0]) + ")";
			break;
		case ERROR:
			res = "error token";
			break;
		case EMPTY:
			res = "empty";
			break;
	}
	return res;
}
