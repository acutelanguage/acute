# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Object
    attr_accessor :done_lookup, :activatable
    attr_reader :slots

    def initialize(init_mt = false)
      @slots = {}
      @done_lookup = false
      @activatable = false
      method_table if init_mt
    end

    def method_table
      method(:parent)           { |env| ::Acute::Nil.instance }
      method(:clone, &clone_method)
      method(:init)             { |env| env[:target] }
      method(:type)             { |env| ::Acute::String.new(env[:target].class.to_s.split("::").last) }
      method(:getSlot)          { |env| lookup(env, env[:msg].eval_arg_at(env, 0).to_s) }
      method(:setSlot)          { |env| set_slot_helper(env) }
      method(:method)           { |env| args = env[:msg].arguments; ::Acute::Block.new(nil, args.pop, args) }
      method(:ruby)             { |env| env[:target].send(:eval, env[:msg].eval_arg_at(env, 0).to_s) }
      method(:do)               { |env| env[:msg].eval_arg_at(env.merge(:sender => env[:target]), 0); env[:target] }
      method(:';')              { |env| env[:sender] }
      method(:ifTrue)           { |env| env[:msg].eval_arg_at(env, 0) }
      method(:ifFalse)          { |env| env[:msg].eval_arg_at(env, 0); env[:target] }
      method(:slotNames)        { |env| ::Acute::List.new(*env[:target].slots.keys.map { |e| ::Acute::String.new(e).to_s }) }
      method(:list)             { |env| ::Acute::List.new(*env[:msg].eval_args(env)) }
      method(:message)          { |env| env[:msg].arguments[0] }
      method(:doMessage)        { |env| env[:msg].eval_arg_at(env, 0).perform_on(env, env[:sender], env[:target]) }
      method(:uniqueId)         { |env| env[:target].object_id }
      method(:asString)         { |env| ::Acute::String.new("#{env[:target].class.to_s.split('::').last}_0x#{env[:target].object_id}") }
      method(:print)            { |env| str = env[:target].perform(env.merge(:msg => ::Acute::Message.new("asString"))); puts str.to_s; str }
      method(:setIsActivatable) { |env| env[:target].activatable = true }
      method(:isActivatable)    { |env| env[:target].activatable ? env[:target] : ::Acute::Nil.instance }
    end

    def lookup(env, sym)
      lookup_func = slots[:lookup]

      lookup_func ||= lambda do |e, s|
        return if self.done_lookup
        obj = slots[s.to_sym]
        return obj if obj
        self.done_lookup = true
        parent = env[:target].slots[:parent]
        obj = parent.lookup(e, s) if parent
        self.done_lookup = false
        obj
      end

      r = lookup_func.call(env, sym.to_sym) if lookup_func && done_lookup == false
      self.done_lookup = false
      r
    end

    def perform(env)
      return env[:msg].cached_result if env[:msg].cached_result?
      obj = lookup(env, env[:msg].name)
      env[:target] = self
      if obj
        if obj && obj.activatable
          activate_func = obj.lookup(env, :activate)
          return activate_func.call(env.merge(:slot_context => self)) if activate_func
        end
        return obj
      end
      obj = lookup(env, :forward)
      return obj.activate(env.merge(:slot_context => slot.context)) if obj
      raise RuntimeError, "Could not find slot '#{env[:msg].name}' on '#{env[:target].class}'."
    end

    def register(name, obj)
      slots[name.to_sym] = obj
      obj
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
        result = m.cached_result
        result ||= target.perform(env.merge(:target => ctx, :sender => ctx, :msg => m))
        target = result
      end while m = m.next
      result
    end

    protected

    def set_slot_helper(env)
      val = env[:msg].eval_arg_at(env, 1)
      env[:target].register(env[:msg].eval_arg_at(env, 0).to_s, val)
    end

    def method(name, &blk)
      register(name, ::Acute::Closure.new({:target => self}, &blk))
    end

    def message(name, *args)
      ::Acute::Message.new(name, args)
    end

    def string(str)
      ::Acute::String.new(str)
    end
  end
end
