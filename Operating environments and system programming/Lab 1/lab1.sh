#!/bin/bash

# Simple screensaver with readable ASCII digits

# Random number generator
init_generator() {
    RANDOM_SEED=$(date +%s)
}

my_random() {
    # Multiplicative algorithm
    RANDOM_SEED=$(( (RANDOM_SEED * 1103515245 + 12345) % 2147483648 ))
    echo $(( RANDOM_SEED % $1 ))
}

# DIGITS 7x5 (very readable)
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

# Main function
main() {
    # Initialize generator
    init_generator
    
    # Get terminal dimensions
    local width=$(tput cols)
    local height=$(tput lines)
    
    # Settings
    local update_interval=1
    local move_interval=10
    local refresh_interval=10
    
    # Initial position
    local x=$(( $(my_random $((width - 50))) ))
    local y=$(( $(my_random $((height - 7))) ))
    [ $x -lt 1 ] && x=1
    [ $y -lt 1 ] && y=1
    
    # Timers
    local last_move=$(date +%s)
    local last_refresh=$(date +%s)
    
    # Terminal setup
    printf "\033[?25l"
    stty -echo
    
    trap 'printf "\033[?25h\n"; stty echo; clear; exit' INT TERM
    
    while true; do
        clear
        
        # Update generator every 10 seconds
        local now=$(date +%s)
        if [ $((now - last_refresh)) -ge $refresh_interval ]; then
            RANDOM_SEED=$(date +%s)
            last_refresh=$now
            echo "Generator updated" >&2
        fi
        
        # Move clock every 10 seconds
        if [ $((now - last_move)) -ge $move_interval ]; then
            x=$(( $(my_random $((width - 50))) ))
            y=$(( $(my_random $((height - 7))) ))
            [ $x -lt 1 ] && x=1
            [ $y -lt 1 ] && y=1
            last_move=$now
        fi
        
        # Get current time
        local time_str=$(date +"%H:%M:%S")
        local curr_x=$x
        
        # Draw each digit
        for ((i=0; i<${#time_str}; i++)); do
            local char="${time_str:$i:1}"
            
            # Split digit into lines
            IFS=$'\n' read -d '' -ra lines <<< "${DIGIT[$char]}"
            
            # Draw each line
            for ((j=0; j<5; j++)); do
                printf "\033[%d;%dH%s" $((y + j)) $curr_x "${lines[$j]}"
            done
            
            # Move position
            if [ "$char" = ":" ]; then
                curr_x=$((curr_x + 4))
            else
                curr_x=$((curr_x + 8))
            fi
        done
        
        # Information line
        local info="LARGE ASCII CLOCK | Move: 10s | Generator update: 10s | Exit: press any key"
        local info_x=$(( (width - ${#info}) / 2 ))
        [ $info_x -lt 0 ] && info_x=0
        
        printf "\033[%d;%dH%s" $((height - 1)) $info_x "$info"
        
        # Generator status
        local status="Seed: $RANDOM_SEED | Next update in: $((last_refresh + refresh_interval - now))s"
        local status_x=$(( (width - ${#status}) / 2 ))
        printf "\033[%d;%dH%s" $((height - 2)) $status_x "$status"
        
        # Check for key press
        if read -t $update_interval -n 1; then
            break
        fi
    done
    
    # Restore terminal
    printf "\033[?25h"
    stty echo
    clear
    echo "Screensaver finished"
}

# Start
main
