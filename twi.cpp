/*
TODO LIST:
- start parsing while the scanner worked correctly
- show runtime error if something went wrong
- decompose into several files 
	(scanner.cpp & later add parser.cpp & interpreter.cpp)

- add statements (print and declare)
- add environments of variables
- add = support in expression evaluation
- add scoping support for statement evaluation
 */

#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<map>
#include<string>

inline bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool is_digit(char c) {
	return (c >= '0' && c <= '9');
}

enum TokenType {
	//One character tokens
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	DOT,
	MINUS,
	PLUS,
	SEMICOLON,
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

const char* tokenNames[] = {
	"(", 
	")", 
	"{", 
	"}", 
	",", 
	".",
	"-",
	"+",
	";",
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


std::map<TokenType, float> opPower = {
	{PLUS, 30},
   	{MINUS, 30},
	{STAR, 40},
	{SLASH, 40},
	{GREATER, 20},
	{GREATER_EQUAL, 20},
	{LESS, 20},
	{LESS_EQUAL, 20},
	{BANG_EQUAL, 10},
	{EQUAL_EQUAL, 10},
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
	ERROR,
	EMPTY
};

enum StmtType {
	EXPR_STMT,
	PRINT_STMT,
	VAR_STMT,
	GROUP_STMT,
};

struct Expr {
	ExprType type;
	Token* token;
	Expr* children[2];
};

struct Stmt {
	//General information
	StmtType type;

	Expr* expr[1];

	//Var stmt info
	Expr* lvalue;
	//Group stmt info
	std::vector<Stmt> group;
};


int currentToken = 0;
bool doPanic = false;
Expr* exprs = nullptr;
int exprsCount = 0;


enum ValueType {
	STRING_VAL,
	NUMBER_VAL,
	BOOL_VAL,
	NIL_VAL,
	NOT_A_VALUE,
	NO_VALUE
};
struct EvalResult {
	ValueType type;
	double value;
	std::string str_value;
};

struct Environment {
	std::map<std::string, ValueType> vars;
}
//we will store envs in a vector for now
std::vector<Environment>;

std::vector<Token> tokens;

std::string print_expr(Expr* expr) {
	std::string res = "";
	switch(expr->type) {
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
		case EMPTY:
			res = "empty";
	}
	return res;
}

bool scan(const char * src, std::vector<Token>& out) {
	bool hasErrors = false;
	const char* p = src;
	int line = 0;
	while(*p != '\0') {
		switch(*p) {	
			case ' ':
			case '\t':
			case '\r':
				break;
			case '\n':
				++line;
				break;
			// one char tokens
			case '(':
				out.push_back({LEFT_PAREN, "", 0, line});
				break;	
			case ')':
				out.push_back({RIGHT_PAREN, "", 0, line});
				break;
			case '{':
				out.push_back({LEFT_BRACE, "", 0, line});
				break;
			case '}':
				out.push_back({RIGHT_BRACE, "", 0, line});
				break;
			case '.':
				out.push_back({DOT, "", 0, line});
				break;
			case ',':
				out.push_back({COMMA, "", 0, line});
				break;
			case ';':
				out.push_back({SEMICOLON, "", 0, line});
				break;
			case '+':
				out.push_back({PLUS, "", 0, line});
				break;
			case '-':
				out.push_back({MINUS, "", 0, line});
				break;
			case '*':
				out.push_back({STAR, "", 0, line});
				break;
			case '/':
				//one line comment case
				if(*(p+1) == '/') {
					p+=2;
					while(*p != '\0' && *p != '\n')
						++p;
					--p;
				}
				//multiple lines comment case
				else if (*(p+1) == '*') {
					p+=2;
					while(!((*p == '\0') || 
						    (*(p+1) == '\0') || 
							(*p == '*' && *(p+1) == '/'))) {
						++p;
					}
					if(*p == '\0') --p;
					if(*p == '*') ++p;
				}
				else { 
				//slash case
				out.push_back({SLASH, "", 0, line});
				}
				break;
			//one or two char tokens
			case '!':
				if(*(p+1) == '=') {
					++p;
					out.push_back({BANG_EQUAL, "", 0, line});
				}
				else {
					out.push_back({BANG, "", 0, line});
				}
				break;
			case '=':
				if(*(p+1) == '=') {
					++p;
					out.push_back({EQUAL_EQUAL, "", 0, line});
				}
				else {
					out.push_back({EQUAL, "", 0, line});
				}
				break;	
			case '>':	
				if(*(p+1) == '=') {
					++p;
					out.push_back({GREATER_EQUAL, "", 0, line});
				}
				else {
					out.push_back({GREATER, "", 0, line});
				}
				break;
			case '<':
				if(*(p+1) == '=') {
					++p;
					out.push_back({LESS_EQUAL, "", 0, line});
				}
				else {
					out.push_back({LESS, "", 0, line});
				}
				break;
			//Literals
			case '"':
				{
					const char* start = p;
					do {
						++p;
					} while (!(*p == '"' || *p == '\0' || *p == '\n'));
					if(*p == '\0' || *p == '\n')
					{
						printf("LINE %d: String does not terminate\n", 
							   line);
						hasErrors = true;
					}	
					else
					{
						out.push_back({STRING, "", 0, line});
						int last_id = out.size()-1;
						out[last_id].content.assign(start+1, p-start-1);
					}

					if(*p == '\0') --p;
				}
				break;
			default:
				if(is_alpha(*p) || *p == '_') {
					const char* start = p;
					do {
						++p;
					} while(is_alpha(*p) || *p == '_' || is_digit(*p));
					out.push_back({IDENTIFIER, "", 0, line});
					int last_id = out.size()-1;
					out[last_id].content.assign(start, p-start);
					if(keywords.find(out[last_id].content) != keywords.end()) {
						out[last_id].type = keywords[out[last_id].content];
					}
					--p;
				}
				else if(is_digit(*p)) {
					const char * start = p;
					do {
						++p;
					} while(is_digit(*p));
					if (*(p) == '.' && is_digit(*(p+1))) {
						++p;	
						do {
							++p;
						} while(is_digit(*p));
					}		
					out.push_back({NUMBER, "", 0, line});
						int last_id = out.size()-1;
						out[last_id].content.assign(start, p-start);
						out[last_id].value = std::stod(out[last_id].content);
					--p;

				}
				else {
					printf("LINE %d: Unexpected character %c\n",line, *p);
					hasErrors = true;
				}
		}
		++p;	
	}
	return hasErrors;
}


std::string double_to_string(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.15g", value); // adjust precision if needed
    return std::string(buf);
}

//eval only arithmetics
EvalResult eval_expression(Expr* expr) {
	if(expr->token != 0 && expr->token->type == NUMBER) {
		return {NUMBER_VAL, expr->token->value, ""};
	}
	else if (expr->token != 0 && expr->token->type == STRING) {
		return {STRING_VAL, 0, expr->token->content};
	}
	else if (expr->token != 0 && expr->token->type == TRUE) {
		return {BOOL_VAL, 1, ""};
	}
	else if (expr->token != 0 && expr->token->type == FALSE) {
		return {BOOL_VAL, 0, ""};
	}
	else if (expr->token != 0 && expr->token->type == NIL) {
		return {NIL_VAL, 0, ""};
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
				res = {NOT_A_VALUE, 0, ""};
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
				res = {NOT_A_VALUE, 0, ""};
			}
			return res;
		}
	}
	else if(expr->type == BINARY) {
		EvalResult a = eval_expression(expr->children[0]);
		EvalResult b = eval_expression(expr->children[1]);
		switch(expr->token->type) {
			case PLUS:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL)) {
					return {NUMBER_VAL, a.value+b.value, ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {STRING_VAL, 0, a.str_value + b.str_value};
				}	
				else if (a.type == STRING_VAL && b.type == NUMBER_VAL) {
					return {STRING_VAL, 0, a.str_value + double_to_string(b.value)};
				}
				else if (b.type == STRING_VAL && a.type == NUMBER_VAL) {
					return {STRING_VAL, 0, double_to_string(a.value) + b.str_value};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case MINUS:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL))
				{
					return {NUMBER_VAL, a.value-b.value, ""};
				}
				else
				{
					return {NOT_A_VALUE, 0, ""};
				}	
			case STAR:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL))
				{
					return {NUMBER_VAL, a.value*b.value, ""};
				}
				else
				{
					return {NOT_A_VALUE, 0, ""};
				}	
			case SLASH:
				if((a.type == NUMBER_VAL || a.type == BOOL_VAL) && 
				   (b.type == NUMBER_VAL || b.type == BOOL_VAL)) {
					return {NUMBER_VAL, a.value/b.value, ""};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case GREATER:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value>b.value), ""};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case GREATER_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value>=b.value), ""};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case LESS:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value<b.value), ""};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case LESS_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value<=b.value), ""};
				}
				else {
					return {NOT_A_VALUE, 0, ""};
				}
			case EQUAL_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value==b.value), ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {BOOL_VAL, (double)(int)(a.str_value==b.str_value), ""};
				}
				else if (a.type == BOOL_VAL && b.type == BOOL_VAL) {
					return {BOOL_VAL, (double)(int)(a.value==b.value), ""};
				}
				else {
					return {BOOL_VAL, 0, ""};
				}
			case BANG_EQUAL:
				if(a.type == NUMBER_VAL && b.type == NUMBER_VAL) {
					return {BOOL_VAL, (double)(int)(a.value!=b.value), ""};
				}
				else if (a.type == STRING_VAL && b.type == STRING_VAL) {
					return {BOOL_VAL, (double)(int)(a.str_value!=b.str_value), ""};
				}
				else if (a.type == BOOL_VAL && b.type == BOOL_VAL) {
					return {BOOL_VAL, (double)(int)(a.value!=b.value), ""};
				}
				else if (a.type == NIL_VAL || b.type == NIL_VAL) {
					return {BOOL_VAL, 0, ""};
				}
				else {
					return {BOOL_VAL, 1, ""};
				}
			default: 
				printf("some binary operation is not supported\n");
				return {NOT_A_VALUE, 0, ""};
		}
	}	
	else if (expr->type == EMPTY) {
		return {NO_VALUE, 0, ""}; 
	}
	else 
	{
		// should be unreachable
		return  {NOT_A_VALUE, 0, ""};	
	}
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

