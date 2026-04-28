#!/usr/bin/expect -f

set name [lindex $argv 0]
set pack [lindex $argv 1]

# set name PXR61850
# set pack PXR61850

spawn ./all_in_one bin.sh $pack
expect "Name with Identify Code: "
send "$name\r\n"
expect eof
