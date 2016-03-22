/* -*- Mode: c; tab-width:4; indent-tabs-mode: nil; c-basic-offset:4 -*-  */

/* 
 *
 * Copyright (C) 2008 Lingyun Yang.
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 * For more information please contact lingyun.yang@gmail.com
 *
 */


/* Author: L. Yang, lingyun.yang@gmail.com */
/* date: 2010-09-01 14:37 */
/* left to command line (command line option does not overwrite here) */
/* header-file="scanner.lex.h" */ 

%option bison-bridge
%option noyywrap nodefault yylineno reentrant debug
%option case-insensitive

%{
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

struct pcdata; /* forward declare (compiler warning in yyparse) */

#include "parser.tab.h"
#include "glpsfuncs.h"

#define MAX_LINE_BUF_SIZE 10240

/* #define DEBUG 1 */

char* element_name;
unsigned int lineno = 1;

char linebuf[MAX_LINE_BUF_SIZE];
char strbuf[MAX_LINE_BUF_SIZE];

void log_message(int level, const char *file, int line, const char *func, 
                 const char *msg);

extern char** beam_lines;

extern char* root_path;

 /* processing property statement in an definition 
  * e.g. L = 0.1, property=(1,2,3,4,5); 
  * every time, when meet an identifier, we record its name,
  * in case we need it on the right hand side. we have to keep its value later.
  * i.e. vec/vec_size/value 
  */

#ifdef _MSC_VER

#define DBG_MSG(msg, ...) \
 fprintf(stderr, "[%s:%d:%s]: " msg, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
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

int push_lattice(struct pcdata* p, const char* fname)
{
    int i = p->ilat + 1;
    if (i < GLPS_MAX_INC_DEPTH) {
        p->latfname[i] = str_duplicate(fname);
        p->ilat = i;
    } else {
        LOG_ERROR("lattice file nested too deep (>%d)", GLPS_MAX_INC_DEPTH);
    }

    return i;
}

/* return index of previous lattice */
int pop_lattice(struct pcdata *p)
{
    const int i = p->ilat;
    if (i >= 0) {
        free(p->latfname[i]);
        p->latfname[i] = NULL;
        --p->ilat;
    }
    return i;
}

%}

/* %option debug */
/* %option reentrant */
/* %option noreject */
  /* definitions */

  /* NUM   -?[0-9]+ */
  /* REAL  -?(([0-9]+)|(([0-9]*\.)?[0-9]+)([eE][-+]?[0-9]+)?)  */
NUM   [0-9]+
/*REAL  (([0-9]+)|(([0-9]*\.)?[0-9]*)([eE][-+]?[0-9]+)?) */
REAL  (([0-9]+\.)|([0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)|([0-9]+\.[0-9]*([eE][-+]?[0-9]+)?))
WS    [ \t\n]*
IDENTIFIER  [A-Za-z][A-Za-z0-9_\.]*

  /* x exclusive start-condition: */
%x sc_comment sc_str sc_beamline sc_element sc_action sc_assign sc_elemhead sc_actexpr

%%

%{
  struct pcdata *pp = yyextra;
%}
  /* --------------------------------------- */

  /* rules */
<INITIAL><<EOF>>  { 
    LOG_TRACE("END of '%s' (%d)\n", pp->latfname[pp->ilat], pp->ilat);
    LOG_TRACE("Reach the End of Lattice without an \"END\" statement."); 

    if ( pop_lattice(pp) <= 0) {
     #ifdef DEBUG
        DBG_MSG("Thats the end %d\n", pp->ilat);
     #endif
        /* yy_delete_buffer(YY_CURRENT_BUFFER); */
        yy_flush_buffer(YY_CURRENT_BUFFER, pp->scaninfo);
        yyterminate();
    }else {
        LOG_DEBUG("==glps== [INFO] Switching to buffer '%s' (%d)...",
                  pp->latfname[pp->ilat], pp->ilat);
        yy_delete_buffer(YY_CURRENT_BUFFER, pp->scaninfo);
        if (pp->ilat>=0) {
            yy_switch_to_buffer((YY_BUFFER_STATE)pp->latfstack[pp->ilat],
                                pp->scaninfo);
        }
        /* lineno = pp->lat_file_lineno[pp->ilat]; */
    }
}

<sc_element,sc_action,sc_beamline><<EOF>>  {
    DBG_MSG("statement is incomplete at the end of file");
}

<*>"sqrt" { yylval->fn = GLPS_NODE_FSQRT; return FUNC; }
  /* Syntax error at first line will also be copied into linebuf */
<*>\n.*  { 
    ++lineno;
    yycolumn = 1;
    strncpy(linebuf, yytext+1, MAX_LINE_BUF_SIZE-1);
    linebuf[MAX_LINE_BUF_SIZE-1] = '\0';
    yyless(1); 

 #ifdef DEBUG
    fprintf(stdout, "%d: %s\n", lineno, linebuf); 
 #endif
   }

  /* Comment */ 
  /*
  "{"                    { BEGIN(sc_comment); }
  <sc_comment>[^"}"\n]+  ;
  <sc_comment>\n         {++lineno;}
  <sc_comment>"}"+       { 
      BEGIN(INITIAL); 
      fprintf(stderr, "--- COMMENT ---\n");
   }
   */
        /* comment */
#.*\n  { ++lineno; yycolumn = 1; 
    /* fprintf(stderr, "\n---- Comment ----\n"); */
   } 

<*>\"[^\"\n]*\"  { 
    /* String, doesn't span > 1 line */
    /* TODO: remove this constraint */
    /* assuming "string", here remove the " */
    yylval->str = (char*)malloc(strlen(yytext));
    strncpy(yylval->str, yytext+1, strlen(yytext)-2);
    yylval->str[strlen(yytext)-2] = '\0';
    /* fprintf(stderr, "it is a string: %s\n", yylval->str); */
    return STRING; 
   }

