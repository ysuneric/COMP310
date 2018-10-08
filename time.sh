#!/bin/sh

for i in {1..100}
do
./tshell << 'EOF'
who
exit
EOF
done