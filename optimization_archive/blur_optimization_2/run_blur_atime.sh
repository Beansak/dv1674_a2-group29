#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")

for img in "${images[@]}"; do
    output="output_${img}"
    echo "Running blur on $img (4 times)..."
    
    times=()
    for run in {1..4}; do
        echo "Run $run:"
        # Capture time and extract seconds
        time_output=$(/usr/bin/time -f "%e" ./blur 15 "data/$img" "$output" 2>&1 | tail -1)
        times+=($time_output)
        echo "Time: ${time_output}s"
    done
    
    # Calculate average
    total=0
    for t in "${times[@]}"; do
        total=$(echo "$total + $t" | bc -l)
    done
    average=$(echo "scale=3; $total / 4" | bc -l)
    
    echo "Average time: ${average}s"
    echo "-----------------------------------------"
done

# Run valgrind only after all timing is done, for all images

echo "Running valgrind (callgrind) on im1"
valgrind --tool=callgrind ./blur 15 "data/im1.ppm" "output_im1.ppm"
echo "-----------------------------------------"