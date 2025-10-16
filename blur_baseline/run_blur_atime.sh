#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")

for img in "${images[@]}"; do
    output="output_${img}"
    echo "Running blur on $img (4 times)..."
    
    times=()
    cpu_usages=()
    for run in {1..4}; do
        echo "Run $run:"
        # Capture time, CPU usage, and other metrics
        time_output=$(/usr/bin/time -f "%e %P %M" ./blur 15 "data/$img" "$output" 2>&1 | tail -1)
        
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

# Run valgrind only after all timing is done, for all images

echo "Running valgrind (callgrind) on im1"
valgrind --tool=callgrind ./blur 15 "data/im1.ppm" "output_im1.ppm"
echo "-----------------------------------------"