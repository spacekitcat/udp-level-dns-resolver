cmake_minimum_required(VERSION 3.10)

# set the project name
project(udp-level-dns-resolver C)

# add the executable
add_executable(udp-level-dns-resolver main.c hexdump.c)
target_link_libraries(udp-level-dns-resolver resolv)
