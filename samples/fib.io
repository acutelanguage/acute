do(
  setSlot("fib", method(n,
    n <(2) ifTrue(n) ifFalse(
      fib(n -(1)) +(fib(n -(2)))
    )
  ))
)

fib(10) print
