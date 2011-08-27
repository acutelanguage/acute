# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  # The Walker walks over our messages and evaluates them, returning a result for the receiver of the next message.
  # It also keeps track of who's sending the message and passes that along too.
  class Walker
    attr_reader :default_context, :tree

    def initialize(default_context, tree)
      @default_context = default_context
      @tree = tree
    end

    def walk(&blk)
      receiver = default_context
      sender = default_context
      m = tree
      begin
        receiver = receiver.perform(receiver, :msg => m, :sender => sender)
        blk.call(receiver) if blk
      end while m = m.next
    end
  end
end
