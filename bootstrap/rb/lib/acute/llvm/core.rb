# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  module LLVM
    class Core < ::Acute::Object
      def initialize
        super()
        register(:parent, $Object)
        method_table
      end

      def method_table
      end
    end
  end
end
