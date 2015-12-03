from math import log, floor
import re

def tokenize(terminals, characters):
    
    terminals = [re.escape(t) for t in terminals]

    terminals = '|'.join(terminals)
    terminals = '(' + terminals + ')'
    
    tokens = [t for t in re.split(terminals, characters)]
    return [t for t in tokens if not t.isspace() and not t ==""]

def variable(tokens):
	if re.match(r"^([a-z][a-zA-Z0-9_]*)$", tokens[0]):
		return (tokens[0], tokens[1:])

def number(tokens):
        if re.match(r"^-?[0-9]*$", tokens[0]):
                if (tokens[0] == '-'):
                        return ((int)(tokens[0:2]), tokens[2:])
                return ((int)(tokens[0]), tokens[1:])

def formula(tokens):
        r = left(tokens)
        if not r is None:
                (e1, tokens) = r
                if len(tokens) > 0 and tokens[0] == 'xor':
                        r = formula(tokens[1:])
                        return ({'Xor': [e1, r[0]]}, r[1])
                if not (e1, tokens) is None:
                        return (e1, tokens)
        return None

def left(tokens):
        if(tokens[0] == 'true'):
                return ('True', tokens[1:])

        if(tokens[0] == 'false'):
           return('False', tokens[1:])

        if tokens[0] =='not' and tokens[1] == '(':
                tokens = tokens[2:]
                r = formula(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ')':
                                return ({'Not': [e1]}, tokens[1:])
        r = variable(tokens)
        if not r is None: 
                return ({'Variable': [r[0]]}, r[1])

        if tokens[0] == '(':
                tokens = tokens[1:]
                r = formula(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ')':
                                return ({'Parens': [e1]}, tokens[1:])

def term(tokens):
        r = factor(tokens)
        if not r is None:
                (e1, tokens) = r
                if len(tokens) > 0 and tokens[0] == '+':
                        r = term(tokens[1:])
                        if not r is None:
                                return ({'Plus': [e1, r[0]]}, r[1])
                else:
                        return (e1, tokens)
        
def factor(tokens):
        (e1, tokens) = left2(tokens)
        if len(tokens) >0 and tokens[0] == '*':
                r = factor(tokens[1:])
                return({'Mult': [e1, r[0]]}, r[1])
        return(e1, tokens)

def left2(tokens):
        if tokens[0] == 'log' and tokens[1] == '(':
                tokens = tokens[2:]
                r = term(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ')':
                                return ({'Log':[e1]}, tokens[1:])
        if tokens[0] == '(':
                r = term(tokens[1:])
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ')':
                                return ({'Parens': [e1]}, tokens[1:])

        r = variable(tokens)
        if not r is None:
                return ({'Variable': [r[0]]}, r[1])

        r = number(tokens)
        if not r is None:
                return ({'Number': [r[0]]}, r[1])

def program(tokens):
        if len(tokens) == 0:
                return ('End',tokens)
        if tokens[0] == 'print':
                tokens = tokens[1:]
                r = expressions(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ';':
                                r = program(tokens[1:])
                                return ({'Print': [e1, r[0]]}, r[1])
        if tokens[0] == 'assign':
                tokens= tokens[1:]
                r = variable(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == ':=':
                                tokens= tokens[1:]
                                r = expressions(tokens)
                                if not r is None:
                                        (e2, tokens) = r
                                        if tokens[0] == ';':
                                                r = program(tokens[1:])
                                                return({'Assign': [{'Variable':[e1]}, e2, r[0]]},r[1])
        if tokens[0] == 'if':
                tokens = tokens[1:]
                r = expressions(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == '{':
                                tokens= tokens[1:]
                                r = program(tokens)
                                if not r is None:
                                        (e2, tokens) = r
                                        if tokens[0] == '}':
                                                tokens = tokens[1:]
                                                r = program(tokens)
                                                return ({'If': [e1, e2,r[0]]}, r[1])

        if tokens[0] == 'while':
                tokens = tokens[1:]
                r = expressions(tokens)
                if not r is None:
                        (e1, tokens) = r
                        if tokens[0] == '{':
                                tokens= tokens[1:]
                                r = program(tokens)
                                if not r is None:
                                        (e2, tokens) = r
                                        if tokens[0] == '}':
                                                tokens = tokens[1:]
                                                r = program(tokens)
                                                return ({'While': [e1, e2,r[0]]}, r[1])
        return ('End', tokens)
                        
                

def expressions(tokens):
        if tokens[1] != '*' and tokens[1] != '+' and tokens[0] != 'log':
                r = formula(tokens)
                if not r is None:
                        return r
                
        r = term(tokens)
        if not r is None:
                return r
        return None

