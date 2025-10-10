#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")

for img in "${images[@]}"; do
    output="output_${img}"
    echo "Running blur_par on $img..."
    /usr/bin/time -v ./blur_par 15 "data/$img" "$output" "8" 2>&1 | grep -E "Percent of CPU this job got|Elapsed \(wall clock\)|Maximum resident set size|Major \(requiring I/O\) page faults|Minor \(reclaiming a frame\) page faults|Voluntary context switches|Involuntary context switches|File system inputs|File system outputs|Page size|Exit status"
    echo "-----------------------------------------"
done

# Run valgrind only after all timing is done, for all images
for img in "${images[@]}"; do
    output="output_${img}"
    echo "Running valgrind on $img..."
    MALLOC_MMAP_THRESHOLD_=0 valgrind --tool=callgrind ./blur_par 15 "data/$img" "$output" "8"
    echo "-----------------------------------------"
done