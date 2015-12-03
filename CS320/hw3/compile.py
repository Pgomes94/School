exec(open('machine.py').read())
exec(open('parse.py').read())
from random import randint

#taken from piazza to create a Fresh to prevent duplicates
def freshStr():
    return str(randint(0,100000))

def compileTerm(env, t, heap):
	if type(t) == dict:
		for label in t:
			children = t[label]
			
			#move heap forward
			#machine instructions: set heap pointer to the value of the Number
			#location is where heap points
			#top of the heap is where the value was set
			if label == 'Number':
				heap += 1
				return ( ['set ' + str(heap) + ' ' + str(children[0])], heap, heap)

			#move heap forward
			#grab variable from env
			#put into the new heap spot
			#location of it will be new heap pointer
			#top of the heap is the same point	
			if label == 'Variable':
				heap += 1
				variable = env[children[0]]
				return (copy(variable, heap),heap,heap)

			if label == 'Plus':
				#get terms to be added
				term1 = children[0]
				term2 = children[1]

				#compileTerm returns tuple(set of instructions to get term to add, location of term to add, top of heap)
				(instructions1, addr1, heap1) = compileTerm(env, term1, heap)
				(instructions2, addr2, heap2) = compileTerm(env, term2, heap1)

				#move heap forward to store the result of add there
				heap2 += 1
				
				#complete set of instructions for plus
				instructions = instructions1 + \
				instructions2 + \
				copy(addr1, 1) + \
				copy(addr2, 2) + \
				['add'] + \
				copy(0, heap2)

				return (instructions, heap2, heap2)

def compileFormula(env, f, heap):
	if type(f) == str:
		#f is either true or false
		if f == 'True':
			heap += 1 
			return (['set ' + str(heap) + ' 1'], heap, heap)
		if f == 'False':
			heap += 1
			return (['set ' + str(heap) + ' 0'], heap, heap)
	if type(f) == dict:
		for label in f:
			children = f[label]
			
			#same as compileTerm variable
			if label == 'Variable':
				heap += 1
				variable = env[children[0]]
				return (copy(variable, heap),heap,heap)

			if label == 'Not':
				fresh = freshStr()
				formula = children[0]
				#compile this formula, compileFormula returns (instructions, address, top of heap), same as compileTerm
				(instructions, addr, heap) = compileFormula(env, formula, heap)
				#increase heap
				heap += 1
				#final set of instructions
				instructions = instructions + \
				['branch setZero' + fresh + ' ' + str(addr), \
				'set ' + str(heap) + ' 1', \
				'goto finish' + fresh, \
				'label setZero' + fresh, \
				'set ' + str(heap) + ' 0', \
				'label finish' + fresh]

				return (instructions, heap, heap)

			if label == 'Or':
				fresh = freshStr()
				formula1 = children[0]
				formula2 = children[1]
				#similar to plus since 2 formulas to compile
				(instructions1, addr1, heap1) = compileFormula(env, formula1, heap)
				(instructions2, addr2, heap2) = compileFormula(env, formula2, heap1)
				heap2 += 1
				#final set of instructions
				instructions = instructions1 + \
				instructions2 + \
				copy(addr1, 1) + \
				copy(addr2, 2) + \
				["add", \
				"branch setOne" + fresh + " 0", \
				"goto finish" + fresh, \
				"label setOne" + fresh,\
				"set 0 1", \
				"label finish" + fresh] + \
				copy(0, heap2)

				return (instructions, heap2, heap2)

			if label == 'And':
				fresh = freshStr()
				formula1 = children[0]
				formula2 = children[1]
				#same as Or
				(instructions1, addr1, heap1) = compileFormula(env, formula1, heap)
				(instructions2, addr2, heap2) = compileFormula(env, formula2, heap1)
				heap2 += 1
				#final set of instructions
				instructions = instructions1 + \
				instructions2 + \
				['branch isTrue' + fresh + ' ' + str(addr1), \
				'goto setZero' + fresh, \
				'label isTrue' + fresh, \
				'branch setOne' + fresh + ' ' + str(addr2), \
				'goto setZero' + fresh, \
				'label setOne' + fresh, \
				'set ' + str(heap) + ' 0', \
				'goto finish' + fresh, \
				'label setZero' + fresh, \
				'set ' + str(heap) + ' 0', \
				'goto finish' + fresh, \
				'label finish' + fresh]

				return (instructions, heap, heap2)

