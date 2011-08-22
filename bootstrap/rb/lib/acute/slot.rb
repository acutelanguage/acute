# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Slot < ::Acute::Object
    attr_accessor :activatable, :data, :cached_result

    def initialize(data, options = { :activatable => false })
      super()
      @activatable = options[:activatable]
      @cached_result = options[:cached_result]
      @data = data
    end

    alias :activatable? :activatable
    alias :cached_result? :cached_result
  end
end
