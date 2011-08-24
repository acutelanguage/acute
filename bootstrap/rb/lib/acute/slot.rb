# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Slot < ::Acute::Object
    attr_accessor :activatable, :data

    def initialize(data, options = { :activatable => false })
      @slots = {}
      @activatable = options[:activatable]
      @data = data
    end

    def to_s
      "<Slot data_type=#{data.class}#{activatable? ? ' activates' : ''}>"
    end

    alias :activatable? :activatable
  end
end
