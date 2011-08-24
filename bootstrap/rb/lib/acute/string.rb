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
    end

    def method_table
      method(:clone, &clone_method)
      method(:with, &with_method)
      method(:setString) { |env, s| @value = s.value; self }
      method(:append)    { |env, s| @value << s.value; self }
      method(:prepend)   { |env, s| @value = s.value + value; self }
    end

    def to_s
      value.to_s
    end

    def clone_method
      lambda do |env|
        o = self.class.new
        # Because we don't have inheritance
        self.slots.each { |k,v| o.register(k.to_sym, v.data, :activatable => v.activatable?) }
        o.register(:parent, self)
        o.register(:init, ::Acute::Closure.new { |env| o }, :activatable => true)
        p o.slots.keys
        o.perform(env[:sender], :msg => ::Acute::Message.new("init"))
        o
      end
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
