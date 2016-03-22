#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

struct pcdata;

#include "parser.tab.h"
#include "scanner.lex.h"
#include "glpsfuncs.h"

/* in C++ Boost log */
void log_message(int level, const char *file, int line, const char *func, 
                 const char *msg);

#ifdef _MSC_VER
#define DBG_MSG(msg, ...)                                               \
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
#define DBG_MSG(msg, args ...)                                          \
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

/* interface to scanner */
void yyerror(struct pcdata *pp, char *s, ...)
{
}

/*!
 * \brief case-insensitive compare: similar return convention as strcmp.
 */
int str_case_cmp(const char* ps1, const char* ps2)
{
    char c1, c2;
    int v;
    if (ps1 == NULL && ps2 == NULL) return 0;
    else if (ps1 == NULL && ps2) return -1;
    else if (ps1 && ps2 == NULL) return 1;

    do {
        c1 = *ps1++;
        c2 = *ps2++;
        /* convert to small case to compare */
        if (c1 >='A' && c1 <= 'Z') c1 += 'a' - 'A';
        if (c2 >='A' && c2 <= 'Z') c2 += 'a' - 'A';
        v = (int) c1 - (int) c2;
    } while ((v==0) && (c1 != '\0'));

    return v;
}

/* = strdup */
char *str_duplicate(const char *s) 
{
    size_t len = 0;
    char *p = NULL;
    if (!s) return NULL;
    len = strlen(s) + 1;
    p = malloc(len);
    return p ? memcpy(p, s, len) : NULL;
}

/* alloc a new one if not found */
GlpsSymbol *glps_lookup(struct pcdata *pp, const char *name)
{
 #ifdef DEBUG
    DBG_MSG("searching: %s\n", name);
 #endif
    GlpsSymbol *sb = NULL;
    GlpsSymbol *p = pp->symtab;
    int i = 0;
    while(p) {
        if(!str_case_cmp(name, p->name)) return p;
        p = p->prev;
        ++i;
    }
 #ifdef DEBUG
    DBG_MSG("new: %s@%d\n", name, i);
 #endif
    sb = malloc(sizeof(GlpsSymbol));
    sb->name    = str_duplicate(name);
    sb->dtype   = GLPS_NODE_NONE;
    sb->memsize = 0;
    sb->nov     = 0;
    sb->val     = NULL;
    
    /* new at the head, adjust the head pointer */
    sb->prev = pp->symtab;
    pp->symtab = sb;

    return sb;
}

void glps_free_symbol(GlpsSymbol *p)
{
    if (p->name) free(p->name);
    /* DBG_MSG("free '%s'\n", (*p)->name); */
    if (p->val) free(p->val);
    free(p);
}

void glps_swap_symbol(GlpsSymbol *a, GlpsSymbol *b)
{
	int i = 0;
	size_t sz = 0;
	void *pv = NULL;
    char *p = a->name;
    a->name = b->name;
    b->name = p;

    i = a->dtype;
    a->dtype = b->dtype;
    b->dtype = i;

    sz = a->memsize;
    a->memsize = b->memsize;
    b->memsize = sz;

    sz = a->nov;
    a->nov = b->nov;
    b->nov = sz;

    pv = a->val;
    a->val = b->val;
    b->val = pv;
}

