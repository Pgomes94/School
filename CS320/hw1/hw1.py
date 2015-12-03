#Patrick Gomes
#CS320 HW1

import re

def tokenize(terminals, characters):
    
    terminals = [re.escape(t) for t in terminals]

    terminals = '|'.join(terminals)
    terminals = '(' + terminals + ')'
    
    tokens = [t for t in re.split(terminals, characters)]
    return [t for t in tokens if not t.isspace() and not t ==""]

def directions(tokens):
    if tokens[0] == ';':
        return( directions(tokens[1:]))
    
    if tokens[0] == 'forward':
        return (dict({'Forward': [directions(tokens[1:])]}))
    
    if tokens[0] == 'reverse':
        return (dict({'Reverse': [directions(tokens[1:])]}))

    if tokens[0] == 'left':
        return (dict({'LeftTurn': [directions(tokens[2:])]}))
    if tokens[0] == 'right':
        return (dict({'RightTurn': [directions(tokens[2:])]}))

    if tokens[0] == 'stop':
        return (tokens[0].title())

def number(tokens, y= False):
    if re.match(r"^\-?([1-9][0-9]*)$", tokens[0]):
        return ({"Number": [int(tokens[0])]}, tokens[1:])

def variable(tokens, y =False):
    if re.match(r"([a-z]*[A-Z]*)$", tokens[0]):
        return ({"Variable": [tokens[0]]}, tokens[1:])

def term(tokens):
    if tokens[0] == "#":
        return (number(tokens[1:])[0], tokens[2:])
    
    if tokens[0] == "@":
        return (variable(tokens[1:])[0], tokens[2:])
    
    if tokens[0] == "log" and tokens[1] =='(':
        (e1, tokens) = term(tokens[2:])
        if tokens[0] == ')':
            return ({'Log':[e1]}, tokens[1:])
        
    if tokens[0] == "plus" and tokens[1] == '(':
        (e1, tokens) = term(tokens[2:])
        if tokens[0] == ',':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] == ')':
                return ({"Plus":[e1,e2]}, tokens[1:])
            
    if tokens[0] == "mult" and tokens[1] == '(':
        (e1, tokens) = term(tokens[2:])
        if tokens[0] == ',':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] == ')':
                return ({"Mult":[e1,e2]}, tokens[1:])

    if tokens[0] =='(':
        tokens= tokens[1:]
        r = term(tokens)
        if not r is None:
            (e1, tokens) =r
            if tokens[0] == '+':
                tokens = tokens[1:]
                r = term(tokens)
                if not r is None:
                    (e2, tokens) = r
                    if tokens[0] == ')':
                        return ({"Plus":[e1,e2]}, tokens[1:])
            elif tokens[0] == '*':
                tokens = tokens[1:]
                r = term(tokens)
                if not r is None:
                    (e2, tokens) = r
                    if tokens[0] == ')':
                        return ({"Mult":[e1,e2]}, tokens[1:])

def formula(tokens):
    if tokens[0] == "true":
        return ('True',tokens[1:])
    
    if tokens[0] == "false":
        return ('False',tokens[1:])

    if tokens[0] == 'not' and tokens[1] == '(':
        (e1, tokens) = formula(tokens[2:])
        if tokens[0] == ')':
          return ({'Not':[e1]}, tokens[1:])

    if tokens[0] == 'or' and tokens[1] == '(':
        (e1, tokens) = formula(tokens[2:])
        if tokens[0] == ',':
            (e2, tokens) = formula(tokens[1:])
            if tokens[0] == ')':
                return ({'Or':[e1,e2]}, tokens[1:])

    if tokens[0] == 'and' and tokens[1] == '(':
        (e1, tokens) = formula(tokens[2:])
        if tokens[0] == ',':
            (e2, tokens) = formula(tokens[1:])
            if tokens[0] == ')':
                return ({'And':[e1,e2]}, tokens[1:])

        
    if tokens[0] == "equal" and tokens[1] == '(':
        (e1, tokens) = term(tokens[2:])
        if tokens[0] == ',':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] == ')':
                return ({"Equal":[e1,e2]}, tokens[1:])

    if tokens[0] == "less" and tokens[1] == 'than' and tokens[2] == '(':
        (e1, tokens) = term(tokens[3:])
        if tokens[0] == ',':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] == ')':
                return ({"LessThan":[e1,e2]}, tokens[1:])
    if tokens[0] == "greater" and tokens[1] == 'than' and tokens[2] == '(':
        (e1, tokens) = term(tokens[3:])
        if tokens[0] == ',':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] == ')':
                return ({"GreaterThan":[e1,e2]}, tokens[1:])

    if tokens[0] =='(':
        tokens= tokens[1:]
        r = term(tokens)
        if not r is None:
            (e1, tokens) =r
            if tokens[0] == '==':
                tokens = tokens[1:]
                r = term(tokens)
                if not r is None:
                    (e2, tokens) = r
                    if tokens[0] == ')':
                        return ({"Equal":[e1,e2]}, tokens[1:])
            elif tokens[0] == '<':
                tokens = tokens[1:]
                r = term(tokens)
                if not r is None:
                    (e2, tokens) = r
                    if tokens[0] == ')':
                        return ({"LessThan":[e1,e2]}, tokens[1:])
            elif tokens[0] == '>':
                tokens = tokens[1:]
                r = term(tokens)
                if not r is None:
                    (e2, tokens) = r
                    if tokens[0] == ')':
                        return ({"GreaterThan":[e1,e2]}, tokens[1:])
        else:
            r = formula(tokens)
            if not r is None:
                (e1, tokens) = r
                if tokens[0] == "&&":
                    tokens = tokens[1:]
                    r = formula(tokens)
                    if not r is None:
                        (e2, tokens) = r
                        if tokens[0] == ')':
                            return ({'And':[e1,e2]}, tokens[1:])
                elif tokens[0] == '||':
                    tokens = tokens[1:]
                    r = formula(tokens)
                    if not r is None:
                        (e2, tokens) = r
                        if tokens[0] == ')':
                            return ({'Or':[e1,e2]}, tokens[1:])
                    

def program(tokens):
    if tokens[0] =='print':
        tokens= tokens[1:]
        r = formula(tokens)
        if not r is None:
            (e1, tokens) =r
            if tokens[0] == ';':
                return ({'Print':[e1, program(tokens[1:])]})
        else:
            r = term(tokens)
            if not r is None:
                (e1, tokens) =r
                if tokens[0] == ';':
                    return ({'Print':[e1, program(tokens[1:])]})
    
    if tokens[0] == 'assign' and tokens[1]=='@':
        (e1, tokens) = variable(tokens[2:])
        if tokens[0] == ':=':
            (e2, tokens) = term(tokens[1:])
            if tokens[0] ==';':
                return ({'Assign':[e1,e2,program(tokens[1:])]})
    if tokens[0] == 'end' and tokens[1] == ';':
        return("End")

def complete(tokens):
    regex = ['print' , 'assign', '@', 'end', 'true', 'false', 'not', 'and', 'or', 'equal',\
             'less', 'greater', 'than', 'plus', 'mult', 'log', '#', '(', ')', ',',':=',';', ' ','&&','||','>','<','==','+','*']
    tokens = tokenize(regex, tokens)
    return program(tokens)
