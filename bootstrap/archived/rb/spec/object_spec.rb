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
    @obj.slots[:fortyTwo].should == 42
  end

  it "looks up a slot" do
    @obj.register("fortyTwo", 42)
    slot = @obj.lookup({ :target => @obj }, :fortyTwo)
    slot.should be 42
  end

  it "cannot find a non-existent slot" do
    @obj.lookup({ :target => @obj }, :amazingStuff).should be_nil
    lambda { @obj.perform({:target => @obj, :msg => ::Acute::Message.new("amazingStuff")}) }.should raise_error(RuntimeError, "Could not find slot 'amazingStuff' on 'Acute::Object'.")
  end

  it "stores an activatable message in the slot table" do
    @obj.register("add", ::Acute::Closure.new({}) { |env, a, b| a + b })
    slot = @obj.lookup({ :target => @obj }, :add)
    slot.should be_an_instance_of(::Acute::Closure)
    slot.func.should be_an_instance_of(Proc)
  end

  it "performs a message that will return the value in the slot" do
    number = ::Acute::Number.new(42)
    @obj.register("fortyTwo", number)
    msg = ::Acute::Message.new("fortyTwo")
    @obj.perform(:target => @obj, :msg => msg).should be number
  end

  it "performs a message that will activate a closure" do
    @obj.register("add", ::Acute::Closure.new({}) { |env| env[:msg].eval_arg_at(env, 0).to_i + env[:msg].eval_arg_at(env, 1).to_i })
    msg = ::Acute::Message.new("add", [::Acute::Message.new("a", [], :cached_result => ::Acute::Number.new(1)), ::Acute::Message.new("b", [], :cached_result => ::Acute::Number.new(2))])
    @obj.perform(:target => @obj, :msg => msg).should be 3
  end

  it "sets a slot in its slot table" do
    num = ::Acute::Number.new(42)
    arg1 = ::Acute::Message.new("\"foo\"", [], :cached_result => ::Acute::String.new("foo"))
    arg2 = ::Acute::Message.new("42", [], :cached_result => num)
    msg = ::Acute::Message.new("setSlot", [ arg1, arg2 ])
    @obj.perform(:target => @obj, :sender => @obj, :msg => msg).should be num
  end

  it "can clone itself" do
    @obj.perform(:target => @obj, :msg => ::Acute::Message.new("clone")).should_not be_nil
  end

  it "has a parent" do
    new_obj = @obj.perform(:target => @obj, :msg => ::Acute::Message.new("clone"))
    new_obj.lookup({ :target => @obj }, "parent").should_not be_nil
  end

  it "found a parent slot with a non-nil value" do
    obj = ::Acute::Object.new
    parent = ::Acute::Object.new
    obj.register("parent", parent)
    obj.perform(:target => obj, :msg => ::Acute::Message.new("parent", [])).should == parent
  end
 
  it "displays the correct slot table" do
    @obj.perform(:target => @obj, :msg => ::Acute::Message.new("slotNames")).value.should == @obj.slots.keys.map(&:to_s)
  end

  it "creates a message conveniently" do
    obj = ::Acute::Object.new
    obj.send(:message, 'foo', 1, 2).should == ::Acute::Message.new("foo", [1, 2])
  end

  it "creates a string conveniently" do
    obj = ::Acute::Object.new
    obj.send(:string, 'foo').value.should == 'foo'
  end

  it "can evaluate a message" do
    msg = ::Acute::Message.new("message", [::Acute::Message.new("slotNames")])
    @obj.perform(:target => @obj, :sender => @obj, :msg => ::Acute::Message.new("doMessage", [msg])).value.should == @obj.slots.keys.map(&:to_s)
  end

  it "can evaluate a message chain and handles ; properly" do
    m1 = ::Acute::Message.new("slotNames")
    m1.next = ::Acute::Message.new(";")
    m1.next.next = ::Acute::Message.new("list")
    msg = ::Acute::Message.new("message", [m1])
    @obj.doMessage({:target => @obj, :sender => @obj, :msg => ::Acute::Message.new("doMessage", [msg])}, @obj).should == ::Acute::List.new
  end
end