void glps_print_symbol(const GlpsSymbol *p, int level,
                       FILE *out, const char *tail)
{
    size_t i = 0;
    if(!p) {
        DBG_MSG("NULL pointer!\n");
        return;
    }

    fprintf(out, "%*s", 2*level, "");   /* indent to this level */
    fprintf(out, "%s ", p->name);
    switch(p->dtype) {
    case GLPS_NODE_CHAR:
        fprintf(out, "(char:%d)", p->dtype);
        for(i = 0; i < p->nov; ++i)
            fprintf(out, " %c",*((int*)(p->val)+i));
        break;
    case GLPS_NODE_INT:
        fprintf(out, "(int:%d)", p->dtype);
        for(i = 0; i < p->nov; ++i)
            fprintf(out, " %d",*((int*)(p->val)+i));
        break;
    case GLPS_NODE_DOUBLE:
        fprintf(out, "(double:%d)", p->dtype);
        for(i = 0; i < p->nov; ++i)
            fprintf(out, " %g",*((double*)(p->val)+i));
        break;
    case GLPS_NODE_LINE:
        fprintf(out, "(line:%d)", p->dtype);
        for(i = 0; i < p->nov; ++i)
            fprintf(out, " %c",*((int*)(p->val)+i));
    case GLPS_NODE_TAG:
        fprintf(out, "(tag)"); break;
    default:
        fprintf(out, "%s (%d) : ERROR", p->name, p->dtype); break;
    }
    if (tail) fprintf(out, "%s", tail);
    fflush(out);
}

void glps_int2double_symbol(GlpsSymbol *p)
{
    size_t i = 0;
	size_t memsz = 0;
	double *v = NULL;
	int *vi = NULL;

	if (p->dtype == GLPS_NODE_DOUBLE) return;

    memsz = p->nov * sizeof(double);
    v = (double*)malloc(memsz);
    vi = (int*) p->val;
    for (i = 0; i < p->nov; ++i) v[i] = vi[i];
    free(p->val);
    p->val = v;
    p->memsize = memsz;
    p->dtype = GLPS_NODE_DOUBLE;
}


void glps_copy_data(GlpsSymbol *dst, const GlpsSymbol *src)
{
    if (strlen(dst->name) < strlen(src->name)) {
        dst->name = realloc(dst->name, strlen(src->name) + 1);
    }
    strcpy(dst->name, src->name);
    dst->dtype = src->dtype;
    if (dst->memsize < src->memsize) {
        dst->val = realloc(dst->val, src->memsize);
        dst->memsize = src->memsize;
    }
    memcpy(dst->val, src->val, src->memsize);
    dst->nov = src->nov;
    /* does not touch prev pointer */
}

void glps_copy_symbol(GlpsSymbol *dst, const GlpsSymbol *src)
{
    if(dst->name) free(dst->name);
    dst->name = src->name ? str_duplicate(src->name) : NULL;
    glps_copy_data(dst, src);
}


GlpsSymbol *glps_new_symbol(const char* name)
{
    GlpsSymbol *sb = (GlpsSymbol*)malloc(sizeof(GlpsSymbol));

    sb->name = name ? str_duplicate(name) : NULL;
    sb->dtype   = GLPS_NODE_NONE;
    sb->memsize = 0;
    sb->nov     = 0;
    sb->val     = NULL;
    sb->prev    = NULL;
    return sb;
}

GlpsSymbol *glps_new_int_a(const char *name, const int *val, size_t n)
{
    GlpsSymbol *sb = (GlpsSymbol*)malloc(sizeof(GlpsSymbol));
    sb->name = name ? str_duplicate(name) : NULL;
    sb->dtype = GLPS_NODE_INT;
    sb->memsize = sizeof(int) * n;
    sb->val = n > 0 ? malloc(sb->memsize) : NULL;
    sb->nov = n;
    memcpy(sb->val, val, sb->memsize);

    return sb;
}

GlpsSymbol *glps_new_int(const char *name, int v)
{
    return glps_new_int_a(name, &v, 1);
}

GlpsSymbol *glps_new_double_a(const char *name, const double *val, size_t n)
{
    GlpsSymbol *sb = (GlpsSymbol*)malloc(sizeof(GlpsSymbol));
    sb->name = name ? str_duplicate(name) : NULL;
    sb->dtype = GLPS_NODE_DOUBLE;
    sb->memsize = sizeof(double) * n;
    sb->val = n > 0 ? malloc(sb->memsize) : NULL;
    sb->nov = n;
    memcpy(sb->val, val, sb->memsize);

    return sb;
}

