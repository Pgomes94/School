#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# analyze.py
#
#  ****************************************************************
#  *************** Modify this file for Problem #4. ***************
#  ****************************************************************
#

exec(open("parse.py").read())

Node = dict
Leaf = str

def typeExpression(env, e):
    if type(e) == Leaf:
        if e == 'True' or e == 'False':
            return 'Boolean'

    if type(e) == Node:
        for label in e:
            children = e[label]
            if label == 'Number':
                return 'Number'

            if label == 'Variable':
                v = children[0]
                if v in env:
                    return env[v]

            elif label == 'Array':
                [x, e] = children
                var = x['Variable'][0]
                if var in env and env[var] == 'Array' and typeExpression(env, e) == 'Number':
                    return 'Number'

            elif label == 'Plus':
                if typeExpression(env, children[0]) == 'Number' and typeExpression(env, children[1]) =='Number':
                    return 'Number'

def typeProgram(env, s):
    if type(s) == Leaf:
        if s == 'End':
            return 'Void'
    elif type(s) == Node:
        for label in s:
            if label == 'Print':
                [e, p] = s[label]
                if typeProgram(env, p) == 'Void':
                    if typeExpression(env, e) == 'Boolean' or typeExpression(env, e) == 'Number':
                        return "Void"

            if label == 'Assign':
                [x, e0, e1, e2, p] = s[label]
                x = x['Variable'][0]
                if typeExpression(env, e0) == 'Number' and\
                   typeExpression(env, e1) == 'Number' and\
                   typeExpression(env, e2) == 'Number':
                     env[x] = 'Array'
                     if typeProgram(env, p) == 'Void':
                           return 'Void'

            if label == 'For':
                [x, p1, p2] = s[label]
                var = x['Variable'][0]
                env[var] = 'Number'
                if typeExpression(env, x) == 'Number':
                    if typeProgram(env, p1) == 'Void':
                        if typeProgram(env,p2) == 'Void':
                            return 'Void'


#eof