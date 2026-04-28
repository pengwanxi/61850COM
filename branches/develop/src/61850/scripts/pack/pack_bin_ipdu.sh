#!/usr/bin/expect -f

set name [lindex $argv 0]
set pack [lindex $argv 1]

spawn ./all_in_one ipdu.sh $pack
expect "Name with Identify Code: "
send "$name\r\n"
expect eof
