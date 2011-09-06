require 'spec_helper'

describe ::Acute::Closure do
  before(:each) do
    @closure = ::Acute::Closure.new({}) { |env, a, b| a + b }
  end

  it "creates a closure" do
    @closure.should_not be_nil
  end

  it "can call the closure" do
    @closure.call({ :target => @closure }, 1, 2).should be 3
  end

  it "is passed the environment" do
    @closure = ::Acute::Closure.new({}) { |env| env }
    @closure.call(:sender => @closure).should == { :sender => @closure }
  end
end
