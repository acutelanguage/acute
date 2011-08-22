describe ::Acute::Slot do
  it "creates a slot" do
    ::Acute::Slot.new(42).should_not be_nil
  end

  it "creates an activatable slot" do
    slot = ::Acute::Slot.new(42, :activatable => true)
    slot.activatable?.should be_true
  end
end