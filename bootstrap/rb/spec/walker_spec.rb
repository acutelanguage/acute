require 'spec_helper'

describe ::Acute::Walker do
  it "walks the tree and yields the block" do
    msg = ::Acute::Message.new("foo", [], :cached_result => ::Acute::Number.new(42))
    walker = ::Acute::Walker.walk(::Acute::Object.new, msg) do |obj|
      obj.should == ::Acute::Number.new(42)
    end
  end
end