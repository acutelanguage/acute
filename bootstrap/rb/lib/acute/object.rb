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
      method(:ifTrue)    { |env, msg| self.perform(env[:sender], :msg => msg) }
      method(:ifFalse)   { |env, msg| ::Acute::Nil.instance }
    end

    def lookup(env = {}, sym)
      lookup_func = slots[:lookup]
      lookup_func ||= lambda { |e, s| slots[s.to_sym] }
      slot = lookup_func.call(env, sym.to_sym) if lookup_func
      raise RuntimeError, "Could not find slot '#{sym}' on '#{self.class}'." unless slot
      slot
    end

    def perform(sender, env = {})
      env.merge!(:sender => sender)
      return env[:msg].cached_result if env[:msg] and env[:msg].cached_result?
      slot = lookup env, env[:msg].name
      if slot and slot.activatable? and slot.data.kind_of? ::Acute::Closure
        func = slot.data
        func.env = env
        activate = func.lookup(env, :activate)
        return activate.data.call env if activate
      end
      raise RuntimeError, "Internal Error: Could not find slot '#{env[:msg].name}'" unless slot
      slot.data
    end

    def register(name, obj, options = {})
      slot = Slot.new(obj, options)
      slots[name.to_sym] = slot
      slot
    end

    def clone_method
      lambda do |env|
        o = self.class.new
        self.slots.each { |k,v| o.register(k, v.data, :activatable => v.activatable?) }
        o.register(:parent, self)
        begin
          slot = o.lookup "init"
        rescue RuntimeError
          slot = nil
        end
        o.perform(env[:sender], :msg => ::Acute::Message.new("init")) if slot
        o
      end
    end

    protected

    def method(name, &blk)
      @slots[name.to_sym] = ::Acute::Slot.new(::Acute::Closure.new(&blk), :activatable => true)
    end
  end
end
