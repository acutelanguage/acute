require 'spec_helper'

describe ::Acute::Slot do
  it "creates a slot" do
    ::Acute::Slot.new(42, ::Acute::Nil.instance).should_not be_nil
  end

  it "creates an activatable slot" do
    slot = ::Acute::Slot.new(42, ::Acute::Nil.instance, :activatable => true)
    slot.activatable?.should be_true
  end

  it "shows a string representation of a slot" do
    ::Acute::Slot.new(42, ::Acute::Nil.instance, :activatable => true).to_s.should == "<Slot data_type=Fixnum context=nil activates>"
  end

  it "receives the ; message properly" do
    first = ::Acute::Parser.new.parse('createSlot("a", createSlot("foo", 42))')
    tree = ::Acute::Parser.new.parse("a ; foo")
    lobby = $state.find("Lobby")
    lambda { lobby.perform(:target => lobby, :sender => lobby, :msg => first) }.should_not raise_error
    lambda { lobby.perform(:target => lobby, :sender => lobby, :msg => tree) }.should_not raise_error
  end
end
