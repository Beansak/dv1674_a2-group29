#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1


images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")
# images=("im4.ppm")

# Add thread counts to test
thread_counts=(1 2 4 8 16 32)

# Remove output files after each run
for threads in "${thread_counts[@]}"; do
    echo "Testing with $threads threads..."
    for img in "${images[@]}"; do
        output="output_${img}_threads_${threads}"
        echo "Running blur_par on $img with $threads threads..."
        /usr/bin/time -v ./blur_par 15 "data/$img" "$output" "$threads" 2>&1 | grep -E "Percent of CPU this job got|Elapsed \(wall clock\)|Maximum resident set size|Major \(requiring I/O\) page faults|Minor \(reclaiming a frame\) page faults|Voluntary context switches|Involuntary context switches|File system inputs|File system outputs|Page size|Exit status"
        rm -f "$output" # Remove the output file
        echo "-----------------------------------------"
    done

    echo "Running valgrind (callgrind) on im1 with $threads threads"
    valgrind --tool=callgrind ./blur_par 15 "data/im1.ppm" "output_im1_threads_${threads}.ppm" "$threads"
    rm -f "output_im1_threads_${threads}.ppm" # Remove the Valgrind output file
    echo "-----------------------------------------"
done