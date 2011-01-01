require 'fdpass'
  fdpass = FDPass.server('/tmp/fdpass.sock')

th = Thread.fork do
  loop do
    
  
  fd = fdpass.recv
  IO.for_fd(fd).puts 100
  end
end

th.join