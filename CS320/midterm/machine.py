#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# machine.py
#
#  ****************************************************************
#  ************* You do not need to modify this file. *************
#  ****************************************************************
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
            print(inst[1])
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

    #print("memory: "+str(mem)) # Uncomment if necessary.
    return outputs

def copy(frm, to):
   return [\
      'set 3 ' + str(frm),\
      'set 4 ' + str(to),\
      'copy'\
   ]
   
def copyToRef(frm, refTo):
   return [\
      'set 3 ' + str(refTo),\
      'set 4 4',\
      'copy',\
      'set 3 ' + str(frm),\
      'copy'\
   ]

def copyFromRef(refFrm, to):
   return [\
      'set 3 ' + str(refFrm),\
      'set 4 3',\
      'copy',\
      'set 4 ' + str(to),\
      'copy'\
   ]

# eof