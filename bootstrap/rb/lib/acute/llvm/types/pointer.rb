# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  module LLVM
    module Types
      class Pointer < Base
        attr_accessor :type

        def initialize
          super()
          @type = nil
          register(:parent, $LLVMTypes)
          method_table
        end

        def method_table
          method(:with) { |env| r = self.perform(env.merge(:msg => ::Acute::Message.new("clone"))); r.type = env[:msg].eval_arg_at(env, 0); r }
          method(:type) { |env| type }
        end

        def to_llvm(builder)
          ::LLVM::Pointer(type.to_llvm(builder))
        end
      end
    end
  end
end
