# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Block < ::Acute::Object
    # Blocks have a list of argument names, a Call object, a scope, and a body message.
    attr_accessor :argument_names, :scope, :body
    attr_reader :locals

    def initialize(scope, body, args)
      super()
      @scope = scope
      @body = body
      @locals = ::Acute::Nil.instance
      @argument_names = args.map(&:name) rescue [] #args[0].map(&:name) rescue []
      register(:parent, $state.find("Object"))
      method_table
    end

    def method_table
      method(:call)          { |env| env[:target].call(env) }#, env[:msg].arguments) }
      method(:activate)      { |env| activate(env) }
      method(:message)       { |env| env[:target].body }
      method(:argumentNames) { |env| ::Acute::List.new(env[:target].argument_names.map { |a| ::Acute::String.new(a) }) }
      method(:scope)         { |env| env[:target].scope }
      method(:setScope)      { |env| env[:target].scope = env[:msg].eval_arg_at(env, 0); env[:target] }
    end

    def activate(env)
      call(env)#, env[:msg].arguments)
    end

    def call(env)#, args = [])
      #self.scope = $state.find("Object") unless scope
      create_locals(env, scope)
      argument_names.each_with_index do |name, idx|
        obj = env[:msg].eval_arg_at(env, idx)
        locals.register(name, obj)
      end
      body.perform_on(env, locals, locals)
      #doMessage(env.merge(:msg => body, :target => locals), locals)
    end

    def scope=(other)
      @scope = other
      locals.register(:parent, other)
    end

    private

    def create_locals(env, parent)
      @locals = ::Acute::Object.new
      locals.register(:parent, parent)
      locals.register(:self, scope || env[:target])
      call = ::Acute::Object.new
      call.register(:parent, $state.find("Object"))
      %w{target message sender}.each { |str| call.register(str, env[str.to_sym]) }
      call.register(:slotContext, env[:slot_context])
      call.register(:activated, self)
      locals.register(:call, call)
    end
  end
end
