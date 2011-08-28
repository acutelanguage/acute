require 'spec_helper'

describe ::Acute::Block do
  before(:all) do
    @scope = ::Acute::Object.new
    @body = ::Acute::Message.new("1 +(1)")
    @block = ::Acute::Block.new(@scope, @body)
  end

  it "creates a valid block" do
    @block.should_not be_nil
  end

  it "can change scopes" do
    other = ::Acute::List.new
    @block.scope = other
    @block.scope.should be other
  end

  it "can call a block" do
    @block.call(:sender => @scope, :target => @scope, :msg => ::Acute::Message.new("method"))
  end

  it "activates when called by name" do
    @scope.register("test", @block, :activatable => true)
    @scope.perform(:target => @scope, :sender => @scope, :msg => ::Acute::Message.new("test")).should == ::Acute::Number.new(2)
  end
end