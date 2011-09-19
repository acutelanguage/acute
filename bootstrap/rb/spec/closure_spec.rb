require 'spec_helper'

describe ::Acute::Closure do
  before(:each) do
    @closure = ::Acute::Closure.new({}) { |env| env[:msg].eval_arg_at(env, 0).to_i + env[:msg].eval_arg_at(env, 1).to_i }
  end

  it "creates a closure" do
    @closure.should_not be_nil
  end

  it "can call the closure" do
    num_msg = ::Acute::Message.new("a", [], :cached_result => ::Acute::Number.new(1))
    @closure.call({ :target => @closure, :msg => ::Acute::Message.new("add", [num_msg, num_msg]) }).should be 2
  end

  it "is passed the environment" do
    @closure = ::Acute::Closure.new({}) { |env| env }
    @closure.call(:sender => @closure).should == { :sender => @closure }
  end
end
