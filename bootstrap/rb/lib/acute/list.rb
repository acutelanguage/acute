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
      register(:parent, $state.find("Object"))
    end

    def method_table
      method(:at)      { |env| env[:target].value[env[:msg].eval_arg_at(env, 0).to_i] }
      method(:append)  { |env| env[:target].value << env[:msg].eval_arg_at(env, 0); self }
      method(:prepend) { |env| env[:target].value.unshift env[:msg].eval_arg_at(env, 0); self }
      method(:isEmpty) { |env| env[:target].value.empty? }
      method(:size)    { |env| ::Acute::Number.new env[:target].value.count }
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