GlpsSymbol *glps_new_double(const char *name, double val)
{
    return glps_new_double_a(name, &val, 1);
}

GlpsSymbol *glps_new_string(const char *name, const char *s)
{
    GlpsSymbol *sb = (GlpsSymbol*)malloc(sizeof(GlpsSymbol));
    sb->name = str_duplicate(name);
    sb->dtype = GLPS_NODE_STRING;
    sb->nov = strlen(s);
    sb->memsize = sb->nov + 1;
    sb->val = malloc(sb->memsize);
    memcpy(sb->val, s, sb->memsize);

    return sb;
}

GlpsSymbol *glps_add_symbol(struct pcdata *pp, GlpsSymbol *s)
{
    s->prev = pp->symtab;
    pp->symtab = s;
    return pp->symtab;
}

/* build the AST */
int glps_parse(struct pcdata *p, const char *fname)
{
    FILE *inp = NULL;
    int err = GLPS_SUCCESS;
    if(yylex_init_extra(p, &(p->scaninfo))) {
        DBG_MSG("init alloc failed\n");
        return GLPS_FAILURE;
    }

    /* glps_add_symbol(p, glps_new_double("pi", M_PI)); */

    if ( !(p->al = calloc(GLPS_MAX_AST, sizeof(struct glps_ast*))) ) {
        DBG_MSG("ast list alloc failed\n");
        return GLPS_FAILURE;
    }

    /* FILE *inp = NULL; */
    if ( !(inp = fopen(fname, "r")) ) {
        DBG_MSG("failed open file '%s'\n", fname);
        return GLPS_FAILURE;
    }

    /* set the input */
    yyset_in(inp, p->scaninfo);
    push_lattice(p, fname); /* guranteed capacity > 1 */
    while(1) {
        int err1 = yyparse(p, p->scaninfo);
        if (err1 != 0) {
            LOG_FATAL("failed parsing '%s' (errcode=%d)", fname, err1);
            err = GLPS_FAILURE;
            break;
        } else {
            LOG_DEBUG("parsed '%s' with errcode= %d", fname, err1);
        }

        inp = yyget_in(p->scaninfo);
     #ifndef NDEBUG
        DBG_MSG("done with parsing %d (%s)\n", feof(inp), fname);
     #endif
        if(feof(inp)) break;
    }

    /* cleanup */
    fclose(inp);

    /* if (buf) *buf = dumpast(p); */
    FILE *buf = fopen("ast.debug", "w");
    dumpast(buf, p);
    fclose(buf);
    
 #ifdef DEBUG
    GlpsSymbol *sb = p->symtab;
    while(sb) {
        printf("%s (%d)\n", sb->name, sb->dtype);
        sb = sb->prev;
    }
 #endif

    yylex_destroy(p->scaninfo);
    
    return err;
}

GlpsAst *glps_new_intval(struct pcdata *pp, int val)
{
    GlpsIntVal *a = malloc(sizeof(GlpsIntVal));
    a->nodetype = GLPS_NODE_INT;
    a->i = val;
    return (GlpsAst*)a;
}

GlpsAst *glps_new_doubleval(struct pcdata *pp, double val)
{
    GlpsDoubleVal *a = malloc(sizeof(GlpsDoubleVal));
    a->nodetype = GLPS_NODE_DOUBLE;
    a->d = val;
    return (GlpsAst*)a;
}

GlpsAst *glps_new_strval(struct pcdata *pp, char *s)
{
    GlpsStrVal *a = malloc(sizeof(GlpsStrVal));
    a->nodetype = GLPS_NODE_STRING;
    a->s = s;
    return (GlpsAst*) a;
}

GlpsAst *glps_new_varref(struct pcdata *pp, GlpsSymbol *s)
{
    GlpsVarRef *a = malloc(sizeof(GlpsVarRef));
    a->nodetype = GLPS_NODE_VARIABLE;
    a->s = s;
    return (GlpsAst*) a;
}

