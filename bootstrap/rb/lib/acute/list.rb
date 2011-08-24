# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class List < ::Acute::Object
    include Comparable
    attr_reader :value

    def initialize(*args)
      super()
      @value = args
      method_table
    end

    def method_table
      method(:append)  { |env, o| value << o }
      method(:prepend) { |env, o| value.unshift o }
      method(:isEmpty) { |env| value.empty? }
      method(:size)    { |env| ::Acute::Number.new value.count }
    end

    def <=>(other)
      self.value <=> other.value
    end

    def to_s
      s = "list("
      value.each do |elem|
        s << elem.to_s
      end
      s << ")"
    end
  end
end
