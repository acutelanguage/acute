# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

require 'enumerator'

module Acute
  class Message < ::Acute::Object
    include Comparable
    attr_accessor :name, :arguments, :next, :cached_result, :line_number, :character_number, :origin

    def initialize(name, arguments = [], options = { :cached_result => nil, :line_number => -1, :character_number => -1, :origin => nil})
      super()
      @name = name
      @arguments = arguments
      @next = nil
      @cached_result = options[:cached_result]
      @line_number = options[:line_number]
      @character_number = options[:character_number]
      @origin = options[:origin]
      register(:parent, $state.find("Object"))
      method_table
    end

    def method_table
      method(:next)            { |env| env[:target].next }
      method(:setNext)         { |env| env[:target].next = env[:msg].eval_arg_at(env, 0); env[:target] }
      method(:setCachedResult) { |env| env[:target].cached_result = env[:msg].eval_arg_at(env, 0); env[:target] }
      method(:asString)        { |env| String.new(to_s) }
      method(:lineNumber)      { |env| ::Acute::Number.new(line_number) }
      method(:characterNumber) { |env| ::Acute::Number.new(character_number) }
      method(:origin)          { |env| ::Acute::String.new(origin)}
    end

    def eval_arg_at(env, idx)
      msg = arguments[idx]
      if msg
        return msg.cached_result if msg.cached_result? && msg.next.nil?
        return msg.perform_on(env.merge(:msg => msg), env[:sender], env[:sender])
      end
      ::Acute::Nil.instance
    end

    def eval_args(env)
      #arguments.map.with_index do |arg, idx|
      arguments.to_enum(:each_with_index).map do |arg, idx|
        eval_arg_at(env, idx)
      end
    end

    def perform_on(env, locals, target = env[:target])
      result = target
      cached_target = target
      m = self

      begin
        result = m.cached_result
        result ||= target.perform(env.merge(:target => target, :sender => locals, :msg => m))
        target = result
      end while m = m.next
      result
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
