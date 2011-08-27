# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Message < ::Acute::Object
    include Comparable
    attr_accessor :name, :arguments, :next, :cached_result

    def initialize(name, arguments = [], options = { cached_result: nil })
      super()
      @name = name
      @arguments = arguments
      @next = nil
      @cached_result = options[:cached_result]
      register(:parent, $Object)
      method_table
    end

    def method_table
      method(:setNext)         { |env, msg| env[:target].next = eval_in_context(msg, env[:sender]); env[:target] }
      method(:setCachedResult) { |env, msg| env[:target].cached_result = eval_in_context(msg, env[:sender]); env[:target] }
    end

    def perform_on(locals, target)
      r = nil
      ::Acute::Walker.walk(locals, self, target) { |o| r = o }
      r
    end

    def <=>(other)
      to_s <=> other.to_s
    end

    def to_s
      s = ""
      n = self
      begin
        s << "#{n.name}#{n.stringify_arguments} "
      end while n = n.next
      s.strip
    end

    alias :cached_result? :cached_result

    protected

    def stringify_arguments
      s = ""
      if arguments.size > 0
        s << "("
        arguments.each_with_index do |arg, idx|
          s << "#{idx == 0 ? '' : ', '}#{arg}"
        end
        s << ")"
      end
      s
    end
  end
end
