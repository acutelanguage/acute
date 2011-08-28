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
      method(:setString) { |env| @value = env[:msg].eval_arg_at(env, 0).value; self }
      method(:append)    { |env| @value << env[:msg].eval_arg_at(env, 0).value; self }
      method(:prepend)   { |env| @value = env[:msg].eval_arg_at(env, 0).value + value; self }
      method(:length)    { |env| ::Acute::Number.new(@value.length) }
      method(:asString)  { |env| String.new(value.to_s) }
    end

    def to_s
      "\"#{value.to_s}\""
    end

    def with_method
      lambda do |env, s|
        o = ::Acute::Message.new("clone").perform_on(env, env[:sender], env[:target])
        ::Acute::Message.new("setString", [s]).perform_on(env, env[:sender], o)
      end
    end

    def <=>(other)
      self.value <=> other.value
    end
  end
end
