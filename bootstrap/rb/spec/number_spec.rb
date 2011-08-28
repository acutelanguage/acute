require 'spec_helper'

describe ::Acute::Number do
  before(:each) do
    @num = ::Acute::Number.new(42)
  end

  it "creates a number" do
    @num.should_not be_nil
  end

  it "can tell what its value is" do
    @num.value == 42
  end

  it "can add two numbers" do
    other = ::Acute::Message.new("23", [], :cached_result => ::Acute::Number.new(23))
    msg = ::Acute::Message.new("+")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 65
  end

  it "can subtract two numbers" do
    other = ::Acute::Message.new("23", [], :cached_result => ::Acute::Number.new(23))
    msg = ::Acute::Message.new("-")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 19
  end

  it "can multiply two numbers" do
    other = ::Acute::Message.new("23", [], :cached_result => ::Acute::Number.new(23))
    msg = ::Acute::Message.new("*")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 966
  end

  it "can add divide numbers" do
    other = ::Acute::Message.new("2", [], :cached_result => ::Acute::Number.new(2))
    msg = ::Acute::Message.new("/")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 21
  end

  it "implements bitwise and" do
    other = ::Acute::Message.new("2", [], :cached_result => ::Acute::Number.new(2))
    msg = ::Acute::Message.new("bitwiseAnd")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 2
    msg.arguments = [::Acute::Message.new("1", [], :cached_result => ::Acute::Number.new(1))]
    @num.perform(:msg => msg).value.should be 0
  end

  it "implements bitwise or" do
    other = ::Acute::Message.new("23", [], :cached_result => ::Acute::Number.new(23))
    msg = ::Acute::Message.new("bitwiseOr")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 63
    msg.arguments = [::Acute::Message.new("100", [], :cached_result => ::Acute::Number.new(100))]
    @num.perform(:msg => msg).value.should be 110
  end

  it "implements bitwise xor" do
    other = ::Acute::Message.new("23", [], :cached_result => ::Acute::Number.new(23))
    msg = ::Acute::Message.new("bitwiseXor")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 61
    msg.arguments = [::Acute::Message.new("100", [], :cached_result => ::Acute::Number.new(100))]
    @num.perform(:msg => msg).value.should be 78
  end

  it "implements bitwise not" do
    msg = ::Acute::Message.new("bitwiseNot")
    @num.perform(:msg => msg).value.should be -43
  end

  it "implements left shift" do
    other = ::Acute::Message.new("2", [], :cached_result => ::Acute::Number.new(2))
    msg = ::Acute::Message.new("shiftLeft")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 168
  end

  it "implements right shift" do
    other = ::Acute::Message.new("2", [], :cached_result => ::Acute::Number.new(2))
    msg = ::Acute::Message.new("shiftRight")
    msg.arguments = [other]
    @num.perform(:msg => msg).value.should be 10
  end

  it "converts its representation to a string" do
    ::Acute::Number.new(42).to_s.should == "42"
  end
end