require 'simplecov'
SimpleCov.start
require 'rubygems'

# Set up gems listed in the Gemfile.
gemfile = File.expand_path('../Gemfile', File.dirname(__FILE__))
begin
  ENV['BUNDLE_GEMFILE'] = gemfile
  require 'bundler'
  Bundler.setup
rescue Bundler::GemNotFound => e
  STDERR.puts e.message
  STDERR.puts "Try running `bundle install`."
  exit!
end

Bundler.require(:spec)

RSpec.configure do |config|
  config.mock_with :rspec

  config.before(:all) do
    lobby = ::Acute::Object.new
    $state = ::Acute::State.new(lobby)
    $state.init_protos
  end
end

require 'acute'

Dir["#{File.dirname(__FILE__)}/support/**/*.rb"].each {|f| require f}
