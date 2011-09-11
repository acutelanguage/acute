# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

require 'parslet'

module Acute
  class Transformer < Parslet::Transform
    
    rule :code => simple(:exp) do
      exp
    end
    
    rule :current=>{:insignificant=>simple(:insg)}, :next=>subtree(:nxt) do
      nxt
    end
        
    rule :current=>simple(:message), :next=>simple(:nxt) do
      message.next = nxt
      message
    end
    
    rule :sugarSep => simple(:sugarSep) do
      ::Acute::Message.new(";", [])
    end

    rule :integer => simple(:integer) do
      ::Acute::Message.new(integer.to_s, [], { cached_result: ::Acute::Number.new(integer.to_s.to_i) })
    end

    rule :string => simple(:string) do
      ::Acute::Message.new(string.to_s, [], { cached_result: ::Acute::String.new(string.to_s) })
    end

    rule :message => { :identifier => simple(:identifier) } do
      ::Acute::Message.new(identifier.to_s)
    end

    rule :message => { :identifier => simple(:identifier), :args => subtree(:args) } do
      ::Acute::Message.new(identifier.to_s, [*args])
    end
  end
end
