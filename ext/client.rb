require 'fdpass'

  fdpass = FDPass.client('/tmp/fdpass.sock')
  fdpass.send(1)
#  fdpass.close
#  sleep 1
#end
