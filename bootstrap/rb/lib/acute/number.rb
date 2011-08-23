# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Number < ::Acute::Object
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

    def to_i
      value.to_i
    end

    private

    def method(name, &blk)
      @slots[name.to_sym] = ::Acute::Slot.new(::Acute::Closure.new(&blk), :activatable => true)
    end
  end
end