# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Number < ::Acute::Object
    include Comparable
    attr_reader :value

    def initialize(num, init_mt = false)
      super()
      @value = num.to_i
      obj_name = init_mt ? "Object" : "Number"
      register("parent", $state.find(obj_name))
      method_table if init_mt
    end

    def method_table
      %w{+ - * / %}.each do |s|
        method(s)         { |env| Number.new(env[:target].value.send(s, env[:msg].eval_arg_at(env, 0).to_i)) }
      end
      method(:compare)    { |env| Number.new(env[:target].value <=> env[:msg].eval_arg_at(env, 0).to_i) }
      method(:<)          { |env| compare_helper(env, -1) }
      method(:>)          { |env| compare_helper(env, 1) }
      method(:==)         { |env| compare_helper(env, 0) }
      method(:'!=')       { |env| (env[:target].perform(env.merge(:msg => ::Acute::Message.new("compare", [env[:msg].arguments[0]]))).to_i != 0) ? env[:target] : ::Acute::Nil.instance }
      method(:bitwiseAnd) { |env| Number.new(env[:target].value & env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseOr)  { |env| Number.new(env[:target].value | env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseXor) { |env| Number.new(env[:target].value ^ env[:msg].eval_arg_at(env, 0).to_i) }
      method(:bitwiseNot) { |env| Number.new(~env[:target].value) }
      method(:shiftLeft)  { |env| Number.new(env[:target].value << env[:msg].eval_arg_at(env, 0).to_i) }
      method(:shiftRight) { |env| Number.new(env[:target].value >> env[:msg].eval_arg_at(env, 0).to_i) }
      method(:factorial)  { |env| Number.new((1..env[:target].value).reduce(1, :*)) }
      method(:times)      { |env| r = ::Acute::Nil.instance; env[:target].value.times { r = env[:msg].eval_arg_at(env, 0) }; r }
      method(:asString)   { |env| String.new(env[:target].value.to_s) }
    end

    def <=>(other)
      self.value <=> other.value
    end

    def compare_helper(env, expected_value)
      (env[:target].perform(env.merge(:msg => ::Acute::Message.new("compare", [env[:msg].arguments[0]]))).to_i == expected_value) ? env[:target] : ::Acute::Nil.instance
    end

    def to_i
      value.to_i
    end

    def to_s
      to_i.to_s
    end
=end
  end
end
