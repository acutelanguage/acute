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
      register(:parent, $state.find("Object"))
      method_table
    end

    def method_table
      method(:clone)    { |env| self }
      method(:ifTrue)   { |env| self }
      method(:ifFalse)  { |env| env[:msg].eval_arg_at(env, 0) }
      method(:asString) { |env| ::Acute::String.new(to_s) }
    end

    def to_s
      "nil"
    end
  end
end
