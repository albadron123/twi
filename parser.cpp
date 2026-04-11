#include"utils.h"
#include"parser.h"

int currentToken = 0;
bool doPanic = false;
Expr* exprs = nullptr;
Stmt* stmts = nullptr;
int exprsCount = 0;
int stmtsCount = 0;


inline bool is_end_of_stmt() {
	return (tokens[currentToken].type == SEMICOLON ||
	   	    tokens[currentToken].type == _EOF);
}

int parse_all() {

	if(exprs == nullptr) {
		exprs = (Expr*)malloc(sizeof(Expr) * 1000);
	}
	if(stmts == nullptr) {
		stmts = (Stmt*)malloc(sizeof(Stmt) * 1000);
	}
	globalStream.clear();

	doPanic = false;
	exprsCount = 0;	
	stmtsCount = 0;
	currentToken = 0;
	
	bool parsedSuccessfully = true;
	int stmtCount = 0;
	while(currentToken < tokens.size())
	{
		Stmt* s = parse_statement(true);
		if(s->type == ERROR_STMT) {
			parsedSuccessfully = false;
		}
		else {
			globalStream.push_back(s);
		}

		doPanic = false;
		++stmtCount;	
	}

	if(!parsedSuccessfully) {
		return -1;
	}
	return stmtCount;
}

void synchronize_parser()
{
	while(tokens[currentToken].type != _EOF && 
		  tokens[currentToken].type != SEMICOLON &&
		  tokens[currentToken].type != FUN) {
		++currentToken;
	}
	if(tokens[currentToken].type == FUN) {
		--currentToken;
	}
}


Stmt* add_empty_stmt() {
	stmts[stmtsCount] = (Stmt){ERROR_STMT, {0}, {0}, 0, {}};
	++stmtsCount;
	return stmts+stmtsCount-1;
}


