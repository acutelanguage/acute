# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Object
    attr_reader :slots

    def initialize
      @slots = {}
    end

    def lookup(sym)
      lookup_func = slots[:lookup]
      lookup_func ||= lambda { |s| slots[s.to_sym] }
      lookup_func.call sym.to_sym
    end

    def perform(sender, env = {})
      env.merge!(:sender => sender)
      slot = lookup env[:msg].name
      if slot.activatable?
        func = slot.data
        func.env = env
        activate = func.lookup(:activate)
        return activate.data.call env if activate
        raise RuntimeError, "Activatable object cannot activate. Could not find 'activate' method." unless activate
      end
      slot.data
    end

    def register(name, obj, options = {})
      slot = Slot.new(options[:activatable] || false, obj)
      slots[name.to_sym] = slot
      slot
    end
  end
end
