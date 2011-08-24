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
    end

    def method_table
      %w{+ - * /}.each do |s|
        method(s)         { |env, n| Number.new(value.send(s, n.to_i)) }
      end
      method(:bitwiseAnd) { |env, n| Number.new(value & n.to_i) }
      method(:bitwiseOr)  { |env, n| Number.new(value | n.to_i) }
      method(:bitwiseXor) { |env, n| Number.new(value ^ n.to_i) }
      method(:bitwiseNot) { |env| Number.new(~value) }
      method(:shiftLeft)  { |env, n| Number.new(value << n.to_i) }
      method(:shiftRight) { |env, n| Number.new(value >> n.to_i) }
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