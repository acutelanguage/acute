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
      @argument_names = args[0].map(&:name) rescue []
      register(:parent, $Object)
      method_table
    end

    def method_table
      method(:call)     { |env| env[:target].call(env, *env[:msg].arguments) }
      method(:activate) { |env| activate(env) }
    end

    def activate(env)
      call(env, *env[:msg].arguments)
    end

    def call(env, *args)
      self.scope = env[:sender] unless scope
      create_locals(scope)
      argument_names.each_with_index do |name, idx|
        obj = locals.perform(env.merge(:msg => args[idx]))
        locals.register(name, obj)
      end
      doMessage(env.merge(:msg => body, :target => locals), locals)
    end

    def scope=(other)
      @scope = other
      locals.register(:parent, other)
    end

    private

    def create_locals(parent)
      @locals = ::Acute::Object.new
      locals.register(:parent, parent)
      locals.register(:self, scope)
    end
  end
end
