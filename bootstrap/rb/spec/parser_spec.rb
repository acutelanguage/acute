require 'spec_helper'

describe ::Acute::Parser do
  before(:each) do
    @parser = ::Acute::Parser.new
  end

  it "builds a message with no arguments" do
    @parser.parse("foo").should == ::Acute::Message.new("foo")
  end

  it "builds a message with an argument" do
    @parser.parse("foo(1)").should == ::Acute::Message.new("foo", [::Acute::Message.new("1")])
  end

  it "builds a message with multiple arguments" do
    @parser.parse("foo(1, 2)").should == ::Acute::Message.new("foo", [::Acute::Message.new("1"), ::Acute::Message.new("2")])
  end

  it "recognizes integers" do
    @parser.parse("1").should == ::Acute::Message.new("1")
  end

  it "recognizes strings" do
    @parser.parse('"foo"').to_s.should == ::Acute::String.new("foo").value
  end

  it "has a next message" do
    tree = @parser.parse('foo bar')
    tree.next.should == ::Acute::Message.new("bar")
  end
end
