cmake_minimum_required(VERSION 3.10)

# set the project name
project(nslookup-kata C)

# add the executable
add_executable(nslookup-kata main.c hexdump.c)
target_link_libraries(nslookup-kata resolv)
