# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Closure < ::Acute::Object
    attr_reader :func

    def initialize(env, init_mt = false, &blk)
      super()
      @func = blk
      @slots[:activate] = Slot.new(lambda {|env| activate(env) }, env[:target], :activatable => true) if init_mt
    end

    def activate(env)
      func.call(env, *env[:msg].arguments)
    end

    def call(env, *args)
      func.call(env, *args)
    end
  end
end
