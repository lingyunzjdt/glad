import sys
import re
import ply.lex as lex


"""
- lte may parse 'C6: KICKER,,L=0.15', but not here
"""
debug = 0
keywords = ('LINE', 'INCLUDE', 'SAVE')
tokens = keywords + (
    'ID', 'EQUALS', 'PLUS', 'MINUS', 'TIMES', 'DIVIDE', 'POWER',
    'LPAREN', 'RPAREN', 'COMMA', 'SEMI', 'INTEGER', 'FLOAT',
    'COLON', 'STRING', 'NEWLINE', 'END' )
literals = ":."
t_ignore = ' \t;'

def t_ID(t):
    r'[A-Z][A-Z_0-9]*'
    #if t.value in keywords:
    #    t.type = t.value
    if t.value.upper() in [x.upper() for x in keywords]:
        t.type = t.value.upper()
    return t

t_EQUALS = r'='
t_PLUS = r'\+'
t_MINUS = r'\-'
t_TIMES = r'\*'
t_DIVIDE = r'/'
t_POWER = r'\^'
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_COMMA = r'\,'
t_SEMI = r';'
t_INTEGER = r'\d+'
t_FLOAT = r'((\d*\.\d+)(E[\+-]?\d+)?|([1-9]\d*E[\+-]?\d+))'
t_STRING = r'\".*?\"'

def t_NEWLINE(t):
    r'&?\n'
    t.lexer.lineno += 1
    #return t

def t_COMMENT(t):
    r'!.*\n'
    t.lexer.lineno += 1
    pass

def t_error(t):
    print("Illegal character %s" % t.value[0])
    t.lexer.skip(1)

# ignore the case
lex.lex(debug=debug, reflags=int(re.VERBOSE|re.I))

#lex.input('x = 3 * 4')
#while True:
#    tok = lex.token()
#    if not tok: break
#    print(tok)


# parsing rules

# When shift/reduce conflicts are encountered, the parser generator resolves the conflict by looking at the precedence rules and associativity specifiers.
#
#    If the current token has higher precedence than the rule on the stack, it is shifted.
#    If the grammar rule on the stack has higher precedence, the rule is reduced.
#    If the current token and the grammar rule have the same precedence, the rule is reduced for left associativity, whereas the token is shifted for right associativity.
#    If nothing is known about the precedence, shift/reduce conflicts are resolved in favor of shifting (the default). 


