#pragma once
#include"utils.h"
#include<vector>

//these are twmporaraly extern
extern int currentToken;
extern bool doPanic;
extern int exprsCount;
extern int stmtsCount;




int parse_all();
extern Expr* exprs;
extern Stmt* stmts;

void synchronize_expression_parser();

Expr* parse_expression(float power);

Stmt* parse_statement(bool allowDecls); 
