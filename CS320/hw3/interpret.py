######################################################################
#
# CAS CS 320, Fall 2014
# Assignment 3 (skeleton code)
# interpret.py
#

exec(open("parse.py").read())

Node = dict
Leaf = str

def evalTerm(env, t):
    if type(t) == dict:
        for label in t:
            children = t[label]
            if label == 'Number':
                x = children[0]
                v = evalTerm(env, x)
                return int(v)
            elif label == 'Variable':
                x = children[0]
                if x in env:
                    a = env[x]
                    v = evalTerm(env, a)
                    return v
                else:
                    print(x + " is unbound.")
                    exit()
            elif label == 'Plus':
                f1 = children[0]
                v1 = evalTerm(env, f1)
                f2 = children[1]
                v2 = evalTerm(env, f2)
                return v1 + v2
    else:
        return t

def evalFormula(env, f):
    if type(f) == dict:
        for label in f:
            children = f[label]
            if label == 'Not':
                f = children[0]
                v = evalFormula(env, f)
                return not v 
            elif label == 'Variable':
                x = children[0]
                if x in env:
                    a = env[x]
                    v = evalFormula ( env, a)
                    return v
                else:
                    print(x + " is unbound.")
                    exit()
            elif label == 'Or':
                f2 = children[1]
                v2 = evalFormula(env, f2)
                if v2 == True:
                    return v2
                f1 = children[0]
                v1 = evalFormula(env, f1)
                return v1 or v2
            elif label == 'And':
                f2 = children[1]
                v2 = evalFormula(env, f2)
                if v2 == False:
                    return v2
                f1 = children[0]
                v1 = evalFormula(env, f1)
                return v1 and v2 
    elif type(f) == str:
        if f == 'True':
            return True 
        if f == 'False':
            return False #		

def evalExpression(env, e): # Useful helper function.
    if type(e) == dict:
        e1 = evalTerm(env,e)
        if e1 is not None:
            return e1
        e1 = evalFormula(env, e)
        if e1 is not None:
            return e1

def execProgram(env, s):
    if type(s) == str:
        if s == 'End':
            return (env, [])
    elif type(s) == dict:
        for label in s:
            if label == 'Print':
                children = s[label]
                f = children[0]
                p = children[1]
                v = evalFormula(env, f)
                if v == None:
                    v = evalTerm(env, f)
                (env, o) = execProgram(env, p)
                return (env, [v] + o)
            if label == 'Assign':
                children = s[label]
                x = children[0]['Variable'][0]
                f = children[1]
                p = children[2]
                env[x] = f
                (env2, o) = execProgram(env, p)
                return (env2, o)
            if label == 'Procedure':
                children = s[label]
                x = children[0]['Variable'][0]
                f = children[1]
                p = children[2]
                env[x] = f
                (env2, o1) = execProgram(env, p)
                return (env2, o1)
            if label == 'Call':
                children = s[label]
                x = children[0]['Variable'][0]
                f = env[x]
                p = children[1]
                (env2, o1) = execProgram(env, f)
                (env3, o2) = execProgram(env2, p)
                return (env3, o1+o2)
            if label == 'If':
                children = s[label]
                e = children[0]
                p1 = children[1]
                p2 = children[2]
                if evalFormula(env, e) == True:
                    (env2, o1) = execProgram(env, p1)
                    (env3, o2) = execProgram(env2, p2)
                    return (env3, o1 + o2)
                else:
                    return execProgram(env, p2)
            if label == 'While':
                children = s[label]
                e = children[0]
                p1 = children[1]
                p2 = children[2]
                while evalFormula(env, e) == True:
                    (env2, o1) = execProgram(env, p1)
                    (env3, o2) = execProgram(env, p2)
                    return (env3, o1 + o2)
                return execProgram(env, p2)
            #ADD CALL AND PROCEDURE
                    
def interpret(s):
    (env, o) = execProgram({}, tokenizeAndParse(s))
    return o
#eof