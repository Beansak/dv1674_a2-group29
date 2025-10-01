#!/bin/bash
# List of input data files

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

datas=("128.data" "256.data" "512.data" "1024.data")

for data in "${datas[@]}"; do
    output="output_${data}.txt"
    echo "Running pearson on $data..."
    /usr/bin/time -v ./pearson "data/$data" "$output" 2>&1 | grep -E "Percent of CPU this job got|Elapsed \(wall clock\)|Maximum resident set size|Major \(requiring I/O\) page faults|Minor \(reclaiming a frame\) page faults|Voluntary context switches|Involuntary context switches|File system inputs|File system outputs|Page size|Exit status"
    echo "-----------------------------------------"
done