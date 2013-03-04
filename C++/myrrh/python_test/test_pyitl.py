#
# Copyright John Reid 2010
#

import _pyitl as P

def test_interval(Interval):
    "Test an interval type."

    # test empty, nonzero, clear
    i = Interval(10, 20)
    assert not i.empty
    assert i
    assert i.__nonzero__()
    i.clear()
    assert i.empty
    assert not i
    assert not i.__nonzero__()

    # test contains
    i = Interval(10, 20)
    assert 10 in i
    assert Interval(10, 20) in i
    assert i.contained_in(Interval(0, 40))
    assert i.free_contains(Interval(12, 18))
    assert not i.free_contains(Interval(12, 20))
    assert i.proper_contains(Interval(12, 20))
    assert not i.proper_contains(Interval(10, 20))
    assert i.is_disjoint(Interval(25, 35))
    assert not i.is_disjoint(Interval(15, 35))
    assert not i.intersects(Interval(25, 35))
    assert i.intersects(Interval(15, 35))

    # sizes
    assert i.cardinality == Interval.__continuous__ and 0 or 11
    assert len(i) == Interval.__continuous__ and 11 or 10
    assert i.size == Interval.__continuous__ and 11 or 10
    assert i.lower == 10
    assert i.upper == 20
    if not Interval.__continuous__:
        assert i.first == 10
        assert i.last == 20

    # boundtype
    assert P.closed_bounded == i.boundtype

    # extend
    assert Interval(5, 20) == Interval(5, 6).extend(i)
    assert Interval(5, 20) == Interval(5, 6).hull(i)
    assert Interval(10, 18) == Interval(12, 18).left_extend(i)
    assert Interval(12, 20) == Interval(12, 18).right_extend(i)




assert not P.IntInterval.__continuous__
test_interval(P.IntInterval)

assert P.FloatInterval.__continuous__
test_interval(P.FloatInterval)


#Interval = P.IntervalInt
#Map = P.SplitIntervalMapInt2Int
Interval = P.FloatInterval
Map = P.SplitFloatIntervalMap

def add_interval(map, interval, value):
    segment = Map.Segment( interval, value )
    print 'Adding', segment, 'to map'
    map += Map.Segment( interval, value )

map = Map()
add_interval(map, Interval(10, 30), 2)
add_interval(map, Interval(10, 30), 4)
add_interval(map, Interval(20, 35), 11)
assert not map.empty
assert map
assert map.__nonzero__()

assert 10 in Interval(10, 30)

iter(map)
for i in map:
    print 'Entry in map:', i

print "Length         : ", len(map)
print "Cardinality    : ", map.cardinality()
print "Size           : ", map.size()
print "Interval count : ", map.interval_count()
print "Iterative size : ", map.iterative_size()
print "Lower          : ", map.lower
print "Upper          : ", map.upper
#print "First          : ", map.first
#print "Last           : ", map.last

print "Is 10 in map?", 10 in map
print "Is (10,30) in map?", Interval(10, 30) in map
print "Is (15,33) in map?", Interval(15, 33) in map
print "Is (9, 12) in map?", Interval( 9, 12) in map

print 'Erasing Interval(28,33)'
map.erase(Interval(28,33))
for i in map:
    print 'Entry in map:', i

print "Finding 11:", map[11]

print 'Clearing'
map.clear()
assert map.empty
assert not map
assert not map.__nonzero__()