<*>\"[^\"\n]*$ {
 #ifdef DEBUG
    DBG_MSG("----\n");
    fprintf(stderr, "Unterminated string\n");
 #endif
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);
    return STRING;
   }

<*>{NUM} {
    LOG_TRACE("int: %s", yytext);
    yylval->ival = atof(yytext); return INTEGER; }

<*>{REAL} {
    LOG_TRACE("real: %s", yytext);
    yylval->dval = atof(yytext);
    return REAL; }

include[ \t\n]*","[ \t\n]*[^;\t\n]+";" {
    FILE *inp = NULL;
    /* include the lattice input */
    char *head, *tail, *inc;
    size_t sz;
    LOG_TRACE("openning file: '%s'", yytext);
    if (pp->ilat >= GLPS_MAX_INC_DEPTH) {
        LOG_ERROR("lattice file nested too deep (>%d)", GLPS_MAX_INC_DEPTH);
    }

    LOG_DEBUG("include file: %s\n", yytext);

    head = strchr(yytext, '"');
    tail = strrchr(yytext, '"');
    if (head && tail && head < tail) {
        sz = tail - head; /* strspn(head, tail); */
        inc = (char*) malloc(sz*sizeof(char));
        strncpy(inc, head + 1, sz - 1);
        inc[sz-1] = '\0';
        LOG_INFO("open new file: '%s'\n", inc);
        if ( (inp = fopen(inc, "r")) == NULL ) {
            LOG_FATAL("can not open lattice file: \"%s\"", inc);
            /* error */
            yyerror(pp, "can not open lattice file: %s", inc);
            yyterminate();
        } else {
            if (push_lattice(pp, inc) < GLPS_MAX_INC_DEPTH) {
                LOG_INFO("lattice pushed: (%d)\n", pp->ilat-1);
                pp->latfstack[pp->ilat-1] = YY_CURRENT_BUFFER;
                yy_switch_to_buffer(
                    yy_create_buffer(inp, YY_BUF_SIZE, pp->scaninfo),
                    pp->scaninfo);
                lineno = 0;
            } else {
                /* yyerror */
            }
        }
        free(inc);
    } else {
        DBG_MSG("file name should be quoted, e.g. include, "
                "\"lattice.lat\"");
        /* yyerror */
    }
}

