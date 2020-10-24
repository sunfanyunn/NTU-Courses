#!/usr/bin/env bash
ips=$(last -i | head -n -2 | awk '{print $3}') 
for ip in $ips
do
    if [ "$ip" == "0.0.0.0" ]; then continue; fi
    geoiplookup $ip | awk '{print $4, $5}'
done | sort -n | uniq -c | sort -n

