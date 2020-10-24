#!/bin/bash

localtime=$(zdump /etc/localtime | awk '{print $5}')
kern_uptime=$(cat /proc/uptime | awk '{print $1}')
uptime_d=$(printf "%.f" $(echo "$kern_uptime / 60 / 60 / 24" | bc))
uptime_h=$(printf "%.f" $(echo "$kern_uptime / 60 / 60 % 24" | bc))
uptime_m=$(printf "%02.f" $(echo "$kern_uptime / 60 % 60" | bc))
uptime_m_no_pad=$(printf "%.f" $(echo "$kern_uptime / 60 % 60" | bc))
user_cnt=$(who | wc -l)
load_avg=$(cat /proc/loadavg | awk '{print $1 ", " $2 ", " $3}')

if [ "$uptime_d" -gt 0 ]; then
    if [ "$uptime_d" = 1 ]; then
        day="$uptime_d day, "
    else
        day="$uptime_d days, "
    fi
fi


user="$user_cnt user"
if [ "$user_cnt" -gt 1 ]; then
    user="$user"s
fi

if [ "$uptime_h" -gt 0 ]; then
    echo -n " $localtime up $day$uptime_h:$uptime_m, "
else
    echo -n " $localtime up $day$uptime_m_no_pad min, "
fi
echo    " $user,  load average: $load_avg"