<INITIAL>end{WS}";" { /* logical end of file */
    LOG_INFO("-- End of Lattice --"); 
    LOG_INFO("LOGICAL END of lattice file: '%s'", pp->latfname[pp->ilat]);

    if ( pop_lattice(pp) < 0) {
        LOG_INFO("LOGICAL END of lattice file: '%s'", pp->latfname[pp->ilat]);
        /* DBG_MSG("T END of lattice file\n"); */
        yy_flush_buffer(YY_CURRENT_BUFFER, pp->scaninfo);
        /* yy_delete_buffer(YY_CURRENT_BUFFER); */
        yyterminate();
    }else {
        LOG_INFO("switching... to buffer %d\n", pp->ilat);

        yy_delete_buffer(YY_CURRENT_BUFFER, pp->scaninfo);
        yy_switch_to_buffer(pp->latfstack[pp->ilat], pp->scaninfo);
        /* lineno = pp->lat_file_lineno[pp->ilat]; */
    }
 } 


<INITIAL>{IDENTIFIER}/{WS}":"{WS}line{WS}"=" {
    BEGIN(sc_beamline); 
    LOG_TRACE("line: '%s'", yytext);
    /* NEW_PROPERTY_SYMBOL(yytext); */
    /* sp = symb_new(yytext); */
    /* yylval->symp = sp; */
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);

    return LINE;
}


<INITIAL>{IDENTIFIER}/{WS}: {
    BEGIN(sc_elemhead);
    LOG_TRACE("element: %s (begin sc_elemhead)", yytext);
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);

    return ELEMENT;
 }

<INITIAL>{IDENTIFIER}/{WS}=  {
    BEGIN(sc_assign);
    yyless(0);
 }
  /*
  <INITIAL>{IDENTIFIER}/{WS}; {
      yylval->str = strdup(yytext);
      return ACTION;
  }
  */
<INITIAL>{IDENTIFIER}/{WS},  { 
    BEGIN(sc_action); 
    /* fprintf(stderr, "Action: %s\n", yytext); */
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);
    return ACTION; 
 }

<INITIAL>{IDENTIFIER}[ \t\n]+{IDENTIFIER}/{WS},  { 
    BEGIN(sc_action); 
    LOG_TRACE("action: '%s'", yytext);
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);
    return ACTION; 
 }

<sc_elemhead>(?#":"{WS}){IDENTIFIER} {
    LOG_TRACE("begin elemhead: '%s'", yytext);

    BEGIN(sc_element);
    yylval->str = str_duplicate(yytext);
    return ELEM_FAM;
}
<sc_beamline>{WS}(?i:line)/{WS}= { 
    /* DBG_MSG("beamline: %s\n", yytext); */
    LOG_TRACE("beamline: '%s'", yytext);
    return LINE_HEAD;
}

<sc_action>{IDENTIFIER}/{WS}[=,;] {
    BEGIN(sc_actexpr);
    /* DBG_MSG("----\n"); */
    /* convert_key_case(yytext); */
    LOG_TRACE("action property: '%s'", yytext);

    yylval->str = str_duplicate(yytext); /* symb_new(yytext); */
    return ACT_PROP; }

<sc_actexpr>"," {
    BEGIN(sc_action);
    return ',';
}

<sc_element>{IDENTIFIER}/{WS}= { 
    /* general element */ 
    LOG_TRACE("magnet property: '%s'", yytext);

    yylval->str = str_duplicate(yytext); /* symb_new(yytext); */
    return ELEM_PROP; 
 }

<sc_beamline>{IDENTIFIER} { 
    /* BL: Line=(ID1, ID2, ...) */
    LOG_TRACE("line element: '%s'", yytext);
    yylval->str = (char*)malloc((strlen(yytext)+1)*sizeof(char));
    strcpy(yylval->str, yytext);
    
    return BL; 
 }

<*>{IDENTIFIER} {
    /* BL preceeds this ID. ID is never in line definition */
    /* always a new copy */
    LOG_TRACE("id= '%s'", yytext);
    yylval->sb = glps_lookup(pp, yytext);
    return ID;
 }

<sc_beamline>-?[0-9]+ { yylval->dval = atoi(yytext); return INTEGER;}
<sc_beamline>"-" { return INV; }

<*>"+"|"-"|"*"|"/" { return yytext[0]; }

<*>":"|","|"="|"("|")" {
    LOG_TRACE("str: '%s'", yytext);
    return yytext[0]; }

<*>";" { BEGIN(INITIAL); return yytext[0]; }

<*>[ \t\r] ;

<*>"&" ;

<*>. {
    LOG_ERROR("unknown character: '%s'", yytext);
    return yytext[0];
 }
%%

