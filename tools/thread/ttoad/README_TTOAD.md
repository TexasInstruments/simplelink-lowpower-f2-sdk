# TI Openthread OAD Server

This describes how to build and run TI Openthread OAD server program.

For more details on the OAD server itself, please refer to the
[OAD section of the Users Guide](./../../../docs/thread/html/thread-oad/native-oad.html).

### Prerequisites

This program can be built and run in Openthread Border Router. You should get
Openthread Border Router ready before start. This guide assumes a border router
has been set up based on
[SimpleLink Academy](http://dev.ti.com/tirex/explore/node?node=ABSaAO-k3rswvnB7RdRtww__pTTHBmu__LATEST).

For transferring files from your PC to the border router, a free program called
WinSCP may be installed and used.

### Installing

1.  Transfer **doorlock_oad_CC1352R1_LAUNCHXL_tirtos_gcc_oad.bin** to your
    border router using WinSCP. Note that
    this image should have a different software version ```MY_APP_SOFTWARE_VER```
    than the image running on the doorlock. Location should be 
    **doorlock_oad_CC1352R1_LAUNCHXL_tirtos_gcc_oad.bin** under
    ```/home/debian/images```.

    In border router, you should see

    ```
    debian@beaglebone:~/images$ ls -al
    total 160
    drwxr-xr-x 2 debian debian   4096 Mar 19  2018 .
    drwxr-xr-x 6 debian debian   4096 Mar  5 08:10 ..
    -rw-r--r-- 1 debian debian 154084 Oct 19  2018 doorlock_oad_CC1352R1_LAUNCHXL_tirtos_gcc_oad.bin

    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo$ ls -al
    total 1748
    drwxr-xr-x 11 debian debian   4096 Mar  5 08:15 .
    drwxr-xr-x  5 debian debian   4096 Mar  6  2018 ..
    -rw-r--r--  1 debian debian  55637 Sep 10  2018 aclocal.m4
    -rwxr-xr-x  1 debian debian   5826 Sep 10  2018 ar-lib
    -rw-r--r--  1 debian debian     70 Sep 10  2018 AUTHORS
    -rwxr-xr-x  1 debian debian   3264 Sep 10  2018 autogen.sh
    drwxr-xr-x  2 debian debian   4096 Sep 10  2018 autom4te.cache
    -rw-r--r--  1 debian debian   7086 Sep 10  2018 ChangeLog
    -rw-r--r--  1 debian debian   3997 Sep 10  2018 coap_config.h
    -rw-r--r--  1 debian debian   3669 Sep 10  2018 coap_config.h.in
    -rwxr-xr-x  1 debian debian   7333 Sep 10  2018 compile
    -rwxr-xr-x  1 debian debian  43940 Sep 10  2018 config.guess
    -rw-r--r--  1 debian debian  41696 Sep 10  2018 config.log
    -rwxr-xr-x  1 debian debian  60628 Sep 10  2018 config.status
    -rwxr-xr-x  1 debian debian  36339 Sep 10  2018 config.sub
    -rwxr-xr-x  1 debian debian 489546 Sep 10  2018 configure
    -rw-r--r--  1 debian debian  18563 Sep 10  2018 configure.ac
    -rw-r--r--  1 debian debian   9412 Sep 10  2018 CONTRIBUTE
    -rw-r--r--  1 debian debian    209 Sep 10  2018 COPYING
    -rwxr-xr-x  1 debian debian  23566 Sep 10  2018 depcomp
    drwxr-xr-x  2 debian debian   4096 Sep 10  2018 doc
    drwxr-xr-x  5 debian debian   4096 Sep 10  2018 examples
    -rw-r--r--  1 debian debian    962 Sep 10  2018 .gitignore
    drwxr-xr-x  3 debian debian   4096 Sep 10  2018 include
    -rw-r--r--  1 debian debian  15752 Sep 10  2018 INSTALL
    -rwxr-xr-x  1 debian debian  15155 Sep 10  2018 install-sh
    -rw-r--r--  1 debian debian    938 Sep 10  2018 libcoap-1.la
    -rw-r--r--  1 debian debian   2393 Sep 10  2018 libcoap-1.map
    -rw-r--r--  1 debian debian    240 Sep 10  2018 libcoap-1.pc
    -rw-r--r--  1 debian debian    260 Sep 10  2018 libcoap-1.pc.in
    -rw-r--r--  1 debian debian   2023 Sep 10  2018 libcoap-1.sym
    drwxr-xr-x  2 debian debian   4096 Sep 10  2018 .libs
    -rwxr-xr-x  1 debian debian 339437 Sep 10  2018 libtool
    -rw-r--r--  1 debian debian   1327 Sep 10  2018 LICENSE.BSD
    -rw-r--r--  1 debian debian 324412 Sep 10  2018 ltmain.sh
    drwxr-xr-x  2 debian debian   4096 Sep 10  2018 m4
    -rw-r--r--  1 debian debian  56776 Sep 10  2018 Makefile
    -rw-r--r--  1 debian debian   7053 Sep 10  2018 Makefile.am
    -rw-r--r--  1 debian debian  64526 Sep 10  2018 Makefile.in
    -rw-r--r--  1 debian debian    302 Sep 10  2018 Makefile.libcoap
    -rwxr-xr-x  1 debian debian   6872 Sep 10  2018 missing
    -rw-r--r--  1 debian debian      0 Sep 10  2018 NEWS
    lrwxrwxrwx  1 debian debian      9 Sep 10  2018 README -> README.md
    -rw-r--r--  1 debian debian   1256 Sep 10  2018 README.md
    drwxr-xr-x  2 debian debian   4096 Sep 10  2018 scripts
    drwxr-xr-x  4 debian debian   4096 Sep 10  2018 src
    -rw-r--r--  1 debian debian     28 Sep 10  2018 stamp-h1
    drwxr-xr-x  3 debian debian   4096 Sep 10  2018 tests
    -rw-r--r--  1 debian debian   2185 Sep 10  2018 TODO
    -rw-r--r--  1 debian debian    469 Sep 10  2018 .travis.yml
    ```

2.  Make sure that the following 2 files are under examples directory

    ```
    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo/examples$ ls -al coap_list.*
    -rw-r--r-- 1 debian debian 979 Sep 10  2018 coap_list.c
    -rw-r--r-- 1 debian debian 841 Sep 10  2018 coap_list.h
    ```

3.  Transfer all files under ttoad folder to your border router using WinSCP.
    Location should be under
    ```/home/debian/ot-br-posix/third_party/libcoap/repo/ttoad```

4.  Change to directory ```/home/debian/ot-br-posix/third_party/libcoap/repo/ttoad```

    ```
    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo$ cd ttoad
    ```

5.  Run ```make```

    ```
    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo/ttoad$ make
    ```

6.  You should see ```ttoad-server``` has been created

    ```
    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo/ttoad$ ls -al
    total 180
    drwxr-xr-x  3 debian debian  4096 Mar  5 09:47 .
    drwxr-xr-x 12 debian debian  4096 Mar  5 08:16 ..
    -rw-r--r--  1 debian debian  7180 Mar  5 09:47 coap_list.o
    drwxr-xr-x  2 debian debian  4096 Mar  5 09:47 .libs
    -rw-rw-rw-  1 debian debian  1000 Mar  5 09:41 Makefile
    -rw-r--r--  1 debian debian  6550 Dec 11  2018 README_TTOAD.md
    -rwxr-xr-x  1 debian debian  7908 Mar  5 09:47 ttoad-server
    -rw-rw-rw-  1 debian debian 48441 Mar  5 08:55 ttoad-server.c
    -rw-r--r--  1 debian debian 90356 Mar  5 09:47 ttoad-server.o

    ```

## Running the program

1.  Run the OAD sever

    ```
    debian@beaglebone:~/ot-br-posix/third_party/libcoap/repo/ttoad$ ./ttoad-server
    ```

2.  Make the doorlock device commissioned and joined to the network by pressing
    the right button. Use the EUI64 and PSKd as shown for your doorlock device.
    Commissioning can be done in two ways. One way to do that is to use Android
    phone. The other way is to use command from border router. From border
    router, you can issue this command with your settings. The below example is
    using my setting.

    ```
       ~/ot-br-posix/tests/meshcop/otbr-commissioner --network-name OpenThreadEom --xpanid 1111111122222222 --agent-passphrase 123456 --joiner-eui64 00124b0018a74571 --joiner-passphrase J01NU5 --agent-addr 192.168.1.145 --agent-port 49191 --comm-envelope-timeout 200 --commission-device
    ```
  
  You can also commission the device by performing the following commands on the
  border router, then pressing BTN-2 on the launchpad.

    ```
    debian@beaglebone:~$ wpanctl commissioner start
    Commissioner started

    debian@beaglebone:~$ wpanctl commissioner joiner-add 00124b001ca1782a 120 DRRLCK1
    Added Joiner 00:12:4B:00:1C:A1:78:2A, timeout:120, PSKd:"DRRLCK1"
    ```

3. You should see download start once the doorlock device has joined.

    ```
    v:1 t:CON c:PUT i:cb16 {} [ ]
    18:58:16 [OAD] block    0 : 131 bytes sent
    18:58:20 [OAD] block    0 : 131 bytes sent
    18:58:27 [OAD] block    1 : 131 bytes sent
    ```

## Authors

If you have any question, please post to our [E2E forum](http://e2e.ti.com).

## License

This code is licensed under a BSD-3-Clause License and is based on an example
project of libcoap embedded in
[OpenThread Border Router](https://github.com/openthread/ot-br-posix).
