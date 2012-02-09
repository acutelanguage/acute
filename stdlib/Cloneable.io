Cloneable do(
  clone := method(
    r := Wee primitive: clone call addTrait(Cloneable) addTrait(Forwardable)
    r proto := self
    r ?(init)
    r
  )

  orphan := method(
    r := Wee primitive: clone call addTrait(Cloneable)
    r proto := self
    r ?(init)
    r
  )

  trait := method(
    r := Wee primitive: clone call addTrait(Forwardable)
    r proto := self
    r ?(init)
    r
  )
)