Stmt* parse_statement(bool allowDecls) {
	Stmt* thisStmt = add_empty_stmt();
	if(is_end_of_stmt())
	{
		thisStmt->type = EMPTY_STMT;
		++currentToken;
		return thisStmt;
	}	
	if(tokens[currentToken].type == IF) {
		++currentToken;
		Expr* condition = 0;
		Stmt* ifBody = 0;
		Stmt* elseBody = 0;
		condition = parse_expression(-999);
		if(condition->type == EMPTY || condition->type == ERROR) {
			printf("STMT PARSE: WE ARE FUCKED IN IF EXPRESSION\n");
			return thisStmt;
		}
		//check the expression	
		if(tokens[currentToken].type != COLON) {
			printf("STMT PARSE: exprected ; after condition in if statement\n");
			return thisStmt;
		}
		++currentToken;
		ifBody = parse_statement(false);
		if(ifBody->type == EMPTY_STMT || ifBody->type == ERROR_STMT) {
			printf("STMT PARSE: WE ARE FUCKED IN IF BODY\n");
			return thisStmt;	
		}

		if(currentToken < tokens.size() && tokens[currentToken].type == ELSE) {
			++currentToken;
			if(tokens[currentToken].type == COLON)
			{
				++currentToken;
				elseBody = parse_statement(false);
				if(elseBody->type == EMPTY_STMT || ifBody->type == ERROR_STMT) {
					printf("STMT PARSE: WE ARE FUCKED IN IF BODY\n");
					return thisStmt;	
				}
			}
			else  {
				printf("STMT PARSE: exprected : after 'else'\n");
				return thisStmt;
			}
		}	
		if(tokens[currentToken-1].type == _EOF || 
		   tokens[currentToken-1].type == SEMICOLON ||
		   elseBody == 0 && ifBody->type == GROUP_STMT ||
		   elseBody != 0 && elseBody->type == GROUP_STMT) {
			if(tokens[currentToken].type == _EOF || 
			   tokens[currentToken].type == SEMICOLON) {
				++currentToken;
			}
			thisStmt->type = IF_STMT;
			thisStmt->exprs[0] = condition;
			thisStmt->stmts[0] = ifBody;
			thisStmt->stmts[1] = elseBody;
			return thisStmt;
		}
		else{
			printf("STMT PARSE: WE ARE FUCKED SOMEWHERE IN IF\n");
			return thisStmt;
		}
	}	
	if(tokens[currentToken].type == WHILE) {
		++currentToken;
		Expr* condition = parse_expression(-999);
		if(condition->type == EMPTY || condition->type == ERROR) {
			printf("STMT PARSE: WE ARE FUCKED IN WHILE EXPRESSION\n");
			return thisStmt;
		}
		//check the expression	
		if(tokens[currentToken].type != COLON) {
			printf("STMT PARSE: exprected ; after condition in while statement\n");
			return thisStmt;
		}
		++currentToken;
		Stmt* whileBody = parse_statement(false);
		if(whileBody->type == EMPTY_STMT || whileBody->type == ERROR_STMT) {
			printf("STMT PARSE: WE ARE FUCKED IN WHILE BODY\n");
			return thisStmt;	
		}

		if(tokens[currentToken-1].type == _EOF || 
		   tokens[currentToken-1].type == SEMICOLON ||
		   whileBody->type == GROUP_STMT) {
			if(tokens[currentToken].type == _EOF || 
			   tokens[currentToken].type == SEMICOLON) {
				++currentToken;
			}
			thisStmt->type = WHILE_STMT;
			thisStmt->exprs[0] = condition;
			thisStmt->stmts[0] = whileBody;
			return thisStmt;
		}
		else {
			printf("STMT PARSE: WE ARE FUCKED SOMEWHERE IN WHILE\n");
			return thisStmt;
		}
	}
	if(tokens[currentToken].type == LEFT_BRACE) {
		//inside of the block statement statement 
		++currentToken;
		Stmt* childStmt;
		do {
			childStmt = parse_statement(true);
			thisStmt->group.push_back(childStmt);
		} while(!(currentToken >= tokens.size() || 
				tokens[currentToken].type == RIGHT_BRACE ));
		if(currentToken == tokens.size()) {
			printf("STMT PARSE: no } found in compound stmt group\n");
			return thisStmt;
		}
		else {
			++currentToken;
			thisStmt->type = GROUP_STMT;
			return thisStmt;
		}
	}
	if(tokens[currentToken].type == PRINT) {
		// we parse a print statement	
		++currentToken;
		
		Expr* printExpr = parse_expression(-999);
		if(printExpr->type == EMPTY || printExpr->type == ERROR) {
			//sync?
			printf("WE ARE FUCKED!\n");	
			return thisStmt;
		}
		else
		{
			//we passed a valid expression
			if(tokens[currentToken].type == _EOF || 
		   	   tokens[currentToken].type == SEMICOLON) {
				++currentToken;
			}

			thisStmt->type = PRINT_STMT;
			thisStmt->exprs[0] = printExpr;
			return thisStmt;
		}
		
	}
	if(tokens[currentToken].type == RETURN) {
		// we parse a print statement	
		++currentToken;
		
		Expr* returnExpr = parse_expression(-999);
		if(returnExpr->type == ERROR) {
			//sync?
			printf("WE ARE FUCKED!\n");	
			return thisStmt;
		}
		else
		{
			//we passed a valid expression
			if(tokens[currentToken].type == _EOF || 
		   	   tokens[currentToken].type == SEMICOLON) {
				++currentToken;
			}

			thisStmt->type = RETURN_STMT;
			thisStmt->exprs[0] = returnExpr;
			return thisStmt;
		}
		
	}
	else if (tokens[currentToken].type == VAR) {
		++currentToken;
		if(tokens[currentToken].type != IDENTIFIER)
		{
			printf("expected identifier after var\n");
			synchronize_parser();	
			return thisStmt;
		}
		thisStmt->lvalue = &tokens[currentToken];	
		++currentToken;
		if(tokens[currentToken].type != EQUAL)
		{
			printf("expected = after var [identifier]\n");
			synchronize_parser();	
			return thisStmt;
		}
		++currentToken;
		Expr* varExpr = parse_expression(-999);
		if(varExpr->type == EMPTY || varExpr->type == ERROR) {
			//sync?
			printf("we are fucked\n");
			return thisStmt;
		}
		if(tokens[currentToken].type == _EOF || 
		   tokens[currentToken].type == SEMICOLON) {
			++currentToken;
			thisStmt->type = VAR_STMT;
			thisStmt->exprs[0] = varExpr;
			return thisStmt;
		}
		else {
			printf("WE ARE FUCKED!\n");
			return thisStmt;
		}	
	}
	else if (tokens[currentToken].type == FUN) {
		std::vector<Token*> params = {};

		++currentToken;
		if(tokens[currentToken].type != IDENTIFIER) {
			printf("Expected identifier after fun keywork\n");	
			synchronize_parser();	
			return thisStmt;
		}	
		Token* functionNameToken = &tokens[currentToken];
		++currentToken;
		if(tokens[currentToken].type != LEFT_PAREN) {
			printf("Expected ( after identifier in function declaration\n");	
			synchronize_parser();	
			return thisStmt;
		}		
		++currentToken;

		if(tokens[currentToken].type != RIGHT_PAREN)
		{
			//parse args	
			if(tokens[currentToken].type != IDENTIFIER)
			{
				printf("Expected identifiers inside the () of function declaration\n");
				synchronize_parser();	
				return thisStmt;
			}

			// We've found our first identifier
			
			params.push_back(&tokens[currentToken]);
			++currentToken;
			while(tokens[currentToken].type == COMMA)
			{
				++currentToken;
				if(tokens[currentToken].type != IDENTIFIER)
				{	
					printf(
						"Expected identifier after , in function decl param list\n");

					synchronize_parser();	
					return thisStmt;
				}
				params.push_back(&tokens[currentToken]);
				++currentToken;
			} 

		}

		if(tokens[currentToken].type != RIGHT_PAREN)
		{
			printf("Exprected ) at the end of function decl param list\n");
			synchronize_parser();	
			return thisStmt;
		}
		
		++currentToken;	

		Stmt* funcBody  = parse_statement(false);
		if(funcBody->type == EMPTY_STMT || funcBody->type == ERROR_STMT) {
			printf("STMT PARSE: WE ARE FUCKED IN FUNCTION BODY\n");	
			synchronize_parser();	
			return thisStmt;	
		}

		if(tokens[currentToken-1].type == _EOF || 
		   tokens[currentToken-1].type == SEMICOLON ||
		   funcBody->type == GROUP_STMT) {
			if(tokens[currentToken].type == _EOF || 
			   tokens[currentToken].type == SEMICOLON) {
				++currentToken;
			}
			// fill the function statement
			if(params.size() != 0) {
				functionParams.push_back(params);
				thisStmt->extraIndex = functionParams.size()-1;
			}
			else {
				thisStmt->extraIndex = -1;
			}
			thisStmt->type = FUNCTION_STMT;
			thisStmt->lvalue = functionNameToken;
			thisStmt->stmts[0] = funcBody;
			return thisStmt;
		}
		else {
			printf("STMT PARSE: WE ARE FUCKED SOMEWHERE AFTER FUNCTION DECL\n");
			synchronize_parser();	
			return thisStmt;
		}
		
	}
	else {

		// we parse an expression statement
		Expr* expr = parse_expression(-999);
		if(expr->type == EMPTY || expr->type == ERROR) {
			//sync?
			printf("we are fucked\n");
			return thisStmt;
		}
		if(tokens[currentToken].type == _EOF || 
		   tokens[currentToken].type == SEMICOLON) {
			++currentToken;
			thisStmt->type = EXPR_STMT;
			thisStmt->exprs[0] = expr;
			return thisStmt;
		}
		else {
			printf("WE ARE FUCKED!\n");
			return thisStmt;
		}
	}
}


