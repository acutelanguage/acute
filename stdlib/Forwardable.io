Forwardable := Object trait do(
  forward := method(self proto ?(doMessage(call message)))
)