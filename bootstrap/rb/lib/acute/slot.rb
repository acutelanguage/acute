# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Slot < ::Acute::Object
    attr_accessor :activatable, :data

    def initialize(activatable = false, data)
      super()
      @activatable = activatable
      @data = data
    end

    alias :activatable? :activatable
  end
end