Expr* add_empty_expr() {
	exprs[exprsCount] = (Expr){ERROR, 0, {0,0}};
	++exprsCount;
	return exprs+exprsCount-1;
}

Expr* try_parse_function_call() {
	Expr* callExpr = 0;
	if(tokens[currentToken].type != LEFT_PAREN)
	{	
		//we are not calling a function -> return 0
		return 0;
	}
	callExpr = add_empty_expr();
	++currentToken;
	if(tokens[currentToken].type == RIGHT_PAREN)
	{
		++currentToken;
		callExpr->type = CALL;
		callExpr->extraIndex = -1;
		return callExpr;
	}
	
	std::vector<Expr*> args = {};
	--currentToken;
	do
	{ 
		++currentToken;

		Expr* arg = parse_expression(-999);
		if(arg->type == ERROR || arg->type == EMPTY) {
			callExpr->type = ERROR;
			printf("Error occured in function call params");
			return callExpr;	
		}
		args.push_back(arg);
	}	
	while(tokens[currentToken].type == COMMA);
	
	if(tokens[currentToken].type == RIGHT_PAREN) {
		//add params somehow
		functionArgs.push_back(args);
		callExpr->extraIndex = functionArgs.size()-1;
		++currentToken;
		callExpr->type = CALL;
		return callExpr;
	}

	printf("PARSE EXPR: expected a ) after function arguments\n");
	//we don't
	//++current token;
	//because we sync later
	return callExpr;
}

