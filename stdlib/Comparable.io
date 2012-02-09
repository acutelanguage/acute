// Comparable only depends on two outside non-primitive methods:
//    compare() -- a ternary comparison operator to be implemented
//    and() -- a boolean and which should exist on true and false.
Comparable := Object clone do(
  == := method(other, Wee primitive: equal call(self compare: other, 0)

  != := method(other, Wee primitive: not call(self compare: other == 0))

  > := method(other, Wee primitive: equal call(self compare: other, 1)

  >= := method(other, self > other or self == other)

  < := method(other, Wee primitive: equal call(self compare: other, -1)

  <= := method(other, self < other or self == other)

  isBetween := method(o1, o2, self >= o1 and: self <= o2)
)