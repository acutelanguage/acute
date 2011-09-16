setSlot("Hash", Object clone do(
  setSlot("init", method(
    self setSlot("data", list)
    self setSlot("keys", list)
  ))

  setSlot("hash", method(v,
    setSlot("magic", 2166136261)
    setSlot("counter", 0)
    v length times(
      setSlot("magic", (magic *: 16777619) bitwiseXor(v at: counter))
      setSlot("counter", counter +: 1)
    )
    magic
  ))

  setSlot("sort", method(v,
    v asString slice(0, 2) asNumber
  ))

  setSlot("at", method(k,
    setSlot("sortValue", sort(hashed(k)))
    data at: sortValue at(keys at: sortValue indexOf: k)
  ))

  setSlot("atPut", method(k, v,
    setSlot("sortValue", sort(hashed(k)))
    setSlot("keyBucket", keys at: sortValue)
    if(keyBucket isNil, setSlot("keyBucket", list))
    keyBucket append: k
    keys atPut(sortValue, keyBucket)

    setSlot("dataBucket", data at: sortValue)
    if(dataBucket isNil, setSlot("dataBucket", list))
    dataBucket atPut(keys at: sortValue indexOf: k, v)
    data atPut(sortValue, dataBucket)
  ))
))

Hash hash("foo") print
