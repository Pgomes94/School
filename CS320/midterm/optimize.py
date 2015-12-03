#####################################################################
#
# CAS CS 320, Fall 2014
# Midterm (skeleton code)
# optimize.py
#
#  ****************************************************************
#  ************* You do not need to modify this file. *************
#  ****************************************************************
#

Node = dict
Leaf = str

def foldConstants(a):
    if type(a) == Leaf:
        return a
    if type(a) == Node:
        for label in a:
            children = a[label]
            if label == 'Array':
                [x, e] = children
                return {'Array':[x, foldConstants(e)]}
            elif label == 'Plus':
                [e1, e2] = children
                e1 = foldConstants(e1)
                e2 = foldConstants(e2)
                if 'Number' in e1 and 'Number' in e2:
                    return {'Number':[e1['Number'][0] + e2['Number'][0]]}
                return a
            elif label == 'Print':
                [e, p] = children
                return {'Print':[foldConstants(e), foldConstants(p)]}
            elif label == 'Assign':
                [x, e0, e1, e2, p] = children
                return {'Assign':[x, foldConstants(e0), foldConstants(e1), foldConstants(e2), foldConstants(p)]}
            elif label == 'For':
                [x, p1, p2] = children
                return {'For':[x, foldConstants(p1), foldConstants(p2)]}
            else:
                return a

def unrollLoops(s):
    if type(s) == Leaf:
        return s
    if type(s) == Node:
        for label in s:
            children = s[label]
            if label == 'Print':
                [e, p] = children
                return {'Print':[e, unrollLoops(p)]}
            elif label == 'Assign':
                [x, e0, e1, e2, p] = children
                return {'Assign':[x, e0, e1, e2, unrollLoops(p)]}
            elif label == 'For':
                [x, p1, p2] = children
                x = x['Variable'][0]
                p1 = unrollLoops(p1)
                p1third = replaceAndConcat(x, 2, p1, unrollLoops(p2))
                p1second = replaceAndConcat(x, 1, p1, p1third)
                p1first = replaceAndConcat(x, 0, p1, p1second)
                return p1first
            else:
                return s

# Helper function for unrollLoops.
def replace(x, n, e):
    if type(e) == Leaf:
        return e
    if type(e) == Node:
        for label in e:
            children = e[label]
            if label == 'Variable':
                [y] = children
                return {'Number':[n]} if x == y else e
            elif label == 'Array':
                [x, e] = children
                return {'Array':[x, replace(x, n, e)]}
            elif label == 'Plus':
                [e1, e2] = children
                return {'Plus':[replace(x, n, e1), replace(x, n, e2)]}
            else:
                return e

# Helper function for unrollLoops.
def replaceAndConcat(x, n, p1, p2):
    if type(p1) == Leaf:
        return p2
    if type(p1) == Node:
        for label in p1:
            children = p1[label]
            if label == 'Print':
                [e, p] = children
                return {'Print':[replace(x, n, e), replaceAndConcat(x, n, p, p2)]}
            elif label == 'Assign':
                [x, e0, e1, e2, p] = children
                return {'Assign':[x, replace(x, n, e0), replace(x, n, e1), replace(x, n, e2), replaceAndConcat(x, n, p, p2)]}

#eof