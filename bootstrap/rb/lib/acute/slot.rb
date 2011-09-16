# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Slot < ::Acute::Object
    attr_accessor :activatable, :data, :context

    def initialize(data, context, options = { :activatable => false })
      @slots = {}
      @context = context
      @activatable = options[:activatable]
      @data = data
      dummy_slot = self.dup
      dummy_slot.context = $state.find("Object")
      dummy_slot.data = $state.find("Object")
      @slots[:parent] = dummy_slot
    end

    def to_s
      "<Slot data_type=#{data.class} context=#{context}#{activatable? ? ' activates' : ''}>"
    end

    alias :activatable? :activatable
  end
end
