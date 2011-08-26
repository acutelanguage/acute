require 'spec_helper'

describe ::Acute::Walker do
  it "creates a new walker with context and a tree" do
    msg = ::Acute::Message.new("1")
    ctx = ::Acute::Object.new
    walker = ::Acute::Walker.new(ctx, msg)
    walker.default_context.should == ctx
    walker.tree.should == msg
  end

  it "walks the tree and yields the block" do
    msg = ::Acute::Message.new("foo", [], :cached_result => ::Acute::Number.new(42))
    walker = ::Acute::Walker.new(::Acute::Object.new, msg)
    walker.walk do |obj|
      obj.should == ::Acute::Number.new(42)
    end
  end
end