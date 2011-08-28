# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  module LLVM
    module Types
      class Integer < Base
        attr_accessor :width

        def initialize(width = nil)
          super()
          @width = width
          register(:parent, $LLVMTypes)
          method_table
        end

        def method_table
          method(:with)  { |env| r = self.perform(env.merge(:msg => ::Acute::Message.new("clone"))); r.width = env[:msg].eval_arg_at(env, 0); r }
          method(:width) { |env| ::Acute::Number.new(width) }
        end

        def to_llvm(builder)
          ::LLVM::Int(width.to_i)
        end
      end
    end
  end
end
