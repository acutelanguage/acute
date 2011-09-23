Lobby setSlot("Hash", Object clone)
Lobby Hash setSlot("data", list)
Lobby Hash setSlot("keys", list)

Lobby Hash setSlot("hash", method(v,
  setSlot("magic", 2166136261)
  setSlot("counter", 0)
  v length times(
    setSlot("magic", magic *: 16777619 bitwiseXor(v at: counter))
    setSlot("counter", counter +: 1)
  )
  magic
))

Lobby Hash setSlot("sort", method(v,
  v asString slice(0, 2) asNumber
))

Lobby Hash setSlot("at", method(k,
  setSlot("sortValue", sort(hashed(k)))
  data at: sortValue at(keys at: sortValue indexOf: k)
))

Lobby Hash setSlot("atPut", method(k, v,
  setSlot("sortValue", sort(hashed(k)))
  setSlot("keyBucket", keys at: sortValue)
  keyBucket isNil ifTrue(setSlot("keyBucket", list))
  keyBucket append: k
  keys atPut(sortValue, keyBucket)

  setSlot("dataBucket", data at: sortValue)
  dataBucket isNil ifTrue(setSlot("dataBucket", list))
  dataBucket atPut(keys at: sortValue indexOf: k, v)
  data atPut(sortValue, dataBucket)
))

Hash clone do(
  atPut("foo", 42)
)
