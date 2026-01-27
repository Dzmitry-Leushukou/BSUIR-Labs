#!/bin/bash
COUNT=5
OUTPUT="report.txt"
{
    
    echo "ТОП-$COUNT процессов по памяти:"
    echo "-----------------------------"
    echo " USER	       PROC	    PID	  %CPU %MEM CMD"
    ps -eo user,comm,pid,%cpu,%mem,cmd --sort=-%mem | head -$((COUNT+1)) | tail -$COUNT
    
    echo ""
    echo "ТОП-$COUNT процессов по CPU:"
    echo "--------------------------"
    echo " USER	       PROC	    PID	  %CPU %MEM CMD"
    ps -eo user,comm,pid,%cpu,%mem,cmd --sort=-%cpu | head -$((COUNT+1)) | tail -$COUNT

    echo ""
    echo "ТОП-$COUNT процессов по CPU и по памяти:"
    echo "--------------------------"
    echo " USER	       PROC	    PID	  %CPU %MEM CMD"
    ps -eo user,comm,pid,%cpu,%mem,cmd --sort=-%cpu,-%mem | head -$((COUNT+1)) | tail -$COUNT


    echo ""
    echo "ТОП-$COUNT процессов по памяти и по CPU:"
    echo "--------------------------"
    echo " USER	       PROC	    PID	  %CPU %MEM CMD"
    ps -eo user,comm,pid,%cpu,%mem,cmd --sort=-%mem,-%cpu | head -$((COUNT+1)) | tail -$COUNT

} > $OUTPUT

