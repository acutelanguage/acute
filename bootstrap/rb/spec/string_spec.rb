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
    str = ::Acute::String.new("testing")
    testable = str.perform(str, :msg => ::Acute::Message.new("with", [::Acute::Message.new("str", [], :cached_result => str)]))
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
    str.perform(str, :msg => ::Acute::Message.new("append", [::Acute::Message.new("second", [], :cached_result => second)])).value.should == "foobar"
  end

  it "can prepend to an existing string" do
    str = ::Acute::String.new("foo")
    second = ::Acute::String.new("bar")
    str.perform(str, :msg => ::Acute::Message.new("prepend", [::Acute::Message.new("second", [], :cached_result => second)])).value.should == "barfoo"
  end
end