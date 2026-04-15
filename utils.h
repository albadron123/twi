#pragma once 

#include<string>
#include<map>
#include<vector>

enum TokenType {
	//One character tokens
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	COMMA,
	DOT,
	MINUS,
	PLUS,
	SEMICOLON,
	COLON,
	SLASH,
	STAR,
	//One or Two character tokens
	BANG,
	BANG_EQUAL,
	EQUAL,
	EQUAL_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,
	//Literals
	IDENTIFIER,
	STRING,
	NUMBER,
	//Keywords
	AND,
	CLASS,
	ELSE,
	FALSE,
	FUN,
	FOR,
	IF,
	NIL,
	OR,
	PRINT,
	RETURN,
	THIS,
	TRUE,
	VAR,
   	WHILE,
	//EOF
	_EOF
};

extern const char* tokenNames[];
extern std::map<std::string, TokenType> keywords;
extern std::map<TokenType, float> opPower;

struct Token {
	TokenType type;
	std::string content;
	//this is used only when we deal with numbers
	double value;
	int line;
};

enum ExprType {
	UNARY,
	BINARY,
	GROUPING,
	LITERAL,
	CALL,
	INDEX,
	LVALUE, //temporal fix for lvalues in funcs
	ERROR,
	EMPTY
};

enum StmtType {
	EXPR_STMT,
	PRINT_STMT,
	VAR_STMT,
	ARRAY_STMT,
	FUNCTION_STMT,
	GROUP_STMT,
	IF_STMT,
	WHILE_STMT,
	ERROR_STMT,
	EMPTY_STMT,
	RETURN_STMT,
};

struct Expr {
	ExprType type;
	Token* token;
	Expr* children[2];

	//for function call it is a pointer to the argument list
	int extraIndex;
};

struct Stmt {
	//General information
	StmtType type;

	Expr* exprs[1];
	Stmt* stmts[2];

	//Var stmt info
	Expr* lvalue;
	//Group stmt info
	std::vector<Stmt*> group;

	//for function params declaration
	int extraIndex;
};

enum ValueType {
	CALEE_VAL,
	STRING_VAL,
	NUMBER_VAL,
	BOOL_VAL,
	NIL_VAL,
	NOT_A_VALUE,
	NO_VALUE
};

struct EvalResult {
	std::string name;
	ValueType type;
	double value;
	std::string str_value;
};


struct Func {
	EvalResult (*func)(std::vector<EvalResult> args);
	int arity;
};

struct UserFunc {
	Stmt* body;
	int arity;
	int paramsId;
};

struct Environment {
	std::map<std::string, EvalResult> vars;
	std::map<std::string, UserFunc> funcs;
};

//we will store envs in a vector for now
extern std::vector<Environment> envs;

extern EvalResult returnResult;
extern bool isReturning;

extern std::vector<Token> tokens;

extern std::vector<Stmt*> globalStream;


extern std::vector<std::vector<Expr*>> functionArgs;
extern std::vector<std::vector<Token*>> functionParams;

extern std::map<std::string, Func> nativeFunctions;

// Utility functions 
std::string print_expr(Expr* expr); 
