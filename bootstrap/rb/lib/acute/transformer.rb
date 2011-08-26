# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

require 'parslet'

module Acute
  class Transformer < Parslet::Transform
    rule :expr => subtree(:expr) do
      t = expr.shift
      r = t
      expr.each do |o|
        t.next = o
        t = t.next
      end
      r
    end

    rule :integer => simple(:integer) do
      ::Acute::Message.new(integer, [], { cached_result: ::Acute::Number.new(integer) })
    end

    rule :string => simple(:string) do
      ::Acute::Message.new(string, [], { cached_result: ::Acute::String.new(string) })
    end

    rule :message => { :identifier => simple(:identifier) } do
      ::Acute::Message.new(identifier, [])
    end

    rule :message => { :identifier => simple(:identifier), :args => subtree(:args) } do
      ::Acute::Message.new(identifier, [*args])
    end
  end
end
