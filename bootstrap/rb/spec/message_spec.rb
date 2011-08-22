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
end