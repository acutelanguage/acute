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
      method(:'')        { |env, msg| r = nil; ::Acute::Walker.walk(msg, env[:sender]) { |o| r = o }; r }
      method(:parent)    { |env| ::Acute::Nil.instance }
      method(:clone, &clone_method)
      method(:init)      { |env| self }
      method(:setSlot)   { |env, name, obj| env[:target].register(eval_in_context(name, env[:sender]).value, eval_in_context(obj, env[:sender])) }
      method(:method)    { |env, *args| ::Acute::Block.new(nil, args.pop, args) }
      method(:ifTrue)    { |env, msg| env[:target].perform(env[:sender], :msg => msg) }
      method(:ifFalse)   { |env, msg| ::Acute::Nil.instance }
      method(:slotNames) { |env| ::Acute::List.new(*env[:target].slots.keys.map { |e| ::Acute::String.new(e).to_s }) }
      method(:list)      { |env, *args| ::Acute::List.new(*args.map { |a| eval_in_context(a, env[:sender]) }) }
      method(:message)   { |env, msg| msg }
      method(:doMessage) { |env, msg| eval_in_context(eval_in_context(msg, env[:sender]), env[:sender], env[:target]) }
    end

    def lookup(env, sym)
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
          raise RuntimeError, "Could not find slot '#{sym}' on '#{env[:target].class}'." unless slot
          return slot
        end

        self.done_lookup = false
        raise RuntimeError, "Could not find slot '#{sym}' on '#{env[:target].class}'."
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
        o = env[:target].class.new
        o.register(:parent, env[:target])
        slot = o.lookup(env, "init")
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

    def eval_in_context(msg, ctx, receiver = nil)
      r = ::Acute::Nil.instance
      Walker.walk(ctx, msg, receiver) { |o| r = o }
      r
    end
  end
end
