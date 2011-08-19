
describe(Message,
  it("can create a new message",
    Message clone isNil verify(not)
  )

  it("factory creates a new message",
    message(a) isNil verify(not)
  )

  it("creates a message from a string",
    Message clone fromString("a b") verify(compare(message(a b)))
  )

  it("has a name",
    message(a) name verify(== "a")
  )

  it("has an argument list",
    message(a(1, 2)) arguments verify(compare(list(1, 2)))
  )

  it("has exactly two arguments",
    message(a(1, 2)) argCount verify(== 2)
  )

  it("gets the second argument",
    message(a(1, 2)) argAt(1) isNil verify(not)
  )

  it("appends an argument",
    message(a(1)) appendArg(message(2)) arguments verify(compare(list(1, 2)))
  )

  it("replaces the argument list with another",
    m1 := message(a(1))
    m2 := message(a(2))
    m1 setArguments(list(message(2))) verify(compare(m2))
  )

  it("retrieves the next message",
    message(a b) next isNil verify(not)
  )

  it("next message has the right name",
    message(a b) next name verify(== "b")
  )

  it("sets the next message to another message",
    m := message(a b)
    o := message(c d)
    m setNext(o) verify(compare(message(a c d)))
  )

  it("retrieves the previous message",
    m := message(a b)
    m next previous verify(== m)
  )

  it("can be represented as a string",
    message(a b) asString verify(== "a b")
  )

  it("evaluates a message in a specific context",
    o := Object clone do(a := 42)
    message(a) doInContext(o) verify(== o a)
  )

  it("detects if we are at the end of the line",
    message(a ; b) next isEndOfLine verify(== true)
  )

  it("cachedResult is nil",
    message(a) cachedResult verify(== nil)
  )

  it("has no cached result",
    message(a) hasCachedResult verify(== false)
  )

  it("sets a cached result",
    m := message(a) setCachedResult(42)
    doMessage(m) verify(== 42)
  )
)
