# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

require 'parslet'

module Acute
  class Parser < ::Parslet::Parser
    def initialize
      @transformer = ::Acute::Transformer.new
      super
    end

    def parse(code)
      @transformer.apply root.parse(code)
    end

    root :expression

    rule :expression do
      ((literal | message)  >> space?).repeat.as(:expr)
    end

    rule :message do
      (identifier >> (str('(') >> separator? >> arglist.maybe.as(:args) >> separator? >> str(')')).maybe).as(:message)
    end

    rule :arglist do
      expression >> (comma >> expression).repeat
    end

    rule :literal do
      integer | string
    end
    
    rule :space do
      match('\s').repeat(1)
    end

    rule :space? do
      space.maybe
    end

    rule :separator do
      space | match('\n').repeat(1) | match('\t').repeat(1)
    end

    rule :separator? do
      separator.maybe
    end

    rule :comma do
      match(',').repeat(1) >> space?
    end
    
    rule :identifier do
      (match('[a-zA-Z_\+\-\*\/!@$%^&=\.\?:<>\|~;]') >> match('[a-zA-Z0-9_\+\-\*\/!@$%^&=\.\?:<>\|~]').repeat).as(:identifier)
    end
    
    rule :integer do
      ((str('+') | str('-')).maybe >> match("[0-9]").repeat(1)).as(:integer)
    end
    
    rule :string do
      str('"') >> (
        str('\\') >> any |
        str('"').absent? >> any 
      ).repeat.as(:string) >> str('"')
    end
  end
end
