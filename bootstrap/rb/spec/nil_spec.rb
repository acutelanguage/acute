require 'spec_helper'

describe ::Acute::Nil do
  before(:each) do
    @nil = ::Acute::Nil.instance
  end

  it "creates the singleton properly" do
    @nil.class.instance.should be @nil
  end

  it "should return the string 'nil' when asked for its string" do
    @nil.to_s.should == "nil"
  end

  it "has a nil value" do
    @nil.value.should be nil
  end

  it "returns itself when being cloned" do
    @nil.perform(:msg => ::Acute::Message.new("clone")).should be @nil
  end

  it "does not evaluate its argument when receiving ifTrue" do
    @nil.perform(:msg => ::Acute::Message.new("ifTrue", [::Acute::Message.new("slotNames")])).should be @nil
  end

  it "evaluates the argument passed to ifFalse" do
    obj = ::Acute::Object.new
    @nil.register(:foo, obj)
    @nil.perform(:sender => @nil, :msg => ::Acute::Message.new("ifFalse", [::Acute::Message.new("foo")])).should be obj
  end
end