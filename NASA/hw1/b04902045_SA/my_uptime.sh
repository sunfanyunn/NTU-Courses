#!/usr/bin/env bash
# uptime.sh
# get uptime from /proc/uptime
#Get localtime
time=$(date)
#IFS=' ' 
read -r -a array <<< "$time"

echo -n " ${array[3]} up "
uptime=$(cat /proc/uptime)
uptime=${uptime%%.*}

minutes=$(( uptime/60%60 ))
hours=$(( uptime/60/60%24 ))
days=$(( uptime/60/60/24 ))

if (($days != 0)); then
    if (($days == 1)); then echo -n "1 day, ";
    else echo -n "$days days, ";
    fi
else
    #days==0
    if (($hours != 0)); then printf "%2d:%02d, " "$hours" "$minutes"
    else 
        #days==0 and hours==0
        if (($minutes == 0)); then printf "0 min"
        else printf "%02d min, " "$minutes"
        fi
    fi
fi

users=$(who | wc -l) 
if (($users == 1)); then
    printf "%2d user, " $users
else 
    printf "%2d users, " $users
fi

read F S T g gg <<< $(cat /proc/loadavg)
printf " load average: %.2f, %.2f, %.2f\n" $F $S $T

