# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class String < ::Acute::Object
    attr_reader :value

    def initialize(str = "")
      super()
      @value = str
      method_table
      register(:parent, $Object)
    end

    def method_table
      method(:with, &with_method)
      method(:setString) { |env, s| @value = eval_in_context(s, env[:sender]).value; self }
      method(:append)    { |env, s| @value << eval_in_context(s, env[:sender]).value; self }
      method(:prepend)   { |env, s| @value = eval_in_context(s, env[:sender]).value + value; self }
      method(:length)    { |env| ::Acute::Number.new(@value.length) }
    end

    def to_s
      value.to_s
    end

    def with_method
      lambda do |env, s|
        o = self.perform(env[:sender], :msg => ::Acute::Message.new("clone"))
        o.perform(env[:sender], :msg => ::Acute::Message.new("setString", [s]))
      end
    end

    def <=>(other)
      self.value <=> other.value
    end
  end
end
