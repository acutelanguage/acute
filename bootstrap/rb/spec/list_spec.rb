require 'spec_helper'

describe ::Acute::List do
  before(:each) do
    @list = ::Acute::List.new
  end

  it "creates a new empty list" do
    @list.should_not be_nil
    @list.should be_an_instance_of(::Acute::List)
  end

  it "can compare two empty lists" do
    r = @list <=> ::Acute::List.new
    r.should be 0
  end

  it "can compare an empty list to a populated list" do
    r = @list <=> ::Acute::List.new(1, 2)
    r.should be -1
  end

  it "outputs a string representation of an empty list" do
    @list.to_s.should == "list()"
  end

  it "outputs a string representation of a populated list" do
    @list.perform(@list, :msg => ::Acute::Message.new("append", [1])).should_not be_nil
    @list.to_s.should == "list(1)"
  end

  it "outputs a string representation of a populated list with multiple arguments" do
    @list.perform(@list, :msg => ::Acute::Message.new("append", [1])).should_not be_nil
    @list.perform(@list, :msg => ::Acute::Message.new("append", [2])).should_not be_nil
    @list.to_s.should == "list(1, 2)"
  end

  it "is empty" do
    @list.perform(@list, :msg => ::Acute::Message.new("isEmpty")).should be_true
  end

  it "size is zero" do
    @list.perform(@list, :msg => ::Acute::Message.new("size")).should == ::Acute::Number.new(0)
  end

  it "appends an item to the list" do
    @list.perform(@list, :msg => ::Acute::Message.new("append", [1])).should_not be_nil
    @list.perform(@list, :msg => ::Acute::Message.new("isEmpty")).should_not be_true
  end

  it "prepends an item to the list" do
    @list.perform(@list, :msg => ::Acute::Message.new("prepend", [1])).should_not be_nil
    @list.perform(@list, :msg => ::Acute::Message.new("isEmpty")).should_not be_true
  end
end
