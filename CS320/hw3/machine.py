#####################################################################
#
# CAS CS 320, Fall 2014
# Assignment 3 (skeleton code)
# machine.py
#

def simulate(s):
    instructions = s if type(s) == list else s.split("\n")
    instructions = [l.strip().split(" ") for l in instructions]
    mem = {0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: -1, 6: 0}
    control = 0
    outputs = []
    while control < len(instructions):
        # Update the memory address for control.
        mem[6] = control 
        
        # Retrieve the current instruction.
        inst = instructions[control]
        
        # Handle the instruction.
        if inst[0] == 'label':
            pass
        if inst[0] == 'goto':
            control = instructions.index(['label', inst[1]])
            continue
        if inst[0] == 'branch' and mem[int(inst[2])]:
            control = instructions.index(['label', inst[1]])
            continue
        if inst[0] == 'jump':
            control = mem[int(inst[1])]
            continue
        if inst[0] == 'set':
            mem[int(inst[1])] = int(inst[2])
        if inst[0] == 'copy':
            mem[mem[4]] = mem[mem[3]]
        if inst[0] == 'add':
            mem[0] = mem[1] + mem[2]

        # Push the output address's content to the output.
        if mem[5] > -1:
            outputs.append(mem[5])
            mem[5] = -1

        # Move control to the next instruction.
        control = control + 1

    print("memory: "+str(mem))
    return outputs

# Examples of useful helper functions from lecture.    
def copy(frm, to):
   return [\
      'set 3 ' + str(frm),\
      'set 4 ' + str(to),\
      'copy'\
   ]

#resets all given addresses value to 0
def reset(addresses):
    return ['set '+ str(addr) + " 0" for addr in addresses]

#increments whatever is in address addr by 1, result at address addr
def increment(addr):
    return  copy(addr, 1) + [ \
            'set 2 1', \
            'add' \
            ] + \
            copy(0, addr) + reset([0, 1, 2, 3, 4])

#decrements whatever is in address addr by 1, result at address addr
def decrement(addr):
    return  copy(addr, 1) + [ \
            'set 2 -1', \
            'add' \
            ] + \
            copy(0, addr) + reset([0, 1, 2, 3, 4])

def call(name):
    #lines 91-95 #copies whats in mem[6] into mem[7]
    #lines 96-100  #copies whats in mem[7] to mem[2]
    #set 1 14 because 14 instructions total
    #lines 105-109 #put the updated value, located at mem[0] into mem[7]
    return decrement(7) + copy(7,4) + [ \
    'set 3 6', \
    'copy', \
    ] + \
    copy(7,3) + [ \
    'set 4 2',\
    'copy', \
    'set 1 14',\
    'add'\
    ] + \
    copy(7,4)+ [ \
    'set 3 0',\
    'copy', \
    'goto ' + name
    ] + \
    increment(7)

def procedure(name, body):
    return [ \
    'goto ' + name + 'end', \
    'label ' + name, \
    ] + \
    body + copy(7,3) + [ \
    'set 4 0', \
    'copy', \
    'jump 0', \
    'label ' + name + 'end' \
    ]  
# eof