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
        method(s)         do |env, n|
          r = ::Acute::Nil.instance
          ::Acute::Walker.new(env[:sender], n).walk do |obj|
            r = obj
          end
          Number.new(value.send(s, r.to_i))
        end
      end
      method(:bitwiseAnd) { |env, n| Number.new(value & eval_in_context(n, env[:sender]).to_i) }
      method(:bitwiseOr)  { |env, n| Number.new(value | eval_in_context(n, env[:sender]).to_i) }
      method(:bitwiseXor) { |env, n| Number.new(value ^ eval_in_context(n, env[:sender]).to_i) }
      method(:bitwiseNot) { |env| Number.new(~value) }
      method(:shiftLeft)  { |env, n| Number.new(value << eval_in_context(n, env[:sender]).to_i) }
      method(:shiftRight) { |env, n| Number.new(value >> eval_in_context(n, env[:sender]).to_i) }
      method(:factorial)  { |env| Number.new((1..value).reduce(1, :*)) }
      method(:times)      { |env, msg| value.times { msg.perform_on(env[:sender], env[:sender]) } }
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