# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Object
    attr_reader :slots

    def initialize(init_mt = false)
      @slots = {}
      method_table if init_mt
    end

    def method_table
      method(:clone, &clone_method)
      method(:slotNames) { |env| ::Acute::List.new(*slots.keys.map { |e| ::Acute::String.new(e).to_s }) }
    end

    def lookup(sym)
      lookup_func = slots[:lookup]
      lookup_func ||= lambda { |s| slots[s.to_sym] }
      return lookup_func.call sym.to_sym if lookup_func
      raise RuntimeError, "Could not find slot '#{sym}'."
    end

    def perform(sender, env = {})
      env.merge!(:sender => sender)
      return env[:msg].cached_result if env[:msg] and env[:msg].cached_result?
      slot = lookup env[:msg].name
      if slot and slot.activatable?
        func = slot.data
        func.env = env.merge(:self => self)
        activate = func.lookup(:activate)
        return activate.data.call env if activate
        raise RuntimeError, "Activatable object cannot activate. Could not find 'activate' method." unless activate
      end
      slot.data
    end

    def register(name, obj, options = {})
      slot = Slot.new(obj, options)
      slots[name.to_sym] = slot
      slot
    end

    def clone_method
      lambda do |env|
        o = Object.new
        o.register(:parent, self)
        o.register(:init, ::Acute::Closure.new { |env| env[:self] })
        o.perform(env[:sender], :msg => ::Acute::Message.new("init"))
        o
      end
    end

    private

    def method(name, &blk)
      @slots[name.to_sym] = ::Acute::Slot.new(::Acute::Closure.new(&blk), :activatable => true)
    end
  end
end
