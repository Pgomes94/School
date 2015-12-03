#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# compile.py
#
#  ****************************************************************
#  *************** Modify this file for Problem #3. ***************
#  ****************************************************************
#

from random import randint
exec(open('parse.py').read())
exec(open('interpret.py').read())
exec(open('optimize.py').read())
exec(open('machine.py').read())
exec(open('analyze.py').read())

Leaf = str
Node = dict

def freshStr():
    return str(randint(0,10000000))

def compileExpression(env, e, heap):
    if type(e) == Node:
        for label in e:
            children = e[label]
            if label == 'Number':
                n = children[0]
                heap = heap + 1
                return (['set ' + str(heap) + ' ' + str(n)], heap, heap)
            elif label == 'Variable':
                v = children[0]
                if v in env:
                    return ([], env[v], heap)
            elif label == 'Plus':
                insts1, addr1, heap = compileExpression(env, children[0], heap)
                insts2, addr2, heap = compileExpression(env, children[1], heap)
                heap = heap + 1
                insts = copy(addr1, 1) + copy(addr2, 2) + ["add"] + copy(0, heap)
                return ((insts1 + insts2 + insts), heap, heap)
            elif label == 'Array':
                insts1, addr1, heap = compileExpression( env, children[0], heap)
                insts2, addr2, heap = compileExpression( env, children[1], heap)
                heap += 1
                insts = copy(addr2, 1)+[
                    "set 2 "+str(addr1),
                    "add"
                ] + copyToRef(0, heap)

                return ((inst1 + inst2 + insts), heap, heap)

def compileProgram(env, p, heap = 8):
    if type(p) == Leaf:
        if s == 'End':
            return (env, [], heap)

    if type(p) == Node:
        for label in p:
            children = p[label]
            if label == 'Print':
                [e, p] = children
                (instsE, addr, heap) = compileExpression(env, e, heap)
                (env, instsP, heap) = compileProgram(env, p, heap)
                return (env, instsE + copy(addr, 5) + instsP, heap)
            elif label == 'Assign':
                v  = children[0]['Variable'][0]
                e1 = children[1]
                e2 = children[2]
                e3 = children[3]
                insts1, addr2, heap = compileExpression( env, e1, heap  )
                insts2, addr3, heap = compileExpression( env, e2, heap  )
                insts3, addr4, heap = compileExpression( env, e3, heap  )
                heap = heap + 1
                insts = [copy(addr2, heap)]
                heap = heap + 1
                insts = insts + [copy(addr3, heap)]
                heap = heap + 1
                insts = insts + [copy(addr4, heap)]
                env[v] = heap-2
                return (env, insts1 + insts2 + insts3 + insts, heap)

def compile(s):
    p = tokenizeAndParse(s)

    # Add call to type checking algorithm for Problem #4.
    o = typeProgram({}, p)
    print(o)
    # Add calls to optimization algorithms for Problem #3.
    s = foldConstants(s)
    s = unrollLoops(s)

    (env, insts, heap) = compileProgram({}, p)
    return insts

def compileAndSimulate(s):
    return simulate(compile(s))

#print(compile("assign x := [3+4, 5, 10];"))
#print(compile("assign x := [2, 4, 10]; for i { print @x[i];}"))

#eof