require 'spec_helper'

describe ::Acute::Parser do
  before(:each) do
    @parser = ::Acute::Parser.new
  end

  it "builds a message with no arguments" do
    @parser.parse("foo").should == ::Acute::Message.new("foo")
  end
  
  it "builds a single arg message with an argument" do
    m = ::Acute::Message.new("foo", [::Acute::Message.new("bar")])
    m.next = ::Acute::Message.new("baz")
    @parser.parse("foo:bar baz").should == m
  end

  it "builds a message with an argument" do
    @parser.parse("foo(1)").should == ::Acute::Message.new("foo", [::Acute::Message.new("1")])
  end

  it "builds a message with multiple arguments" do
    @parser.parse("foo(1, 2)").should == ::Acute::Message.new("foo", [::Acute::Message.new("1"), ::Acute::Message.new("2")])
  end
  
  it "builds a message with an empty name" do
    @parser.parse("(1, 2)").should == ::Acute::Message.new("", [::Acute::Message.new("1"), ::Acute::Message.new("2")])
  end
  
  it "builds a single arg message with an empty name" do
    @parser.parse(":1").should == ::Acute::Message.new("", [::Acute::Message.new("1")])
  end

  it "recognizes integers" do
    @parser.parse("1").should == ::Acute::Message.new("1")
  end

  it "recognizes strings" do
    @parser.parse('"foo"').to_s.should == ::Acute::String.new("foo").value
  end
  
  it "builds message name as string" do
    @parser.parse("foo").name.class.should == "string".class
  end

  it "has a next message" do
    tree = @parser.parse('foo bar')
    tree.next.should == ::Acute::Message.new("bar")
  end
  
  it "recognizes ';' separator message" do
    tree = @parser.parse('foo ; bar')
    tree.next.name.should == ";"
  end
  
  it "recognizes '\n' separator message" do
    tree = @parser.parse("foo\n bar")
    tree.next.name.should == ";"
  end
  
  it "ignores '\n' separator message preceding ;" do
    tree = @parser.parse("foo\n; bar")
    tree.to_s.should == "foo ; bar"
  end
  
  it "ignores '\n' separator message preceding eof" do
    tree = @parser.parse("foo\n")
    tree.to_s.should == "foo"
  end
  
  it "ignores '\n' separator message preceding )" do
    tree = @parser.parse("foo(a\n)")
    tree.to_s.should == "foo(a)"
  end
  
  it "ignores '\n' separator message after file begining" do
    tree = @parser.parse("\nfoo")
    tree.to_s.should == "foo"
  end
  
  it "ignores '\n' separator message after (" do
    tree = @parser.parse("foo(\n a)")
    tree.to_s.should == "foo(a)"
  end
  
  it "ignores '\n' separator message after :" do
    tree = @parser.parse("foo:\n a")
    tree.to_s.should == "foo(a)"
  end
end
