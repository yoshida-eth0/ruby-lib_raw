# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'lib_raw/version'

Gem::Specification.new do |spec|
  spec.name          = "lib_raw"
  spec.version       = LibRaw::VERSION
  spec.authors       = ["Yoshida Tetsuya"]
  spec.email         = ["yoshida.eth0@gmail.com"]
  spec.description   = %q{LibRaw is a library for reading RAW files obtained from digital photo cameras}
  spec.summary       = %q{LibRaw is a library for reading RAW files obtained from digital photo cameras}
  spec.homepage      = ""
  spec.license       = "LGPL"

  spec.files         = `git ls-files`.split($/)
  spec.extensions    = %w[ext/lib_raw/extconf.rb]
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.3"
  spec.add_development_dependency "rake"
end
