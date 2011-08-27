# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  # The Walker walks over our messages and evaluates them, returning a result for the receiver of the next message.
  # It also keeps track of who's sending the message and passes that along too.
  class Walker
    def self.walk(sender, tree, receiver = nil, &blk)
      receiver = sender unless receiver
      m = tree
      begin
        receiver = receiver.perform(receiver, :msg => m, :sender => sender, :target => receiver)
        blk.call(receiver) if blk
      end while m = m.next
    end
  end
end
