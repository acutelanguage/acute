task :default => [:test]

require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new

task :chdir do |t|
  Dir.chdir "bootstrap/rb"
end

task :test => [:chdir, :spec]
