#####################################################################
#
# CAS CS 320, Fall 2014
# Assignment 3 (skeleton code)
# parse.py
#

import re

def parse(seqs, tmp, top = True):
    for (label, seq) in seqs:
        tokens = tmp[0:]
        (ss, es) = ([], [])
        for x in seq:
            if type(x) == type(""):
                if tokens[0] == x:
                    tokens = tokens[1:]
                    ss = ss + [x]
                else: break
            else:
                r = x(tokens, False)
                if not r is None:
                    (e, tokens) = r
                    es = es + [e]
        if len(ss) + len(es) == len(seq) and (not top or len(tokens) == 0):
            return ({label:es} if len(es) > 0 else label, tokens)

def number(tokens, top = True):
    if re.compile(r"(0|[1-9][0-9]*)").match(tokens[0]):
        return ({"Number": [int(tokens[0])]}, tokens[1:])

def variable(tokens, top = True):
    if re.compile(r"[a-z][A-Za-z]*").match(tokens[0]):
        return ({"Variable": [tokens[0]]}, tokens[1:])

def formula(tmp, top = True):
    tokens = tmp[0:]
    r = leftFormula(tokens, False)
    if not r is None:
        (e1, tokens) = r
        if len(tokens) > 0 and tokens[0] == 'and':
            r = formula(tokens[1:], False)
            if not r is None:
                (e2, tokens) = r
                return ({'And':[e1,e2]}, tokens)
        elif len(tokens) > 0 and tokens[0] == 'or':
            r = formula(tokens[1:], False)
            if not r is None:
                (e2, tokens) = r
                return ({'Or':[e1,e2]}, tokens)
        else:
            return (e1, tokens)

def leftFormula(tmp, top = True):
    r = parse([\
        ('True', ['true']),\
        ('False', ['false']),\
        ('Not',  ['not', '(', formula, ')']),\
        ], tmp, top)
    if not r is None:
        return r

    tokens = tmp[0:]
    if tokens[0] == '(':
        r = formula(tokens[1:], False)
        if not r is None:
            (e, tokens) = r
            if tokens[0] == ')':
                return (e, tokens[1:])

    tokens = tmp[0:]
    r = variable(tokens, False)
    if not r is None:
        return r

def term(tmp, top = True):
    tokens = tmp[0:]
    r = leftTerm(tokens, False)
    if not r is None:
        (e1, tokens) = r
        if len(tokens) > 0 and tokens[0] == '+':
            r = term(tokens[1:], False)
            if not r is None:
                (e2, tokens) = r
                return ({'Plus':[e1,e2]}, tokens)
        else:
            return (e1, tokens)

def leftTerm(tmp, top = True):
    tokens = tmp[0:]
    if tokens[0] == '(':
        r = term(tokens[1:], False)
        if not r is None:
            (e, tokens) = r
            if tokens[0] == ')':
                return (e, tokens[1:])

    tokens = tmp[0:]
    r = variable(tokens, False)
    if not r is None:
        return r

    tokens = tmp[0:]
    r = number(tokens, False)
    if not r is None:
        return r

def program(tmp, top = True):
    if len(tmp) == 0:
        return ('End', [])
    r = parse([\
        ('Print', ['print', formula, ';', program]),\
        ('Print', ['print', term, ';', program]),\
        ('Assign',  [variable, ':=', formula, ';', program]),\
        ('Assign',  [variable, ':=', term, ';', program]),\
        ('Assign',  ['assign', variable, ':=', formula, ';', program]),\
        ('Assign',  ['assign', variable, ':=', term, ';', program]),\
        ('If',  ['if', formula, '{', program, '}', program]),\
        ('If',  ['if', term, '{', program, '}', program]),\
        ('While',  ['while', formula, '{', program, '}', program]),\
        ('While',  ['while', term, '{', program, '}', program]),\
        ('Procedure',  ['procedure', variable, '{', program, '}', program]),\
        ('Call',  ['call', variable, ';', program]),\
        ('End', [])
        ], tmp, top)
    if not r is None:
        return r

def tokenizeAndParse(s):
    tokens = re.split(r"(\s+|assign|:=|print|\+|if|while|{|}|;|true|false|call|procedure|not|and|or|\(|\))", s)
    tokens = [t for t in tokens if not t.isspace() and not t == ""]
    (p, tokens) = program(tokens)
    return p

#eof