// #define VERBOSE
Expr* parse_expression(float power)
{
	Expr* lhs;
	//parse current token 
	if(tokens[currentToken].type == _EOF || 
	   tokens[currentToken].type == SEMICOLON) {
		exprs[exprsCount] = {EMPTY, 0, {0,0}};
		lhs = exprs+exprsCount;
		++exprsCount;
		return lhs;
	}
	else if(tokens[currentToken].type == IDENTIFIER) {
		exprs[exprsCount] = {LITERAL, &tokens[currentToken], {0,0}};
		lhs = exprs+exprsCount;
		++exprsCount;

		//we move to the next token after identifier
		++currentToken;
		Expr* callExpr = try_parse_function_call();
		while(callExpr != 0)
		{
			if(callExpr->type == CALL) {
				Expr* tmp = lhs;	
				lhs = callExpr;
				lhs->children[0] = tmp;
			}
			else {
				doPanic = true;
				synchronize_parser();
				return callExpr;
			}		
			callExpr = try_parse_function_call();
		}
		// after we parsed identifier ("()")* are on the next token
		// but we will do ++currentToken after if else chain
		// so we need to:
		--currentToken;
	}
	else if(tokens[currentToken].type == NUMBER ||
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
			synchronize_parser();
			return panicRes;
		}
	}
	else {
		printf("PARSE:%d unexpected token [%s] of type %s in line %d\n",
			   currentToken,
			   tokens[currentToken].content.c_str(),
			   tokenNames[tokens[currentToken].type],
			   tokens[currentToken].line);

		exprs[exprsCount] = {ERROR, &tokens[currentToken-1], {0,0}};
		Expr* panicRes = exprs + exprsCount;	
		++exprsCount;
		doPanic = true;
		synchronize_parser();
		return panicRes;			
	}


	++currentToken;
	

	while(tokens[currentToken].type != _EOF &&
		  tokens[currentToken].type != SEMICOLON)
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
				Expr* rhs;
				if(tokens[currentToken-1].type == EQUAL) {
					//here we parse for right associative ops
					rhs = parse_expression(opPower[tokens[currentToken-1].type]);
				}
				else {
					//here we parse for left associative ofs
					rhs = parse_expression(opPower[tokens[currentToken-1].type]+1);
				}
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

