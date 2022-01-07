# udp-level-dns-resolver

The goal of this Kata is to implement a custom version of `nslookup` at the
socket level. `nslookup` is a DNS resolver program, it queries the DNS servers for records related a specified fully qualified domain name.

The aim of this Kata is to implement a program that can query the local DNS server for A records about the specified full qualified domain name. The motivation for doing this Kata is to learn more about how DNS resolvers work. It won't have a fraction of the features of the real nslookup program, it's designed to be used in real life scenarios.

This implementation operates at UDP level, it sends a DNS query to the local DNS server (or forwarder, as is more likely the case) directly via UDP on port 53. It then listens for the response UDP packet from the DNS resolver. The only functionality it lifts from `resolv.h` is the ability to determine the IP address for the local DNS forwarder. This should match the
output from `scutil --dns` or `cat /etc/resolv.conf`.

## building

```sh
git clone git@github.com:spacekitcat/udp-level-dns-resolver.git
cd udp-level-dns-resolver
mkdir build
cd build
cmake ../
make
./udp-level-dns-resolver google.com
```

Output:
```sh
DNS query payload:

DNS query payload:

99 E2 01 00 00 01 00 00   00 00 00 00 06 67 6F 6F   .............goo
67 6C 65 03 63 6F 6D 00   00 01 00 01


Recieved 44 bytes.

DNS response payload:

99 E2 81 80 00 01 00 01   00 00 00 00 06 67 6F 6F   .............goo
67 6C 65 03 63 6F 6D 00   00 01 00 01 C0 0C 00 01   gle.com.........
00 01 00 00 00 60 00 04   8E FA BB EE

```