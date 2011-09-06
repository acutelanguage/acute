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

  it "is able to set the string contents" do
    str = ::Acute::String.new("testing")
    testable = str.perform(:target => str, :msg => ::Acute::Message.new("setString", [::Acute::Message.new("str", [], :cached_result => str)]))
    testable.value.should == str.value
  end

  it "compares two identical strings in ruby" do
    str = ::Acute::String.new("foo")
    (str <=> str).should be 0
  end

  it "compares two different strings in ruby" do
    str = ::Acute::String.new("foo")
    other = ::Acute::String.new("bar")
    (str <=> other).should be 1
  end

  it "can append to an existing string" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(:target => str, :msg => ::Acute::Message.new("append", [::Acute::Message.new("second", [], :cached_result => second)])).value.should == "foobar"
  end

  it "can prepend to an existing string" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(:target => str, :msg => ::Acute::Message.new("prepend", [::Acute::Message.new("second", [], :cached_result => second)])).value.should == "barfoo"
  end

  it "creates a new string using the factory method 'with'" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(:target => str, :sender => str, :msg => ::Acute::Message.new("with", [::Acute::Message.new("second", [], :cached_result => second)])).value.should == "bar"
  end
  
  it "can be converted into a message with correct name" do
    str = ::Acute::String.new("foo")
    msg = str.perform(:target => str, :sender => str, :msg => ::Acute::Message.new("asMessage"))
    msg.name.should == "foo"
  end
  
  it "can be converted into a message chain with correct next" do
    str = ::Acute::String.new("foo bar")
    msg = str.perform(:target => str, :sender => str, :msg => ::Acute::Message.new("asMessage"))
    msg.next.name.should == "bar"
  end
  
  it "can be converted into a message with correct arguments" do
    str = ::Acute::String.new("foo(me, foo you)")
    msg = str.perform(:target => str, :sender => str, :msg => ::Acute::Message.new("asMessage"))
    msg.arguments.should == ["me", "foo you"]
  end
  
end
