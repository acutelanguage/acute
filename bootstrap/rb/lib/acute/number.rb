# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Number < ::Acute::Object
    include Comparable
    attr_reader :value

    def initialize(num)
      super()
      @value = num.to_i
      method_table
      register("parent", $Object)
    end

    def method_table
      %w{+ - * /}.each do |s|
        method(s)         { |env| Number.new(value.send(s, env[:msg].eval_arg_at(env, 0).to_i)) }
      end
      method(:bitwiseAnd) { |env| Number.new(value & env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseOr)  { |env| Number.new(value | env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseXor) { |env| Number.new(value ^ env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseNot) { |env| Number.new(~value) }
      method(:shiftLeft)  { |env| Number.new(value << env[:msg].eval_arg_at(env, 0).to_i) }
      method(:shiftRight) { |env| Number.new(value >> env[:msg].eval_arg_at(env, 0).to_i) }
      method(:factorial)  { |env| Number.new((1..value).reduce(1, :*)) }
      method(:times)      { |env| r = ::Acute::Nil.instance; value.times { r = env[:msg].eval_arg_at(env, 0) }; r }
      method(:asString)   { |env| String.new(value.to_s) }
    end

    def <=>(other)
      self.value <=> other.value
    end

    def to_i
      value.to_i
    end

    def to_s
      to_i.to_s
    end
  end
end