GlpsAst *glps_new_assign(struct pcdata *pp, GlpsSymbol *s, GlpsAst *v)
{
    struct glps_assign *a = malloc(sizeof(struct glps_assign));
    a->nodetype = GLPS_NODE_ASSIGN;
    a->symb = s; 
    a->v = v;

    return (GlpsAst *)a;
}

GlpsAst *glps_new_prpt(struct pcdata *pp, char *s, GlpsAst *v)
{
    struct glps_prpt *a = malloc(sizeof(struct glps_prpt));
    a->nodetype = GLPS_NODE_PROPERTY;
    a->name = s;
    a->v = v;

    return (GlpsAst *)a;
}

/*
GlpsAstList *newprptlist(struct pcdata *pp, GlpsAst *ast, GlpsAstList *next)
{
    GlpsAstList *al = malloc(sizeof(GlpsAstList));
    al->nodetype = GLPS_NODE_PRPTLIST;
    al->ast = ast;
    al->next = next;

    return al;
}
*/

void add_prefix(struct pcdata *pp, const char *s, GlpsAst *ast)
{
    switch(ast->nodetype) {
    case GLPS_NODE_PROPERTY:
     #ifndef NDEBUG
        DBG_MSG("add prefix: %s\n", s);
     #endif
        {
            GlpsAssign *a = (GlpsAssign*) ast;
            size_t sz = strlen(s) + strlen(a->symb->name) + 2;
            char *name = (char*) malloc(sz);
            name[0] = '\0';
            strcat(name, s);
            strcat(name, ".");
            strcat(name, a->symb->name);
            free(a->symb->name);
            a->symb->name = name;
        }
        break;
    case GLPS_NODE_CONCAT_PRPT:
        add_prefix(pp, s, ast->l);
        add_prefix(pp, s, ast->r);
        break;
    default:
        /* DBG_MSG("skip: %d\n", ast->nodetype); */
        break;
    }
}

GlpsAst *glps_new_action(struct pcdata *pp, char *s, GlpsAst *a)
{
    GlpsAction *ac = (GlpsAction*)malloc(sizeof(GlpsAction));
    ac->nodetype = GLPS_NODE_ACTION;
    ac->name = s;
    ac->prpt = a;
    /* add_prefix(pp, s, a); */
    return (GlpsAst*) ac;
}

GlpsAst *glps_new_ast(struct pcdata *pp, int nt, GlpsAst *l, GlpsAst *r)
{
    GlpsAst *a = malloc(sizeof(GlpsAst));
    a->nodetype = nt;
    a->l = l;
    a->r = r;

    return a;
}

GlpsAst *glps_new_element(struct pcdata *pp, char *name,
                          char *fam, GlpsAst *a)
{
    GlpsElement *ae = (GlpsElement*) malloc(sizeof(GlpsElement));
    ae->nodetype = GLPS_NODE_ELEMENT;
    ae->name = name;
    ae->family = fam;
    ae->prpt = a;
    /* if(a) add_prefix(pp, name, a); */

    return (GlpsAst*) ae;
}

GlpsAst *glps_new_line(struct pcdata *pp, char *name, GlpsAst *a)
{
    GlpsLine *al = malloc(sizeof(GlpsLine));
    al->nodetype = GLPS_NODE_LINE;
    al->name = name;
    al->line = a;

    return (GlpsAst*) al;
}

GlpsAst *glps_new_lineelem(struct pcdata *pp, char *s)
{
    GlpsStrVal *a = malloc(sizeof(GlpsStrVal));
    a->nodetype = GLPS_NODE_LINEELEM;
    a->s = s;
    
    return (GlpsAst*) a;
}

