#!/bin/sh

for ip in $(last -a -i \
            | awk '{print $10}' \
            | sed '/^ *$/d' \
            | grep -v 0.0.0.0 \
            | grep "[0-9]\+\.[0-9]\+.[0-9]\+\.[0-9]\+"); do \
    geoiplookup "$ip" 2> /dev/null \
        | cut -f2 -d':' \
        | grep -v "not found"; done | sort | uniq -c | sort -n
