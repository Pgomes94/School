from math import log, floor
import re
import parse

def tokenize(terminals, characters):
    
    terminals = [re.escape(t) for t in terminals]

    terminals = '|'.join(terminals)
    terminals = '(' + terminals + ')'
    
    tokens = [t for t in re.split(terminals, characters)]
    return [t for t in tokens if not t.isspace() and not t ==""]

def evalTerm(env, t):
    if type(t) == dict:
        for label in t:
            children = t[label]
            if label == 'Number':
                f = children[0]
                return f
            elif label == 'Variable':
                f = children[0]
                if f in env:
                        v = env[f]
                        return v
            elif label == 'Log':
                f = children[0]
                x = evalTerm(env, f)
                if not x is None:
                        v = floor( log(x,2) )
                        return v
            elif label == 'Plus':
                f1 = children[0]
                x1 = evalTerm(env, f1)
                f2 = children[1]
                x2 = evalTerm(env, f2)
                if not x1 is None and not x2 is None:
                        return x1 + x2
            elif label == 'Mult':
                f1 = children[0]
                x1 = evalTerm(env, f1)
                f2 = children[1]
                x2 = evalTerm(env, f2)
                if not x1 is None and not x2 is None:
                        return x1 * x2
            elif label == 'Parens':
                f = children[0]
                return evalTerm(env, f)
                
def evalFormula(env, f):
        if type(f) == dict:
                for label in f:
                        children = f[label]
                        if label == 'Variable':
                                t = children[0]
                                if t in env:
                                        v = env[t]
                                        return v
                        if label == 'Not':
                                t = children[0]
                                v = evalFormula(env, t)
                                return not v
                        elif label == 'Xor':
                                t2 = children[1]
                                v2 = evalFormula(env, t2)
                                t1 = children[0]
                                v1 = evalFormula(env, t1)
                                return v1 != v2
                        elif label == 'Parens':
                                t = children[0]
                                return evalFormula(env, t)                                
        elif type(f) == str:
                if f == 'True':
                        return True
                if f == 'False':
                        return False

def execProgram(env, s):
        if type(s) == list:
                s2 = s
                s = s[0]
        if type(s) == dict:
                for label in s:
                        children = s[label]
                        if label == 'Print':
                                f = [execProgram(env, x) for x in children]
                                f = f[0:len(f) -1]
                                try:
                                    s2
                                except NameError:
                                    s2 = None
                                if not s2 is None:
                                        f.append(execProgram(env, s2[1])[1][0])
                                return (env, f)
                        elif label == 'Assign':
                                f = children[0]
                                env[(f['Variable'])[0]] = execProgram(env,children[1])
                                return execProgram(env, (children[2:])[0])
                        elif label == 'Not':
                                f = children[0]
                                v = evalFormula(env, f)
                                return not v
                        elif label== 'Variable':
                                if children[0] in env:
                                        return env[children[0]]
                        elif label == 'Number':
                                f = children[0]
                                return f
                        elif label == 'If':
                                f = children[0]
                                f = env[ (f['Variable'])[0]]
                                if f == True:
                                        return execProgram(env, (children[1:]))
                                elif f == False:
                                        return execProgram(env, (children[2:]))
                                        return
                        elif label =='While':
                                f = children[0]
                                f2 = execProgram(env,f)
                                e1=list()
                                while f2 == True:
                                        f2 = children[1:]
                                        e1.append(execProgram(env, f2[0])[1][0])
                                        execProgram(env, f)
                                        f2 = execProgram(env, f)
                                try:
                                    s2
                                except NameError:
                                    s2 = None
                                    f2 = children[2:]
                                    e = execProgram(env,f2[0])
                                    for x in e[0]:
                                            e1.append(env[x])
                                    return (env, e1)
                                if not s2 is None:
                                        e1.append(execProgram(env, s2[1])[1][0])
                                        return (env,e1)
                                temp=children[1:][0].values()[0]
                                while( type(temp == dict)):
                                        if 'End' in temp:
                                                return (env,e1)
                                        temp = temp[1:][0].values()[0]
                                try:
                                    s2
                                except NameError:
                                    s2 = None
                                    f2 = children[2:]
                                    print(f2)
                                    print(execProgram(env, f2[0]))
                                    e1.append(execProgram(env, f2[0])[1][0])
                                    return (env, e1)
                                if not s2 is None:
                                        e1.append(execProgram(env, s2[1])[1][0])
                                        return (env,e1)
                        elif label =='Plus':
                                f = evalTerm(env, s)
                                return f
                        elif label =='Mult':
                                f = evalTerm(env, s)
                                return f
                        elif label == 'Xor':
                                t2 = children[1]
                                v2 = evalFormula(env, t2)
                                t1 = children[0]
                                v1 = evalFormula(env, t1)
                                return v1 != v2
        elif type(s) == str:
                if s =='End':
                        return (env, [])
                if s == 'False':
                        return False
                if s == 'True':
                        return True

def interpret(tokens):
        regex =['print' , 'assign' , ':=' , 'if', 'true' , 'false' , 'while' , 'not' , 'xor' , '+'\
                , '*' , ';' , '{' , '}' , '(' , ')' , ' ']
        tokens = parse.tokenize(regex, tokens)
        tokens = parse.program(tokens)
        return execProgram({}, tokens[0])[1]
