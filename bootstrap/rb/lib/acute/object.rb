# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Object
    attr_accessor :done_lookup
    attr_reader :slots

    def initialize(init_mt = false)
      @slots = {}
      @done_lookup = false
      method_table if init_mt
    end

    def method_table
      method(:parent)    { |env| ::Acute::Nil.instance }
      method(:clone, &clone_method)
      method(:setSlot)   { |env, name, obj| self.register(name, obj)}
      method(:ifTrue)    { |env, msg| self.perform(env[:sender], :msg => msg) }
      method(:ifFalse)   { |env, msg| ::Acute::Nil.instance }
      method(:slotNames) { |env| ::Acute::List.new(*slots.keys.map { |e| ::Acute::String.new(e).to_s }) }
    end

    def lookup(env = {}, sym)
      lookup_slot = slots[:lookup]
      lookup_func = lookup_slot.data if lookup_slot

      lookup_func ||= lambda do |e, s|
        slot = slots[s.to_sym]
        return slot if slot
        return if done_lookup
        self.done_lookup = true
        parent_slot = slots[:parent]

        if parent_slot
          slot = parent_slot.data.lookup(e, s)
          self.done_lookup = false
          return slot
        end

        self.done_lookup = false
        raise RuntimeError, "Could not find slot '#{sym}'."
      end

      r = lookup_func.call(env, sym.to_sym) if lookup_func && done_lookup == false
      self.done_lookup = false
      r
    end

    def perform(sender, env = {})
      #perform_slot = lookup(env, :perform)
      #return perform_slot.data.call(env, sender) if perform_slot

      return env[:msg].cached_result if env[:msg] and env[:msg].cached_result?
      env.merge!(:sender => sender)
      slot = lookup(env, env[:msg].name)
      if slot and slot.activatable? and slot.data.kind_of? ::Acute::Closure
        func = slot.data
        func.env = env
        activate = func.lookup(env, :activate)
        return activate.data.call env if activate
      end
      return ::Acute::Nil.instance unless slot
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
        o.register(:parent, self)
        begin
          slot = o.lookup(env, "init")
        rescue RuntimeError
          slot = nil
        end
        o.perform(env[:sender], :msg => ::Acute::Message.new("init")) if slot
        o
      end
    end

    protected

    def method(name, &blk)
      register(name, ::Acute::Closure.new(&blk), :activatable => true)
    end

    def message(name, *args)
      ::Acute::Message.new(name, args)
    end

    def string(str)
      ::Acute::String.new(str)
    end

    def eval_in_context(msg, ctx)
      ctx.perform(ctx, :msg => msg)
    end
  end
end