void synchronize_expression_parser()
{
	while(currentToken != tokens.size() && 
		  tokens[currentToken].type != SEMICOLON) {
		++currentToken;
	}
}

// #define VERBOSE
Expr* parse_expression(float power)
{
	Expr* lhs;
	//parse current token 
	if(currentToken == tokens.size() || 
	   tokens[currentToken].type == SEMICOLON) {
		exprs[exprsCount] = {EMPTY, 0, {0,0}};
		lhs = exprs+exprsCount;
		++exprsCount;
		return lhs;
	}
	else if(tokens[currentToken].type == IDENTIFIER || 
	   		tokens[currentToken].type == NUMBER ||
	   		tokens[currentToken].type == STRING ||
			tokens[currentToken].type == TRUE ||
			tokens[currentToken].type == FALSE ||
			tokens[currentToken].type == NIL) {
		exprs[exprsCount] = {LITERAL, &tokens[currentToken], {0,0}};
		lhs = exprs+exprsCount;
		++exprsCount;
#ifdef VERBOSE
		std::string res = print_expr(lhs);
		printf("IN %d EXPRESSION: %s\n", currentToken, res.c_str());	
#endif
	}
	else if (tokens[currentToken].type == MINUS ||
			 tokens[currentToken].type == BANG) {
		//add a unary operator here
		exprs[exprsCount] = {UNARY, &tokens[currentToken], {0,0}};
		lhs = exprs + exprsCount;
		++exprsCount;
		++currentToken;
		Expr* insideOfUnary = parse_expression(1000);
		if(doPanic) 
			return insideOfUnary;
		if(insideOfUnary->type == EMPTY) 
		{
			printf("PARSE: expected a token after unary [%s] but string terminated\n",
					tokenNames[lhs->token->type]);
			doPanic = true;
			//we are in sync here as we actually reached the end of
			//expression unexpectedly
			return insideOfUnary;
		}

		lhs->children[0] = insideOfUnary;
#ifdef VERBOSE
		std::string res = print_expr(lhs);
		printf("UN %d EXPRESSION: %s\n", currentToken, res.c_str());
#endif
		--currentToken;

	}
	else if (tokens[currentToken].type == LEFT_PAREN) {
		//add a grouping expr
		exprs[exprsCount] = {GROUPING, 0, {0,0}};
		lhs = exprs + exprsCount;
		++exprsCount;
		++currentToken;
		Expr* insideOfGrouping = parse_expression(-999);
		if(doPanic) 
			return insideOfGrouping;
		if(insideOfGrouping->type == EMPTY) 
		{
			printf("PARSE: expected a token after [(] but stream terminated\n");
			doPanic = true;
			//we are in sync here as we actually reached the end of
			//expression unexpectedly
			return insideOfGrouping;
		}

		if(tokens[currentToken].type == RIGHT_PAREN) {
			//we are cool
			lhs->children[0] = insideOfGrouping;
#ifdef VERBOSE
			std::string res = print_expr(lhs);
			printf("GR %d EXPRESSION: %s\n", currentToken, res.c_str());
#endif
		} 
		else {
			printf("PARSE: expected ) after token [%s] of type %s in line %d\n",
			   tokens[currentToken-1].content.c_str(),
			   tokenNames[tokens[currentToken-1].type],
			   tokens[currentToken-1].line);	

			exprs[exprsCount] = {ERROR, &tokens[currentToken-1], {0,0}};
			Expr* panicRes = exprs + exprsCount;	
			++exprsCount;
			doPanic = true;
			synchronize_expression_parser();
			return panicRes;
		}
	}
	else {
		printf("PARSE: unexpected token [%s] of type %s in line %d\n",
			   tokens[currentToken].content.c_str(),
			   tokenNames[tokens[currentToken].type],
			   tokens[currentToken].line);

		exprs[exprsCount] = {ERROR, &tokens[currentToken-1], {0,0}};
		Expr* panicRes = exprs + exprsCount;	
		++exprsCount;
		doPanic = true;
		synchronize_expression_parser();
		return panicRes;			
	}


	++currentToken;
	

	while(currentToken < tokens.size() && tokens[currentToken].type != SEMICOLON)
	{	
		//parse operation 
		if(opPower.find(tokens[currentToken].type) == opPower.end())  
		{	
#ifdef VERBOSE
			printf("DEBUG NOTE: can't parse an operation: [%s]\n",
				   tokenNames[tokens[currentToken].type]);
#endif
			break;
		}
		else
		{
			if(opPower[tokens[currentToken].type] >= power)
			{
				exprs[exprsCount] = {BINARY, &tokens[currentToken], {lhs, 0}};
				lhs = exprs+exprsCount;
				++exprsCount;
				++currentToken;
				// we add +1 to adject to the left associativity (otherwise the op
				// will be right-associative
				Expr* rhs = parse_expression(opPower[tokens[currentToken-1].type]+1);
				if(doPanic) {
					//just return the error partat this point,
					//think, that it's just not correct
					//but we will never use this tree again so it doesn't really matter
					//at this point
					return rhs;
				}
				if(rhs->type == EMPTY) 
				{
					printf("PARSE: expected a token but stream terminated\n");
					doPanic = true;
					//we are in sync here as we actually reached the end of
					//expression unexpectedly
					return rhs;
				}	
				lhs->children[1] = rhs;
#ifdef VERBOSE	
				std::string res = print_expr(lhs);
				printf("OUT %d EXPRESSION: %s\n", currentToken, res.c_str());
#endif

			}
			else
			{
				break;
			}
		}
	}
	return lhs;
}


