/*
 * Copyright (C) 2008 Lingyun Yang.
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 * For more information please contact lyyang@bnl.gov
 *
 */


/* Author: Lingyun Yang, lyyang@bnl.gov */

#include <stdlib.h>
#include <stdio.h>

#ifndef GLPSFUNCS_H_
#define GLPSFUNCS_H_

#ifdef __cplusplus
extern "C" {
 #endif 

 #define GLPS_VERSION 0x050000
 #define GLPS_VERSION_STR "5.0.0"
 #define glps_VERSION_MAJOR 5   
 #define glps_VERSION_MINOR 0
 #define glps_VERSION_PATCH 0

    /* struct elemtree_el; */
    /* typedef struct elemtree_el elemtree; */

    /* Error handling */
    enum GLPS_ERR_CODE {
        GLPS_SUCCESS      = 0,        /**< general success return value */
        GLPS_FAILURE      = 1,
        GLPS_ERR_UNIOPERATOR,
        GLPS_ERR_UNIOPERAND,
        GLPS_ERR_BINOPERATOR,
        GLPS_ERR_BINOPERAND,
        GLPS_ERR_VARIABLE,
        GLPS_ERR_FILE        = 20,
        GLPS_ERR_BEAMLINE    = 21,       /**< invalid beamline */
        GLPS_ERR_ELEMENT     = 22,       /**< invalid magnet */
        GLPS_ERR_PROPERTY    = 23,       /**< invalid property */
        GLPS_ERR_ACTION      = 25,
        GLPS_ERR_DATATYPE    = 24,       /**< invalid data type */
        GLPS_ERR_RANGE       = 26,       /* range error */
        GLPS_ERR_STATEMENT   = 27,       /* */
    };

    enum GLPS_NODE_TYPE {
        GLPS_NODE_NONE  = -1,   /* */
        /* const data, order matters when upconvert */
        GLPS_NODE_CHAR = 1,   /* char */
        GLPS_NODE_SHORT,      /* (reserved) */ 
        GLPS_NODE_INT,        /* integer value */
        GLPS_NODE_LONG,       /* (reserved) */
        GLPS_NODE_FLOAT,      /* (reserved) */
        GLPS_NODE_DOUBLE,     /* double value */
        GLPS_NODE_STRING,     /* (reserved) string */
        /* */
        GLPS_NODE_ASSIGN = 32,  /* assignment */
        GLPS_NODE_PROPERTY,     /* a property (like assignment) */
        GLPS_NODE_VARIABLE,     /* symbol */
        GLPS_NODE_LINE,         /* beam line */
        GLPS_NODE_LINEELEM,     /* subline or element in line */
        GLPS_NODE_ACTION,       /* action */
        GLPS_NODE_ELEMENT,      /* element */
        /* */
        GLPS_NODE_TAG    = 64,  /* a tag, no value */
        /* operators */
        GLPS_NODE_FUNC    =96,    /* function, f(x) */
        GLPS_NODE_FADD,            /* '+' */
        GLPS_NODE_FSUB,            /* '-' */
        GLPS_NODE_FMULT,           /* '*' */
        GLPS_NODE_FDIV,            /* '/' */
        GLPS_NODE_FINV,            /* inverse  */
        GLPS_NODE_FEXP,            /* exp(x) */
        GLPS_NODE_FLOG,
        GLPS_NODE_FSQRT,
        GLPS_NODE_FPRINT,
        GLPS_NODE_CONCAT_EXPR,    /* concatenation */
        GLPS_NODE_CONCAT_PRPT,    /* concatenation */
        GLPS_NODE_CONCAT_LINE,    /* concat subline/element */
        GLPS_NODE_LINE_INV,       /* inverse beamline */
        GLPS_NODE_LINE_MULT,      /* multiply beamline */
    };


    /*! Global symbol table, store every variable, function defined. It also
     *  stores the property of magnets and actions. But since the magnet
     *  properties are copied into a new structure after matched in Yacc
     *  grammar, and freed after that, they are seperated from the global
     *  linked list.
     */
    typedef struct glps_symbol {
        char *name;     /* PI, L, ... */
        int  dtype;     /* data_type, e.g. GLPS_INT */
        size_t memsize; /* allocaed mem size: n*sizeof(data_type) */
        size_t nov;     /* number of elements, 1=scalar, n=array */
        void *val;      /* dsize * nov is the total mem alloc size */
        struct glps_symbol *prev;
    } GlpsSymbol;

    typedef struct glps_ast {
        int nodetype;
        struct glps_ast *l;
        struct glps_ast *r;
    } GlpsAst;

    typedef struct glps_intval {
        int nodetype;
        int i;
    } GlpsIntVal;

    typedef struct glps_doubleval {
        int nodetype;
        double d;
    } GlpsDoubleVal;

    typedef struct glps_strval {
        int nodetype;
        char *s;
    } GlpsStrVal;

    typedef struct glps_varref {
        int nodetype;
        GlpsSymbol *s;
    } GlpsVarRef;

    typedef struct glps_assign {
        int nodetype;
        struct glps_symbol *symb;
        struct glps_ast *v;
    } GlpsAssign;

    typedef struct glps_prpt {
        int nodetype;
        char *name;
        struct glps_ast *v;
    } GlpsProperty;

    typedef struct glps_element {
        int nodetype;
        char* name;       /* element name */
        char* family;       /* element type/family */
        struct glps_ast* prpt;
    } GlpsElement;
    
    typedef struct glps_action {
        int nodetype;
        char *name;  /* action name */
        struct glps_ast *prpt; /* a list of assignment */
    } GlpsAction;

    typedef struct glps_beamline {
        int nodetype;
        char* name;     /* name of beamline */
        struct glps_ast *line;    /* elements */
    } GlpsLine;

    /* open multiple lattice file, push/pop the stack. */
    #define GLPS_MAX_INC_DEPTH 10
    #define GLPS_MAX_AST 500000

    struct pcdata {
        void *scaninfo;
        struct glps_symbol *symtab;
        int nast;
        struct glps_ast **al;           /* hold MAX_AST */
        int ilat;
        char* latfname[GLPS_MAX_INC_DEPTH];
        /* int lat_file_lineno[GLPS_MAX_INC_DEPTH]; */
        struct yy_buffer_state *latfstack[GLPS_MAX_INC_DEPTH]; 
    };
    int push_lattice(struct pcdata* p, const char* fname);
    int pop_lattice(struct pcdata *p);

    /*!\brief parse the lattice file into internal data structure 
     */
    int parse_lattice(const char* f);

    /*! \brief release the internal structure storing parsed lattice 
     */
    int free_lattice();

    char *get_path_root(char *f);
    int parse_lattice_flat(char* f, char* flat);
    int parse_lattice_xml(char* lat, char* xml);
    int parse_lattice_ini(char* f);
    int print_flat_lattice(FILE* pf, char* bl);


    /* ----------------------------------------- */

    /*! \brief get the definition of a statement, including action, element
     *   and beamline 
     */
    /*! \brief compare two string, case-incensitive */
    int str_case_cmp(const char* ps1, const char* ps2);
    const char* glps_strerror ();
    char *str_duplicate(const char *s); /* = strdup */

    /* void glps_set_rootpath(const char* rootpath); */

    /* read data in symb linked list */
    int symb_get_double(const char* name, double* val);
    int symb_get_vector(const char* name, double* val, int n);
    int symb_get_string(const char* name, char* val, int n);
    int symb_get_line(const char* name, char** val, int m, int n);
    int symb_get_fline(const char* name, char** val, int m, int n);

    void glps_error (const char * reason, const char * file, int line, 
                     int glps_errno);
    void glps_abort (const char * reason, const char * file, int line, 
                     int glps_errno);

    /* interface to scanner */
    void yyerror(struct pcdata *pp, char *s, ...);

    GlpsSymbol *glps_lookup(struct pcdata *pp, const char *name);

    void glps_print_symbol(const GlpsSymbol *p, int level,
                           FILE *out, const char *tail);

    /* build AST */
    int init_buildin_symbols(struct pcdata *pp);
    GlpsAst *glps_new_varref(struct pcdata *pp, GlpsSymbol *s);
    GlpsAst *glps_new_doubleval(struct pcdata *pp, double d);
    GlpsAst *glps_new_intval(struct pcdata *pp, int d);
    GlpsAst *glps_new_strval(struct pcdata *pp, char *s);
    GlpsAst *glps_new_ast(struct pcdata *pp, int nt, GlpsAst *l, GlpsAst *r);
    GlpsAst *glps_new_assign(struct pcdata *pp, GlpsSymbol *s, GlpsAst *ast);
    GlpsAst *glps_new_prpt(struct pcdata *pp, char *s, GlpsAst *ast);
    GlpsAst *glps_new_action(struct pcdata *pp, char *s, GlpsAst *a);
    GlpsAst *glps_new_element(struct pcdata *pp, char *name,
                              char *fam, GlpsAst *a);
    GlpsAst *glps_new_line(struct pcdata *pp, char *name, GlpsAst *a);
    GlpsAst *glps_new_lineelem(struct pcdata *pp, char *s);

    void glps_free_symbol(GlpsSymbol *p);
    void glps_copy_symbol(GlpsSymbol *dst, const GlpsSymbol *src);
    void glps_swap_symbol(GlpsSymbol *a, GlpsSymbol *b);
    void glps_copy_data(GlpsSymbol *dst, const GlpsSymbol *src);
    void glps_int2double_symbol(GlpsSymbol *p);
    GlpsSymbol *glps_new_symbol(const char* name);
    GlpsSymbol *glps_new_string(const char *name, const char *s);
    GlpsSymbol *glps_new_int_a(const char *name, const int *v, size_t n);
    GlpsSymbol *glps_new_int(const char *name, int v);
    GlpsSymbol *glps_new_double_a(const char *name, const double *v, size_t n);
    GlpsSymbol *glps_new_double(const char *name, double v);
    GlpsSymbol *glps_add_symbol(struct pcdata *p, GlpsSymbol *symb);

    int glps_parse(struct pcdata *pp, const char *fname);
    void glps_free(struct pcdata *pp);

    /* int eval(struct pcdata *pp, GlpsSymbol *r, GlpsAst *a); */

    void dumpast(FILE* buf, struct pcdata *pp);

#ifdef __cplusplus
}
#endif

#endif

// Local Variables:
// mode: c
// tab-width: 4
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
