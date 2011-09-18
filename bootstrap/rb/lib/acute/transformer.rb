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
      lc = sugarSep.line_and_column
      l = lc[0]
      c = lc[1]
      ::Acute::Message.new(";", [], {:cached_result => nil, :line_number => l, :character_number => c})
    end

    rule :integer => simple(:integer) do
      lc = integer.line_and_column
      l = lc[0]
      c = lc[1]
      ::Acute::Message.new(integer.to_s, [], {:cached_result => ::Acute::Number.new(integer.to_s.to_i), :line_number => l, :character_number => c})
    end

    rule :string => simple(:string) do
      lc = string.line_and_column
      l = lc[0]
      c = lc[1]-1
      ::Acute::Message.new(string.to_s, [], {:cached_result => ::Acute::String.new(string.to_s), :line_number => l, :character_number => c})
    end

    rule :message => { :identifier => simple(:identifier) } do
      lc = identifier.line_and_column
      l = lc[0]
      c = lc[1]
      ::Acute::Message.new(identifier.to_s, [], {:cached_result => nil, :line_number => l, :character_number => c})
    end

    rule :message => { :identifier => simple(:identifier), :opener => simple(:opener), :args => subtree(:args) } do
      lc = (identifier or opener).line_and_column
      l = lc[0]
      c = lc[1]
      ::Acute::Message.new(identifier.to_s, [*args], {:cached_result => nil, :line_number => l, :character_number => c})
    end
  end
end
