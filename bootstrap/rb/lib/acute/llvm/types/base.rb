# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  module LLVM
    module Types
      class Base < ::Acute::LLVM::Core
        def initialize
          super()
          register(:parent, $LLVMCore)
          method_table
        end

        def method_table
        end
      end
    end
  end
end
