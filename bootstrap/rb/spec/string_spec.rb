require 'spec_helper'

describe ::Acute::String do
  it "creates a new empty string" do
    str = ::Acute::String.new("")
    str.should_not be_nil
    str.value.should == ""
  end

  it "creates a new string with content" do
    str = ::Acute::String.new("foobar")
    str.should_not be_nil
    str.value.should == "foobar"
  end

  it "creates a new string with the factory 'with'" do
    str_proto = ::Acute::String.new
    replacement = ::Acute::String.new("testing")
    testable = str_proto.perform(str_proto, :msg => ::Acute::Message.new("setString", [replacement]))
    testable.value.should == replacement.value
  end

  it "can append to an existing string" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(str, :msg => ::Acute::Message.new("append", [second])).value.should == "foobar"
  end

  it "can prepend to an existing string" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(str, :msg => ::Acute::Message.new("prepend", [second])).value.should == "barfoo"
  end
end