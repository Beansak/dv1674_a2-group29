#!/bin/bash

echo "NOTE: this script relies on the binaries blur and baseline outputs in data_o to exist"

status=0
red=$(tput setaf 1)
reset=$(tput sgr0)

for image in im1 im2 im3 im4
    do
        ./blur 15 "data/$image.ppm" "./data_o/blur_${image}_test.ppm"

        if ! cmp -s "./data_o/blur_${image}.ppm" "./data_o/blur_${image}_test.ppm"
        then
            echo "${red}Error: Incongruent output data detected when blurring image $image.ppm ${reset}"
            status=1
        fi

        rm "./data_o/blur_${image}_test.ppm"
    done
done

exit $status