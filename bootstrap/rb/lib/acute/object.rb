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
      method(:init)      { |env| self }
      method(:setSlot)   { |env| slots[env[:msg].eval_arg_at(env, 0).to_s] = ::Acute::List.new}
      method(:method)    { |env, *args| ::Acute::Block.new(nil, args.pop, args) }
      method(:ifTrue)    { |env| env[:msg].eval_arg_at(env, 0) }
      method(:ifFalse)   { |env| ::Acute::Nil.instance }
      method(:slotNames) { |env| ::Acute::List.new(*env[:target].slots.keys.map { |e| ::Acute::String.new(e).to_s }) }
      method(:list)      { |env| ::Acute::List.new(*env[:msg].eval_args(env)) }
      method(:message)   { |env| env[:msg].arguments[0] }
      method(:doMessage) { |env| env[:msg].eval_arg_at(env, 0).perform_on(env, env[:sender], env[:target]) }
      method(:asString)  { |env| ::Acute::String.new("#{env[:target].class.to_s.split('::').last}_0x#{self.object_id}") }
      method(:print)     { |env| str = env[:target].perform(env.merge(:msg => ::Acute::Message.new("asString"))); puts str.to_s; str }
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
          #raise RuntimeError, "Could not find slot '#{sym}' on '#{env[:target].class}'." unless slot
          return slot
        end

        self.done_lookup = false
        return nil
        raise RuntimeError, "Could not find slot '#{sym}' on '#{env[:target].class}'."
      end

      r = lookup_func.call(env, sym.to_sym) if lookup_func && done_lookup == false
      self.done_lookup = false
      r
    end

    def perform(env)
      return env[:msg].cached_result if env[:msg].cached_result?
      slot = lookup(env, env[:msg].name)
      if slot && slot.data
        if slot.activatable?
          activate_func = slot.data.lookup(env, :activate)
          return activate_func.data.call(env.merge(:slot_context => slot.context)) if activate_func
        end
        return slot.data
      end
      slot = lookup(env, :forward)
      slot.data.activate(env.merge(:slot_context => slot.context)) if slot && slot.activatable?
      raise RuntimeError, "Could not find slot '#{env[:msg].name}' on '#{env[:target].class}'."
    end

    def register(name, obj, options = {})
      slot = Slot.new(obj, self, options)
      slots[name.to_sym] = slot
      slot
    end

    def clone_method
      lambda do |env|
        o = env[:target].class.new
        o.register(:parent, env[:target])
        slot = o.lookup(env, "init")
        o.perform(env.merge(:msg => ::Acute::Message.new("init"))) if slot
        o
      end
    end

    def doMessage(env, ctx)
      target = env[:target]
      result = target
      cached_target = target
      m = env[:msg]

      begin
        if m.name.eql? ';'
          target = cached_target
        else
          result = m.cached_result
          result ||= target.perform(env.merge(:target => ctx, :sender => ctx, :msg => m))
          target = result
        end
      end while m = m.next
      result
    end

    protected

    def method(name, &blk)
      register(name, ::Acute::Closure.new({:target => self}, &blk), :activatable => true)
    end

    def message(name, *args)
      ::Acute::Message.new(name, args)
    end

    def string(str)
      ::Acute::String.new(str)
    end
  end
end