precedence = (
    ('left', 'COMMA'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES', 'DIVIDE'),
    ('left', 'POWER'),
    ('right', 'UMINUS'), # given fictitious token
)

def print_nested_tuple(p, level=0):
    print(" " * (level*4), p[0])
    for x in p[1:]:
        if isinstance(x, (str, int, float)):
            print(" " * ((level+1) * 4), x)
        else:
            print_nested_tuple(x, level+1)


variables = {}
elements = {}
_the_line_ = None
lines = {}

def ucase_set(d, k, v):
    d[k.upper()] = v

def ucase_get(d, k, val=None):
    return d.get(k.upper(), val)

def p_statement_list(p):
    '''statements : 
                  | statements assignment
                  | statements element
                  | statements beamline'''
    global _the_line_
    if len(p) == 3:
        tag = p[2][0]
        if tag == 'ASSIGN' or tag == 'VASSIGN':
            name, val = p[2][1:]
            ucase_set(variables, name, val)
        elif tag == 'ELEMENT':
            #print(p[1], type(p[2]), p[2], p[2][1])
            name, elemtype, prpts = p[2][1:]
            ucase_set(elements, name, p[2])
        elif tag == 'LINE':
            # print('LINE', p[2])
            name, elemlist = p[2][1:]
            ucase_set(lines, name, elemlist)
            _the_line_ = name
        print(p[2])
        # print_nested_tuple(p[2], 0)
    pass

def p_assignment_assign(p):
    '''assignment : ID EQUALS expression
                  | ID EQUALS STRING'''
    p[0] = ('ASSIGN', p[1], p[3])


def p_expression_binop(p):
    '''expression : expression PLUS expression
                  | expression MINUS expression
                  | expression TIMES expression
                  | expression DIVIDE expression
                  | expression POWER expression'''
    # p[0] = ('BINOP', p[2], p[1], p[3])
    x1 = variables.get(p[1], p[1])
    x2 = variables.get(p[3], p[3])
    # print(x1, x2)
    if isinstance(x1, (tuple, list)) and isinstance(x2, (tuple, list)):
        if len(x1) == len(x2):
            if p[2] == '+':
                p[0] = tuple([x1[i] + x2[i] for i in range(len(x1))])
            elif p[2] == '-':
                p[0] = tuple([x1[i] - x2[i] for i in range(len(x1))])
            elif p[2] == '*':
                p[0] = tuple([x1[i] * x2[i] for i in range(len(x1))])
            elif p[2] == '/':
                p[0] = tuple([x1[i] / x2[i] for i in range(len(x1))])
    elif p[2] == '+': p[0] = x1 + x2
    elif p[2] == '-': p[0] = x1 - x2
    elif p[2] == '*': p[0] = x1 * x2
    elif p[2] == '/': p[0] = x1 / x2
    elif p[2] == '^': p[0] = x1 ** x2

def p_expression_group(p):
    '''expression : LPAREN expression RPAREN'''
    p[0] = p[2]

# override the default rule, setting it to that of 'UMINUS'
def p_expression_uminus(p):
    'expression : MINUS expression %prec UMINUS'
    p[0] = -p[2]


def p_expression_number(p):
    '''expression : FLOAT
                  | INTEGER'''
    p[0] = eval(p[1]) #('NUM', eval(p[1]))

def p_expression_id(p):
    'expression : ID'
    p[0] = variables.get(p[1], p[1]) #('VAR', p[1])

def p_expression_elemprpt(p):
    '''expression : ID "." ID'''
    tag, name, elemtype, prpts = elements[p[1]]
    p[0] = prpts[p[3]]

def p_assignment_exprvector(p):
    '''assignment : ID EQUALS LPAREN exprlist RPAREN'''
    p[0] = ('VASSIGN', p[1], p[4])
    # print(p[0])

def p_exprlist_2(p):
    '''exprlist : expression COMMA expression'''
    p[0] = (p[1], p[3])
    
def p_exprlist_group(p):
    '''exprlist : exprlist COMMA expression'''
    p[0] = p[1] + (p[3],)

def p_element(p):
    '''element : ID ":" ID'''
    p[0] = ('ELEMENT', p[1], p[3], {})

def p_element_elegant(p):
    '''element : STRING ":" ID'''
    # elegant allows '"LIN_P" : Drift'
    p[0] = ('ELEMENT', p[1][1:-1], p[3], {})
    
def p_element_properties(p):
    '''element : element COMMA assignment'''
    tag, name, elemtype, prpts = p[1]
    # print(p[3])
    if p[3][0] == 'ASSIGN':
        prpts[p[3][1]] = p[3][2]
    p[0] = (tag, name, elemtype, prpts)

def p_elemlist_bin(p):
    '''elemlist : ID
                | MINUS elemlist
                | INTEGER TIMES elemlist'''

    if len(p) == 2:
        if ucase_get(lines, p[1], None):
            elemline = ucase_get(lines, p[1])
            p[0] = tuple([p[1] + '.' + x for x in elemline])
        elif ucase_get(elements, p[1], None):
            p[0] = (p[1],)
        else:
            print(lines.keys())
            print(elements.keys())
            raise RuntimeError("can not find element/line '%s'" % p[1])
    elif len(p) == 3:
        p[0] = tuple(['-' + x for x in reversed(p[2]) ])
    else:
        p[0] = tuple([p[3] for i in range(eval(p[1]))])

def p_elemlist_pargroup(p):
    '''elemlist : LPAREN elemlist RPAREN'''
    p[0] = p[2]
    
def p_elemlist_group(p):
    '''elemlist : elemlist COMMA elemlist'''
    p[0] = p[1] + p[3]
    
def p_beamline(p):
    '''beamline : ID ":" LINE EQUALS LPAREN elemlist RPAREN'''
    p[0] = ('LINE', p[1], p[6])

def p_beamline_ltepatch(p):
    '''beamline : STRING ":" LINE EQUALS LPAREN elemlist RPAREN'''
    p[0] = ('LINE', p[1][1:-1], p[6])
    
def p_error(p):
    if p:
        print("Syntax error at '%s'" % p.value)
    else:
        print("Syntax error at EOF")


def expand_line(line, elemdict):
    fuline = []
    for elem in line:
        name = elem.replace('-', '')
        direction = (len(elem) - len(name)) % 2
        fam = name.split('.')
        if not ucase_get(elemdict, fam[-1]):
            raise RuntimeError("can not find element '%s'" % fam[-1])
        tag, origname, elemtype, prpts = ucase_get(elemdict, fam[-1])
        fuprpts = prpts.copy()
        fuprpts.update({'.TYPE': elemtype, '.NAME': fam[-1], '.FULL_NAME': elem, '.DIRECTION': direction})
        fuline.append(fuprpts)
    return fuline


# Set up a logging object
import logging
logging.basicConfig(
    level = logging.DEBUG,
    filename = "log.txt",
    filemode = "w",
    format = "%(filename)10s:%(lineno)4d:%(message)s"
)
log = logging.getLogger()

import ply.yacc as yacc
parser = yacc.yacc(write_tables=False, debug=debug, debuglog=log)

lat = open(sys.argv[1], 'r').read()
print(lat)
parser.parse(lat, debug=log)

# print(parser)
import json

print('Variables:\n', json.dumps(variables, sort_keys=True, indent=4, separators=(',', ':')))
print('Elements:\n', json.dumps(elements, sort_keys=True, indent=4, separators=(',', ':')))


#print('Lines:\n', json.dumps(lines, sort_keys=True, indent=4, separators=(',', ':')))
print("the line:", _the_line_)
if _the_line_:
    fuline = expand_line(ucase_get(lines, _the_line_), elements)
    print('Line:\n', json.dumps(fuline, sort_keys=True, indent=4, separators=(',', ':')))


