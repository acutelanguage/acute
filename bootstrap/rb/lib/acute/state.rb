# Acute Language
# Copyright © 2011, Jeremy Tregunna, All Rights Reserved.
#

module Acute
  class State
    attr_accessor :lobby
    def initialize(lobby)
      @lobby = lobby
    end

    def init_protos
      object = ::Acute::Object.new(true)
      lobby.register(:Slot, ::Acute::Slot.new(::Acute::Nil.instance, ::Acute::Nil.instance))
      lobby.register(:Lobby, lobby)
      lobby.register(:Object, object)
      lobby.register(:type, ::Acute::String.new("Lobby"))
      lobby.register(:parent, object)
      lobby.register(:List, ::Acute::List.new)
      lobby.register(:String, ::Acute::String.new)
      lobby.register(:Number, ::Acute::Number.new(0, true))
      lobby.register(:nil, ::Acute::Nil.instance)
      lobby.register(:Closure, ::Acute::Closure.new({}))
      lobby.register(:Block, ::Acute::Block.new(nil, nil, []))
      lobby.register(:Message, ::Acute::Message.new(""))
      llvm = ::Acute::LLVM::Core.new
      lobby.register(:LLVM, llvm)
      llvm_types = ::Acute::LLVM::Types::Base.new
      llvm.register(:Types, llvm_types)
      llvm_types.register(:Pointer, ::Acute::LLVM::Types::Pointer.new)
      llvm_types.register(:Pointer, ::Acute::LLVM::Types::Integer.new)
    end

    def find(str)
      slot = lobby.lookup({ :target => lobby }, str)
      slot.data if slot
    end
  end
end
