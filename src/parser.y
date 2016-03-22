/* -*- Mode: c; tab-width:4; indent-tabs-mode: nil; c-basic-offset:4 -*-  */
%define api.pure
%parse-param { struct pcdata *pp }
%parse-param { void *scanner }
%lex-param {void *scanner}

%{
 /*
  * Copyright (C) 2008-2011 Lingyun Yang.
  *
  * This software may be used and distributed according to the terms
  * of the GNU General Public License, incorporated herein by reference.
  *
  * For more information please contact lyyang@lbl.gov
  *
  */

 /* Author: Lingyun Yang, lyyang@lbl.gov */
 /* date: 2011-05-20 09:40 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <ctype.h>

 #include "glpsfuncs.h"

 #ifndef M_PI
  #define M_PI 3.14159265358979323846
 #endif

    /* in C++ Boost log */
 void log_message(int level, const char *file, int line, const char *func, 
                 const char *msg);

 #ifdef _MSC_VER
   #define DBG_MSG(msg, ...) \
     fprintf(stderr, "[%s:%d:%s]: " msg, __FILE__, __LINE__, __FUNCTION__, \
             ##__VA_ARGS__)
   #define LOG_MSG(lvl, msg, ...) \
     { char *s = (char*) malloc(2048); \
       _snprintf(s, 2047, msg, ##__VA_ARGS__); \
       log_message(lvl, __FILE__, __LINE__, __FUNCTION__, s); \
       free(s); \
     }
   #define LOG_TRACE(   msg, ...) LOG_MSG(0, msg, ##__VA_ARGS__)
   #define LOG_DEBUG(   msg, ...) LOG_MSG(1, msg, ##__VA_ARGS__)
   #define LOG_INFO(    msg, ...) LOG_MSG(2, msg, ##__VA_ARGS__)
   #define LOG_WARNING( msg, ...) LOG_MSG(3, msg, ##__VA_ARGS__)
   #define LOG_ERROR(   msg, ...) LOG_MSG(4, msg, ##__VA_ARGS__)
   #define LOG_FATAL(   msg, ...) LOG_MSG(5, msg, ##__VA_ARGS__)
 #else
   #define DBG_MSG(msg, args ...) \
     fprintf(stderr, "[%s:%d:%s]: " msg, __FILE__, __LINE__, __func__, ##args)
   #define LOG_MSG(lvl, msg, args ...) \
     { char *s = (char*) malloc(2048); \
       snprintf(s, 2047, msg, ##args); \
       log_message(lvl, __FILE__, __LINE__, __func__, s); \
       free(s); \
     }
   #define LOG_TRACE(   msg, args ...) LOG_MSG(0, msg, ##args)
   #define LOG_DEBUG(   msg, args ...) LOG_MSG(1, msg, ##args)
   #define LOG_INFO(    msg, args ...) LOG_MSG(2, msg, ##args)
   #define LOG_WARNING( msg, args ...) LOG_MSG(3, msg, ##args)
   #define LOG_ERROR(   msg, args ...) LOG_MSG(4, msg, ##args)
   #define LOG_FATAL(   msg, args ...) LOG_MSG(5, msg, ##args)
 #endif

 /* if a statement is redefined */
 #define DUPLICATE_REPLACE 1
 #define DUPLICATE_APPEND 2
 #define DUPLICATE_IGNORE 3

 /* static int duplicate_mode = DUPLICATE_REPLACE; */
 /* int yydebug=1; */

 /* yy_flex_debug = 1; */

 #define YYDEBUG 1
 struct pcdata;

 #if YYBISON
   /* fix: warning implicit declaration of function 'yylex' is invalid in C99 */
   union YYSTYPE;
   int yylex(union YYSTYPE *, void *);
 #endif
%}

%verbose
/* %define parse.trace */
%debug

/* %locations */
/* %pure_parser */ /* makes yylloc local */

%union {
    int       fn;   /* function */
    int     ival;   /* int value */
    double  dval;   /* plain number */
    char*    str;   /* string */
    struct glps_ast *a;
    struct elemtree_el *bl;
    struct glps_symbol *sb;
}

%{
    struct pcdata;
    /* #include "scanner.lex.h" */
    #include "glpsfuncs.h"
    #define YYLEX_PARAM pp->scaninfo
%}

%token <str> STRING ELEMENT ACTION ELEM_PROP ACT_PROP LINE ELEM_FAM
%token <dval> REAL
%token <ival> INTEGER
%token <sb> ID
%token <fn> FUNC

%token <str> BL DEFED_PRPT

%token SET TITLE SHOW INCLUDE LINE_HEAD
%token BEND BPM CAVITY CORR DRIFT MARKER MULTIPOLE INV QUAD SEXT WIGGLER 

%left ','
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <a> expression
%type <a> expression_list
%type <a> stmt_property
%type <a> stmt_property_list
%type <a> beamline

%%

statement_list: statement
        |       statement_list statement
  /*|       error ';' { 
    fprintf(stderr, "unknown statement, quitting\n"); YYABORT; } */
        ;

statement: ID '=' expression ';' {
    /* DBG_MSG(">>> id = expression; (%s = )\n", $1->name); */
    pp->al[pp->nast++] = glps_new_assign(pp, $1, $3);
  }
| ID '=' '(' expression_list ')' ';' {
    pp->al[pp->nast++] = glps_new_assign(pp, $1, $4);
  }
| DEFED_PRPT '=' expression ';' {}
| DEFED_PRPT '=' '(' expression_list ')' ';' {}
/*| ACTION ',' ID ';' { DBG_MSG("Not implemented yet!!\n"); }*/
| ACTION ',' stmt_property_list ';' {
    /* DBG_MSG("Not implemented yet!!: %s\n", $1); */
    pp->al[pp->nast++] = glps_new_action(pp, $1, $3);
  }
| ELEMENT ':' ELEM_FAM ';' {
    /* DBG_MSG("simple element %s: %s;\n", $1, $3); */
    pp->al[pp->nast++] = glps_new_element(pp, $1, $3, NULL);
  }
| ELEMENT ':' ELEM_FAM ',' stmt_property_list ';' {
    /* DBG_MSG("element  %s:%s\n", $1, $3); */
    pp->al[pp->nast++] = glps_new_element(pp, $1, $3, $5);
  }
| LINE ':' LINE_HEAD '=' '(' beamline ')' ';'  {
    pp->al[pp->nast++] = glps_new_line(pp, $1, $6);
  }
| error {
    DBG_MSG("syntax error\n");
    LOG_FATAL("syntax error");
    YYABORT;
  }
;

expression_list: expression ',' expression {
    $$ = glps_new_ast(pp, GLPS_NODE_CONCAT_EXPR, $1, $3);}
| expression_list ',' expression {
    $$ = glps_new_ast(pp, GLPS_NODE_CONCAT_EXPR, $1, $3);}
;

expression: expression '+' expression {
    $$ = glps_new_ast(pp, GLPS_NODE_FADD, $1, $3); }
|  expression '-' expression { $$ = glps_new_ast(pp, GLPS_NODE_FSUB, $1, $3); }
|  expression '*' expression { $$ = glps_new_ast(pp, GLPS_NODE_FMULT, $1, $3); }
|  expression '/' expression { $$ = glps_new_ast(pp, GLPS_NODE_FDIV, $1, $3); }
|  '-' expression %prec UMINUS {
    $$ = glps_new_ast(pp, GLPS_NODE_FINV, $2, NULL); }
|  '(' expression ')' { $$ = $2; }
|  INTEGER { $$ = glps_new_intval(pp, $1); }
|  REAL { $$ = glps_new_doubleval(pp, $1); }
|  ID {
    /* ID already in pp->symbtab, set from scanner.l */
    $$ = glps_new_varref(pp, $1);  }
|  FUNC '(' expression ')' {
    DBG_MSG("not implemented func(expression)\n");
    LOG_ERROR("not implemented func(expression)\n");
    $$ = NULL; }
|  FUNC '(' expression_list ')' {
    DBG_MSG("not implemented func(expr_list)\n");
    LOG_ERROR("not implemented func(expr_list)\n");
    $$ = NULL;}
| STRING { $$ = glps_new_strval(pp, $1); }
;

stmt_property_list: stmt_property { $$ = $1; }
| stmt_property_list ',' stmt_property {
    $$ = glps_new_ast(pp, GLPS_NODE_CONCAT_PRPT, $1, $3); }
;

stmt_property: ELEM_PROP '=' expression { $$ = glps_new_prpt(pp, $1, $3); }
| ELEM_PROP '=' '(' expression_list ')' { $$ = glps_new_prpt(pp, $1, $4); }
| ACT_PROP '=' expression { $$ = glps_new_prpt(pp, $1, $3);  }
| ACT_PROP '=' '(' expression_list ')' { $$ = glps_new_prpt(pp, $1, $4); }
| ELEM_PROP { $$ = glps_new_prpt(pp, $1, glps_new_intval(pp, 1)); }
| ACT_PROP { $$ = glps_new_prpt(pp, $1, glps_new_intval(pp, 1)); }
;

beamline: BL { $$ = glps_new_lineelem(pp, $1); }
|  beamline ',' beamline { $$ = glps_new_ast(pp, GLPS_NODE_CONCAT_LINE, $1, $3); }
|  INTEGER '*' '(' beamline ')' {
    $$ = glps_new_ast(pp, GLPS_NODE_LINE_MULT, glps_new_intval(pp, $1), $4); }
|  INTEGER '*' BL {
    GlpsAst *n = glps_new_intval(pp, $1);
    $$ = glps_new_ast(pp, GLPS_NODE_LINE_MULT, n, glps_new_lineelem(pp, $3)); }
|  INV BL { $$ = glps_new_ast(pp, GLPS_NODE_LINE_INV, glps_new_lineelem(pp, $2), NULL); }
|  INV '(' beamline ')' { $$ = glps_new_ast(pp, GLPS_NODE_LINE_INV, $3, NULL);  }
;
%%


int yylex(YYSTYPE *yylval, void *scanner);

/*! \brief Error report with line number.
 *
 * If the error (syntax error) happens at the first line, the position (line
 * number) is not given correctly.
 */

/*! \brief Add a property list to a element.
 *
 * append to the end.
 */

