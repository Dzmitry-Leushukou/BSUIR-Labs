#!/bin/bash

init_generator() {
    RANDOM_SEED=$(date +%s)
}

my_random() {

    RANDOM_SEED=$(( (RANDOM_SEED * 1103515245 + 12345) % 2147483648 ))
    echo $(( RANDOM_SEED % $1 ))
}


declare -A DIGIT

DIGIT[0]="  ###  
 #   # 
 #   # 
 #   # 
  ###  "

DIGIT[1]="   #   
  ##   
   #   
   #   
 ##### "

DIGIT[2]="  ###  
 #   # 
    #  
   #   
 ##### "

DIGIT[3]="  ###  
 #   # 
   ##  
 #   # 
  ###  "

DIGIT[4]=" #   # 
 #   # 
 ##### 
     # 
     # "

DIGIT[5]=" ##### 
 #     
 ##### 
     # 
 ##### "

DIGIT[6]="  ###  
 #     
 ####  
 #   # 
  ###  "

DIGIT[7]=" ##### 
     # 
    #  
   #   
   #   "

DIGIT[8]="  ###  
 #   # 
  ###  
 #   # 
  ###  "

DIGIT[9]="  ###  
 #   # 
  #### 
     # 
  ###  "

DIGIT[":"]="       
   #   
       
   #   
       "


main() {
    init_generator
    

    local width=$(tput cols)
    local height=$(tput lines)
    

    local update_interval=1
    local move_interval=10
    local refresh_interval=10
    
    local x=$(( $(my_random $((width - 50))) ))
    local y=$(( $(my_random $((height - 7))) ))
    [ $x -lt 1 ] && x=1
    [ $y -lt 1 ] && y=1
    
    local last_move=$(date +%s)
    local last_refresh=$(date +%s)
    
    printf "\033[?25l"
    stty -echo
    
    trap 'printf "\033[?25h\n"; stty echo; clear; exit' INT TERM
    
    while true; do
        clear
        
        local now=$(date +%s)
        if [ $((now - last_refresh)) -ge $refresh_interval ]; then
            RANDOM_SEED=$(date +%s)
            last_refresh=$now
            echo "Generator updated" >&2
        fi
        

        if [ $((now - last_move)) -ge $move_interval ]; then
            x=$(( $(my_random $((width - 50))) ))
            y=$(( $(my_random $((height - 7))) ))
            [ $x -lt 1 ] && x=1
            [ $y -lt 1 ] && y=1
            last_move=$now
        fi
        
        local time_str=$(date +"%H:%M:%S")
        local curr_x=$x
        
        for ((i=0; i<${#time_str}; i++)); do
            local char="${time_str:$i:1}"
            
            IFS=$'\n' read -d '' -ra lines <<< "${DIGIT[$char]}"
            
            for ((j=0; j<5; j++)); do
                printf "\033[%d;%dH%s" $((y + j)) $curr_x "${lines[$j]}"
            done
            
            if [ "$char" = ":" ]; then
                curr_x=$((curr_x + 4))
            else
                curr_x=$((curr_x + 8))
            fi
        done
        
        local info="LARGE ASCII CLOCK | Move: 10s | Generator update: 10s | Exit: press any key"
        local info_x=$(( (width - ${#info}) / 2 ))
        [ $info_x -lt 0 ] && info_x=0
        
        printf "\033[%d;%dH%s" $((height - 1)) $info_x "$info"
        
        local status="Seed: $RANDOM_SEED | Next update in: $((last_refresh + refresh_interval - now))s"
        local status_x=$(( (width - ${#status}) / 2 ))
        printf "\033[%d;%dH%s" $((height - 2)) $status_x "$status"
        
        if read -t $update_interval -n 1; then
            break
        fi
    done
    

    printf "\033[?25h"
    stty echo
    clear
    echo "Screensaver finished"
}


main
