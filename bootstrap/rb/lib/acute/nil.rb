# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

require 'singleton'

module Acute
  class Nil < ::Acute::Object
    include ::Singleton
    attr_reader :value

    def initialize
      super()
      @value = nil
      method_table
    end

    def method_table
      method(:clone)   { |env| self }
      method(:ifTrue)  { |env, _| self }
      method(:ifFalse) { |env, msg| env[:sender].perform(env[:sender], :msg => msg) }
    end

    def to_s
      "nil"
    end
  end
end
