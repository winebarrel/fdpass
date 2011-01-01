Gem::Specification.new do |spec|
  spec.name              = 'fdpass'
  spec.version           = '0.1.0'
  spec.summary           = 'This is a library to transmit the file descriptor between the processes.'
  spec.files             = Dir.glob('ext/*.{c,h}') + %w(ext/extconf.rb README)
  spec.author            = 'winebarrel'
  spec.email             = 'sgwr_dts@yahoo.co.jp'
  spec.homepage          = 'https://github.com/winebarrel/fdpass'
  spec.extensions        = 'ext/extconf.rb'
  spec.has_rdoc          = false
end