void run(const char* src) {
	if(exprs == nullptr)
	{
		exprs = (Expr*)malloc(sizeof(Expr) * 1000);
	}

	//scan tokens 
	tokens.clear();
	scan(src, tokens); 
	doPanic = false;
	exprsCount = 0;	
	currentToken = 0;

	//print info about scanned tokens:
	
#ifdef VERBOSE
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

	int expressionNumber = 0;
	while(currentToken < tokens.size())
	{
		//parse and eval if correct
		Expr* e = parse_expression(-999);
		if(doPanic) {
			// mistake is found and announced 
			doPanic = false;
			printf(":: panic at expression %d\n", expressionNumber);	
			//we go to the first non-semicolon token or over the end of file
			++currentToken;
		}
		else if(currentToken != tokens.size() && 
				tokens[currentToken].type != SEMICOLON) {
			synchronize_expression_parser();
			printf(":: panic at expression %d\n", expressionNumber);
			//we go to the first non-semicolon token or over the end of file
			++currentToken;
		}
		else {
			std::string res = print_expr(e);
#ifdef VERBOSE
			printf("EXPRESSION: %s\n", res.c_str());
#endif
			print_evaluation(eval_expression(e));

			++currentToken;
		}
		++expressionNumber;
		// we build a new expression tree at this point
		exprsCount = 0;
	}
}

int read_file(const char* filename, char** buffer) {
	FILE* file = fopen(filename, "r");
	if(file == NULL)
	{
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
	run(buf);
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
			run(line);
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
