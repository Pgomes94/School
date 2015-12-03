#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# validate.py
#
#  ****************************************************************
#  *************** Modify this file for Problem #5. ***************
#  ****************************************************************
#

exec(open('interpret.py').read())
exec(open('compile.py').read())

def expressions(n):
    if n <= 0:
        []
    elif n == 1:
        return [ ] # Add base case(s) for Problem #5.
    else:
        pass # Add recursive case(s) for Problem #5.

def programs(n):
    if n <= 0:
        []
    elif n == 1:
        return ['End']
    else:
        print(n)
        ps = programs(n-1)
        es = expressions(n-1)
        psN = []
        psN += [{'Assign':[{'Variable':['a']}, e, e, e, p]} for p in ps for e in es]
        psN += [{'Print': [e, p]} for p in ps for e in es]
        psN += [{'For': [{'Variable': ['x']}, p1, p2]} for p1 in ps for p2 in ps]
        
        pass # Add more nodes to psN for Problem #5.
        
        return ps + psN

# We always add a default assignment to the program in case
# there are variables in the parse tree returned from programs().

def defaultAssigns(p):
    return \
      {'AssignArray':[\
        {'Variable':['a']}, {'Number':[1]}, {'Number':[1]}, {'Number':[1]}, p\
      ]}

# Compute the formula that defines correct behavior for the
# compiler for all program parse trees of depth at most 4.
# Any outputs indicate that the behavior of the compiled
# program does not match the behavior of the interpreted
# program.

for p in [defaultAssigns(p) for p in programs(4)]:
    try:
        if simulate(compileProgram({}, p)[1]) != execute({}, p)[1]:
            print('\nIncorrect behavior on: ' + str(p))
    except:
        print('\nError on: ' + str(p))

#eof