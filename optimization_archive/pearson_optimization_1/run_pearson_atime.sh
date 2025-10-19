#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

datas=("128.data" "256.data" "512.data" "1024.data")

for data in "${datas[@]}"; do
    output="output_${data}.txt"
    echo "Running pearson on $data (4 times)..."
    
    times=()
    cpu_usages=()
    for run in {1..4}; do
        echo "Run $run:"
        # Capture time, CPU usage, and other metrics
        time_output=$(/usr/bin/time -f "%e %P %M" ./pearson "data/$data" "$output" 2>&1 | tail -1)
        
        # Parse the output: time CPU% memory
        read elapsed_time cpu_percent max_memory <<< "$time_output"
        
        times+=($elapsed_time)
        cpu_usages+=($cpu_percent)
        
        echo "Time: ${elapsed_time}s, CPU: ${cpu_percent}, Max Memory: ${max_memory}KB"
    done
    
    # Calculate averages
    total_time=0
    total_cpu=0
    for i in "${!times[@]}"; do
        total_time=$(echo "$total_time + ${times[$i]}" | bc -l)
        # Remove % sign and convert to number
        cpu_num=$(echo "${cpu_usages[$i]}" | sed 's/%//')
        total_cpu=$(echo "$total_cpu + $cpu_num" | bc -l)
    done
    average_time=$(echo "scale=3; $total_time / 4" | bc -l)
    average_cpu=$(echo "scale=1; $total_cpu / 4" | bc -l)
    
    echo "Average time: ${average_time}s"
    echo "Average CPU utilization: ${average_cpu}%"
    echo "-----------------------------------------"
done
