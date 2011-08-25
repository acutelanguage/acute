# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Message < ::Acute::Object
    include Comparable
    attr_accessor :name, :arguments, :next, :cached_result

    def initialize(name, arguments = [], options = { cached_result: nil })
      @name = name
      @arguments = arguments
      @next = nil
      @cached_result = options[:cached_result]
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
