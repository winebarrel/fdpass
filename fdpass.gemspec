Gem::Specification.new do |spec|
  spec.name              = 'fdpass'
  spec.version           = '0.1.2'
  spec.summary           = 'This is a library to transmit the file descriptor between the processes.'
  spec.files             = Dir.glob('ext/*.{c,h}') + %w(ext/extconf.rb README)
  spec.author            = 'winebarrel'
  spec.email             = 'sgwr_dts@yahoo.co.jp'
  spec.homepage          = 'https://bitbucket.org/winebarrel/fdpass'
  spec.extensions        = 'ext/extconf.rb'
  spec.has_rdoc          = false
end