#helper for program, compiles Expressions
def compileExpression(env, e, heap):
	term = compileTerm(env, e, heap)
	if not term is None:
		return term
	return compileFormula(env, e, heap)

#different return - returns (env, instructions, heap) instead of (instructions, addr, heap)
def compileProgram(env, s, heap):
	#this handles case where program is empty, thus everything remains the same and there are no instructions
	if type(s) == str:
		return (env, [], heap)
	if type(s) == dict: 
		for label in s:
			children = s[label]

			if label == 'Print':
				expression = children[0]
				s1 = children[1]
				#compile expression to be printed
				(instructions1, addr, heap1) = compileExpression(env, expression, heap)
				#rest of the instructions that need to be done.
				(env1, instructions2, heap2) = compileProgram(env, s1, heap1)
				#final instructions are, instructions to compile Expression, copy expression to output buffer, instructions for programs after.
				return (env, instructions1 + copy(addr, 5) + instructions2, heap2)
			
			if label == 'Assign':
				v = children[0]['Variable'][0]
				expression = children[1]
				s1 = children[2]
				(instructions1, addr, heap) = compileExpression(env, expression, heap)
				#add the variable to the environment
				env[v] = addr
				#rst of instructions to be done
				(env, instructions2, heap) = compileProgram(env, s1, heap)
				return (env, instructions1 + instructions2, heap)

			if label == 'If':
				expression = children[0]
				program = children[1]
				s1 = children[2]
				fresh = freshStr()
				#compile the expression
				(instructions1, addr, heap1) = compileExpression(env, expression, heap)
				#compile program for if statement
				(env1, instructions2, heap2) = compileProgram(env, program, heap1)
				#compile everything after program for if
				(env2, instructions3, heap3) = compileProgram(env1, s1, heap2)
				#final instructions
				instructions = instructions1 + \
				instructions2 + \
				['branch ifBody' + fresh + ' ' + str(addr), \
				'goto skip' + fresh, \
				'label ifBody' + fresh] + \
				instructions3 + \
				['label skip' + fresh]

				return (env2, instructions, heap3)

			if label == 'While':
				expression = children[0]
				program = children[1]
				s1 = children[2]
				fresh = freshStr()
				#compile the expression
				(instructions1, addr, heap1) = compileExpression(env, expression, heap)
				#compile program for while statement
				(env1, instructions2, heap2) = compileProgram(env, program, heap1)
				#compile everything after program for while
				(env2, instructions3, heap3) = compileProgram(env1, s1, heap2)
				#final instructions
				instructions = instructions1 + \
				['label whileStart' + fresh, \
				'branch whileBody' + fresh + ' ' + str(addr),\
				'goto whileEnd' + fresh,\
				'label whileBody' + fresh] + \
				instructions2 + \
				['goto whileStart' + fresh, \
				'label whileEnd'+ fresh] + \
				instructions3

				return (env2, instructions3, heap3)

			#instructions for procedure and the rest of the program
			if label == 'Procedure':
				v = children[0]['Variable'][0]
				program = children[1]
				s1 = children[2]
				(env1, instructions1, heap1) = compileProgram(env, program, heap)
				(env2, instructions2, heap2) = compileProgram(env1, s1, heap1)
				instructions3 = procedure(v, instructions1)
				return (env2, procedure(v, instructions1) + instructions2, heap2)

			#calls the variable then executes rest of program
			if label == 'Call':
				v = children[0]['Variable'][0]
				s1 = children[1]
				(env, instructions, heap) = compileProgram(env, s1, heap)
				instructions1 = call(v)
				return (env, call(v) + instructions, heap)

def compile(s):
	tokens = tokenizeAndParse(s)
	#heap starts after 7
	#env is initially empty
	(env, instructions, heap) = compileProgram( {} , tokens, 7)
	# set -1 0 => first input in stack
	# set 7 to -1 => initializes the stack
	return ['set -1 0' , 'set 7 -1'] + instructions	
