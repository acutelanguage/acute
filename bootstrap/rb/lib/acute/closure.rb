# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Closure < ::Acute::Object
    attr_accessor :env
    attr_reader :func

    def initialize(env = {}, &blk)
      super()
      @env = env
      @func = blk
      activate_slot = Slot.new(lambda do |env|
        blk_slot = env[:sender].lookup(env, env[:msg].name)
        blk_slot.data.func.call(env, *env[:msg].arguments) if blk_slot
      end)
      activate_slot.activatable = true
      @slots[:activate] = activate_slot
    end

    def call(env, *args)
      func.call(env, *args)
    end
  end
end
