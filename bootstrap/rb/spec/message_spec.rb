require 'spec_helper'

describe ::Acute::Message do
  it "creates a valid message with a name" do
    m = ::Acute::Message.new("test")
    m.should_not be_nil
    m.name.should.eql? "test"
  end

  it "creates a valid message with a name and arguments" do
    m = ::Acute::Message.new("test", [1, 2])
    m.should_not be_nil
    m.name.should.eql? "test"
    m.arguments.should == [1, 2]
  end

  it "changes the message name" do
    m = ::Acute::Message.new("test")
    m.name = "foo"
    m.name.should.eql? "foo"
  end

  it "adds a new argument" do
    m = ::Acute::Message.new("test", [1])
    m.arguments << 2
    m.arguments.should == [1, 2]
  end

  it "replaces the arguments" do
    m = ::Acute::Message.new("test", [1, 2])
    m.arguments = [5, 6]
    m.arguments.should == [5, 6]
  end

  it "sets the next message" do
    m1 = ::Acute::Message.new("testing")
    m2 = ::Acute::Message.new("stuff", [1, 2])
    m1.next = m2
    m1.to_s.should.eql? "testing stuff"
  end

  it "sets a cached result" do
    m = ::Acute::Message.new("testing")
    m.cached_result = 42
    m.cached_result.should be 42
  end

  it "returns the cached result in perform" do
    obj = ::Acute::Object.new
    other = ::Acute::Object.new
    obj.register("object", obj)
    m = ::Acute::Message.new("object")
    m.cached_result = other
    obj.perform(:msg => m).should be other
  end

  it "performs a message on a context" do
    msg = ::Acute::Message.new("foo")
    locals = ::Acute::Object.new
    target = ::Acute::Object.new
    target.register("foo", ::Acute::Number.new(42))
    msg.perform_on({:msg => msg}, locals, target).value.should be 42
  end

  it "tries to evaluate an argument out of bounds" do
    msg = ::Acute::Message.new("foo", [::Acute::Message.new("bar")])
    msg.eval_arg_at({}, 5).should be ::Acute::Nil.instance
  end

  it "evaluates all arguments" do
    msg = ::Acute::Message.new("foo", [::Acute::Message.new("bar", [], :cached_result => 1)])
    msg.eval_args({}).should == [1]
  end
end