#
# Copyright John Reid 2009
#

import _seqan as seqan, copy

def visit_node(it):
    print '%3d occurrences of "%s"' % (it.occurrences, it.representative)
    if it.goDown():
        visit_node(copy.copy(it))
        while it.goRight():
            visit_node(copy.copy(it))

string = seqan.String('ACGTACGT')
print 'String =', string, 'of length', len(string)
index = seqan.Index(string)
print 'Index =', index, 'of length', len(index)
iterator = seqan.Iterator(index)
visit_node(iterator)

string_set = seqan.StringSet()
string_set.appendValue(string)
string_set.appendValue(seqan.String('AC'))
assert 2 == len(string_set)