#ifdef _MSC_VER
#define DUMP_AST(buf, ns, msg, ...)                                        \
    { fprintf(buf, msg, ##__VA_ARGS__);}
#else
#define DUMP_AST(buf, ns, msg ...)                                        \
    { fprintf(buf, msg); }
#endif

static int binop_name(int op) {
    switch(op) {
    case GLPS_NODE_FUNC: return 'F';    /* function, f(x) */
    case GLPS_NODE_FADD: return '+';    /* '+' */
    case GLPS_NODE_FSUB: return '-';    /* '-' */
    case GLPS_NODE_FMULT: return '*';   /* '*' */
    case GLPS_NODE_FDIV: return '/';    /* '/' */
    case GLPS_NODE_FINV: return '-';    /* inverse  */
    case GLPS_NODE_FEXP: return 'e';    /* exp(x) */
    case GLPS_NODE_FLOG: return 'L';
    case GLPS_NODE_FSQRT: return 'S';
    case GLPS_NODE_FPRINT: return 'P';
    }
    return ' ';
}
void dump_ast_node(FILE *buf, GlpsAst *a, int level)
{
    int ns = 0;
    /* printf("%*s", 2*level, "");*/   /* indent to this level */
    DUMP_AST(buf, ns, "%*s", 2*level, "");
    ++level;

    if (!a) {
        DUMP_AST(buf, ns, "NULL\n");
        return;
    }

    switch(a->nodetype) {
    case GLPS_NODE_ASSIGN:
        LOG_DEBUG("assignment: %s, lvl= %d", ((GlpsAssign*)a)->symb->name, level);
        DUMP_AST(buf, ns, "= %s\n", ((GlpsAssign*)a)->symb->name);
        dump_ast_node(buf, ((GlpsAssign*)a)->v, level);
        break;
    case GLPS_NODE_PROPERTY:
        DUMP_AST(buf, ns, "= %s\n", ((GlpsProperty*)a)->name);
        dump_ast_node(buf, ((GlpsProperty*)a)->v, level);
        break;
    case GLPS_NODE_DOUBLE:
        DUMP_AST(buf, ns, "%g (double)\n", ((GlpsDoubleVal*)a)->d);
        break;
    case GLPS_NODE_INT:
        DUMP_AST(buf, ns, "%d (int)\n", ((GlpsIntVal*)a)->i);
        break;
    case GLPS_NODE_FADD:
    case GLPS_NODE_FSUB:
    case GLPS_NODE_FMULT:
    case GLPS_NODE_FDIV:
        DUMP_AST(buf, ns, "%c\n", binop_name(a->nodetype));
        dump_ast_node(buf, a->l, level);
        dump_ast_node(buf, a->r, level);
        break;
    case GLPS_NODE_FINV:
        DUMP_AST(buf, ns, "uniop inv\n");
        dump_ast_node(buf, a->l, level);
        break;
    case GLPS_NODE_STRING:
        DUMP_AST(buf, ns, "\"%s\"\n", ((GlpsStrVal*)a)->s);
        break;
    case GLPS_NODE_VARIABLE:
        DUMP_AST(buf, ns, "%s\n", ((GlpsVarRef*)a)->s->name);
        break;
    case GLPS_NODE_CONCAT_EXPR:
    case GLPS_NODE_CONCAT_PRPT:
        DUMP_AST(buf, ns, ",\n");
        dump_ast_node(buf, a->l, level);
        dump_ast_node(buf, a->r, level);
        break;
    case GLPS_NODE_ACTION:
        {
            GlpsAction *ac = (GlpsAction*) a;
            DUMP_AST(buf, ns, "act: %s\n", ac->name);
            dump_ast_node(buf, ac->prpt, level);
        }
        break;
    case GLPS_NODE_ELEMENT:
        {
            GlpsElement *ae = (GlpsElement*) a;
            DUMP_AST(buf, ns, "element: %s, %s\n", ae->name, ae->family);
            dump_ast_node(buf, ae->prpt, level);
        }
        break;
    case GLPS_NODE_LINE:
        DUMP_AST(buf, ns, "line '%s':\n", ((GlpsLine*)a)->name);
        dump_ast_node(buf, ((GlpsLine*)a)->line, level);
        break;
    case GLPS_NODE_CONCAT_LINE:
        DUMP_AST(buf, ns, ",\n");
        dump_ast_node(buf, a->l, level);
        dump_ast_node(buf, a->r, level);
        break;
    case GLPS_NODE_LINEELEM:
        DUMP_AST(buf, ns, "%s\n", ((GlpsStrVal*)a)->s);
        break;
    case GLPS_NODE_LINE_INV:
        DUMP_AST(buf, ns, "inv\n");
        dump_ast_node(buf, a->l, level);
        break;
    case GLPS_NODE_LINE_MULT:
        DUMP_AST(buf, ns, "*\n");
        dump_ast_node(buf, a->l, level);
        dump_ast_node(buf, a->r, level);
        break;
    default:
        DBG_MSG("invalid node: %d\n", a->nodetype);
        break;
    }

    return;
}

void dumpast(FILE *buf, struct pcdata *pp)
{
    int i = 0;
    for(i = 0; i < pp->nast; ++i) {
        fprintf(buf, "ast.%d\n", i);
        dump_ast_node(buf, pp->al[i], 2);
    }
}

void glps_free_ast(GlpsAst *a)
{
    if (!a) return;

    switch(a->nodetype) {
    case GLPS_NODE_ASSIGN:
        glps_free_ast(((GlpsAssign*)a)->v);
        break;
    case GLPS_NODE_PROPERTY:
        free(((GlpsProperty*)a)->name);
        glps_free_ast(((GlpsProperty*)a)->v);
        break;
    case GLPS_NODE_DOUBLE:
    case GLPS_NODE_INT:
        break;
    case GLPS_NODE_FADD:
    case GLPS_NODE_FSUB:
    case GLPS_NODE_FMULT:
    case GLPS_NODE_FDIV:
        glps_free_ast(a->l);
        glps_free_ast(a->r);
        break;
    case GLPS_NODE_FINV:
        glps_free_ast(a->l);
        break;
    case GLPS_NODE_STRING:
        free(((GlpsStrVal*)a)->s);
        break;
    case GLPS_NODE_VARIABLE:
        break;
    case GLPS_NODE_CONCAT_EXPR:
    case GLPS_NODE_CONCAT_PRPT:
        glps_free_ast(a->l);
        glps_free_ast(a->r);
        break;
    case GLPS_NODE_ACTION:
        free(((GlpsAction*)a)->name);
        glps_free_ast(((GlpsAction*)a)->prpt);
        break;
    case GLPS_NODE_ELEMENT:
        {
            GlpsElement *ae = (GlpsElement*) a;
            free(ae->name);
            free(ae->family);
            glps_free_ast(ae->prpt);
        }
        break;
    case GLPS_NODE_LINE:
        free(((GlpsLine*)a)->name);
        glps_free_ast(((GlpsLine*)a)->line);
        break;
    case GLPS_NODE_CONCAT_LINE:
        glps_free_ast(a->l);
        glps_free_ast(a->r);
        break;
    case GLPS_NODE_LINEELEM:
        free(((GlpsStrVal*)a)->s);
        break;
    case GLPS_NODE_LINE_INV:
        glps_free_ast(a->l);
        break;
    case GLPS_NODE_LINE_MULT:
        glps_free_ast(a->l);
        glps_free_ast(a->r);
        break;
    default:
        DBG_MSG("invalid node: %d\n", a->nodetype);
        break;
    }

    free(a);
    return;
}

void glps_free(struct pcdata *pp)
{
    int i = 0;
    struct glps_symbol *head = pp->symtab;
    while (head) {
        struct glps_symbol *r = head->prev;
        /* DBG_MSG("freeing '%s'\n", head->name); */
        free(head->name);
        free(head->val);
        free(head);
        head = r;
    }

    for (i = 0; i < pp->nast; ++i) {
        glps_free_ast(pp->al[i]);
    }

    free(pp->al);
}

// Local Variables:
// mode: c
// tab-width: 4
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
