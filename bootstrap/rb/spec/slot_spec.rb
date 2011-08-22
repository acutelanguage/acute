describe ::Acute::Slot do
  it "creates a slot" do
    ::Acute::Slot.new(42).should_not be_nil
  end

  it "creates an activatable slot" do
    slot = ::Acute::Slot.new(42, :activatable => true)
    slot.activatable?.should be_true
  end

  it "sets a cached result" do
    slot = ::Acute::Slot.new(42)
    slot.cached_result = 42
    slot.cached_result.should be 42
  end

  it "returns the cached result in perform" do
    obj = ::Acute::Object.new
    other = ::Acute::Object.new
    obj.register("object", obj)
    slot = obj.lookup "object"
    slot.should_not be_nil
    slot.cached_result = other
    obj.perform(obj, :msg => ::Acute::Message.new("object")).should be other
  end
end