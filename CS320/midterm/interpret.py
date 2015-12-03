#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# interpret.py
#
#  ****************************************************************
#  *************** Modify this file for Problem #2. ***************
#  ****************************************************************
#

exec(open("parse.py").read())

Node = dict
Leaf = str

def evaluate(env, e):
	if type(e) == Leaf:
		if e == "True":
			return True
		elif e == "False":
			return False
	elif type(e) == Node:
		for label in e:
			children = e[label]
			if label == 'Number':
				x = children[0]
				v = evaluate(env, x)
				return int(v)
			elif label == 'Variable':
				x = children[0]
				if x in env:
					return env[x]
			elif label == 'Plus':
				f1 = children[0]
				v1 = evaluate(env, f1)
				f2 = children[1]
				v2 = evaluate(env, f2)
				return v1 + v2
			elif label == 'Array':
				x = children[0]
				if x in env:
					k = evaluate(children[1])
					return env[x][k]

def execute(env, s):
	if type(s) == Node:
		for label in s:
			children = s[label]
			if label == 'Assign':
				v = children[0]
				e1 = evaluate(children[1])
				e2 = evaluate(children[2])
				e3 = evaluate(children[3])
				p = children[4]
				e = [e1,e2,e3]
				env[v] = e
				(env, o) = execute(env, p)
				return (env, o)
			if label == 'Print':
				p = evaluate(children[0])
				r = children[1]
				(env, o) = execute(env, r)
				return (env, [p] + o)

			if label == 'For':
				v = children[0]
				p1 = children[1]
				p2 = children[2]
				for x in range(3):
					env[v] = x
					(env, o1) = execute(env, p1)
					o = o + o1
				(env, o2) = execute(env, p2)
				return (env, o + o2)

			if label == '':
				return "End"
def interpret(s):
    s = tokenizeAndParse(s)
    execute( {}, s)


#eof