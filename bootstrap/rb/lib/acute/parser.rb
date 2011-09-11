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

    root :code
    
    rule :code do
      expression.maybe.as(:code)
    end

    rule :expression do
      atom.as(:current) >> expression.maybe.as(:next)
    end
    
    rule :atom do
      literal | message | sugar_separator | insignificant_separator
    end

    rule :message do
      (identifier >> (opener >> arglist.as(:args).maybe >> closer).maybe).as(:message)
    end

    rule :arglist do
      expression >> (comma >> expression).repeat
    end

    rule :literal do
      integer | string
    end
    
    rule :identifier do
      match('[a-zA-Z0-9_\+\-\*\/!@$%^&=\.\?:<>\|~;]').repeat(1).as(:identifier)
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
    
    rule :sugar_separator do
      (terminating_separator >> terminator.absent?).as(:sugarSep)
    end
    
    rule :insignificant_separator do
      separator.as(:insignificant)
    end
    
    rule :opener do
      str('(') >> separator.maybe
    end

    rule :closer do
      str(')')
    end
    
    rule :comma do
      match(',').repeat(1) >> separator.maybe
    end
    
    rule :space do
      str(' ').repeat(1)
    end

    rule :separator do
      terminating_separator | non_terminating_separator
    end
    
    rule :terminating_separator do
      non_terminating_separator.maybe >> match('\n') >> (non_terminating_separator | match('\n')).repeat
    end
    
    rule :non_terminating_separator do
      (space | match('\t')).repeat(1)
    end
    
    rule :terminator do
      closer | comma | str(";") | any.absent?
    end
  end
end
