# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Message < ::Acute::Object
    attr_accessor :name, :arguments

    def initialize(name, arguments = [])
      @name = name
      @arguments = arguments
    end
  end
end
