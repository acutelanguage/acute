# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Closure < ::Acute::Object
    attr_reader :func

    def initialize(env, &blk)
      super()
      @func = blk
      @activatable = true
      @slots[:activate] = lambda {|e| activate(e) }
    end

    def activate(env)
      call(env, env[:msg].arguments)
    end

    def call(env, args = [])
      func.call(env)
    end
  end
end
