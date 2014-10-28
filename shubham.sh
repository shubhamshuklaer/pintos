#!/bin/bash
tar -czvf ../pintos.tar.gz --exclude='.git' ../pintos
expect - <<'END_EXPECT'
    set timeout -1
    spawn scp ../pintos.tar.gz group15@172.16.24.253:/home/CS342/2014/STUD/group15/master/
    expect "group15@172.16.24.253's password:"
    send "faadu_legends\r"
    expect "%100"
    expect eof 
END_EXPECT

expect - <<'END_EXPECT'
    set timeout -1
    set prompt "(%|#|\\$) $" 
    spawn ssh group15@172.16.24.253
    expect "group15@172.16.24.253's password:"
    send "faadu_legends\r"
    expect -re $prompt
    send "cd master\r"
    expect -re $prompt
    send "rm -rf pintos\r"
    expect -re $prompt
    send "tar -xzvf pintos.tar.gz\r"
    expect -re $prompt
END_EXPECT
