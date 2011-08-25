require 'spec_helper'

describe ::Acute::Object do
  before(:each) do
    @obj = ::Acute::Object.new(true)
  end

  it "is a valid object" do
    @obj.should_not be_nil
  end

  it "registers a slot on the object" do
    @obj.register("fortyTwo", 42)
    @obj.slots[:fortyTwo].data.should == 42
  end

  it "looks up a slot" do
    @obj.register("fortyTwo", 42)
    slot = @obj.lookup(:fortyTwo)
    slot.should be_an_instance_of(::Acute::Slot)
    slot.data.should be 42
  end

  it "cannot find a non-existent slot" do
    lambda { @obj.lookup(:amazingStuff) }.should raise_error(RuntimeError, "Could not find slot 'amazingStuff'.")
  end

  it "stores an activatable message in the slot table" do
    @obj.register("add", ::Acute::Closure.new { |env, a, b| a + b }, :activatable => true)
    slot = @obj.lookup(:add)
    slot.data.should be_an_instance_of(::Acute::Closure)
    slot.data.func.should be_an_instance_of(Proc)
  end

  it "performs a message that will return the value in the slot" do
    @obj.register("fortyTwo", 42)
    msg = ::Acute::Message.new("fortyTwo")
    @obj.perform(@obj, :msg => msg).should be 42
  end

  it "performs a message that will activate a closure" do
    @obj.register("add", ::Acute::Closure.new { |env, a, b| a + b }, :activatable => true)
    msg = ::Acute::Message.new("add", [1, 2])
    @obj.perform(@obj, :msg => msg).should be 3
  end

  it "can clone itself" do
    @obj.perform(@obj, :msg => ::Acute::Message.new("clone")).should_not be_nil
  end

  it "has a parent" do
    new_obj = @obj.perform(@obj, :msg => ::Acute::Message.new("clone"))
    new_obj.lookup("parent").data.should_not be_nil
  end

  it "displays the correct slot table" do
    @obj.perform(@obj, :msg => ::Acute::Message.new("slotNames")).value.should == @obj.slots.keys.map(&:to_s)
  end
end