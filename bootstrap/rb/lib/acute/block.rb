# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class Block < ::Acute::Object
    # Blocks have a list of argument names, a Call object, a scope, and a body message.
    attr_accessor :argument_names, :scope, :body
    attr_reader :locals

    def initialize(scope, body, *args)
      super()
      @scope = scope
      @body = body
      @locals = ::Acute::Nil.instance
      @argument_names = args[0].map(&:name)
      method_table
    end

    def method_table
      method(:call)   { |env, *args| env[:target].call(env, *args) }
    end

    def call(env, *args)
      create_locals(env[:target])
      env[:slot_context] = locals
      scope = env[:target] if scope.nil?
      body.arguments = argument_names.map.with_index do |name, idx|
        obj = eval_in_context(args.shift, env[:sender])
        locals.register(name, obj)
        ::Acute::Message.new(name, [], :cached_result => obj)
      end
      if args.count > 0
        body.arguments += args
      end
      locals.eval_in_context(body, locals, locals)
    end

    private

    def create_locals(parent)
      @locals = ::Acute::Object.new
      locals.register(:parent, parent)
    end
  end
end
