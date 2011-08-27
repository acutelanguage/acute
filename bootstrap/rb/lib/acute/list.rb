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
      method(:parent)  { |env| env[:sender].perform(env[:sender], ::Acute::Message.new("Object")) }
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
      max_count = value.count - 1
      value.each_with_index do |elem, i|
        s << "#{elem.to_s}#{i != max_count ? ', ' : ''}"
      end
      s << ")"
    end
  end
end
