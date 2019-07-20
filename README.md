# Container execution example

Just an example of running a container using direct syscalls on C.

## Running

To build run `make`.
To run, first extract a filesystem to a directory of name "rootfs".
Then run:

```sh
./cnt /bin/ls /
set up pivot root
bin   dev  home  lib64	mnt  proc  run	 srv  tmp  var
boot  etc  lib	 media	opt  root  sbin  sys  usr
```
The program requires CLONE_NEWUSER support on the kernel, otherwise will you need 
to run as root.
