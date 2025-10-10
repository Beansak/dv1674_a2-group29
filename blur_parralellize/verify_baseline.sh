#!/bin/bash
# test :)
echo "NOTE: this script relies on the binaries blur_par and baseline outputs in data_o to exist"

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

status=0
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
reset=$(tput sgr0)

errors_found=0
warnings_found=0

for image in im1 im2 im3 im4
    do
        ./blur_par 15 "data/$image.ppm" "./data_o/blur_${image}_test.ppm" "4"



        if ! cmp -s "./data_o/blur_${image}.ppm" "./data_o/blur_${image}_test.ppm"
        then
            echo "${red}Error: Incongruent output data detected when blurring image $image.ppm ${reset}"
            errors_found=1
            status=1
        else
            echo "${green}Output matches for $image.ppm${reset}"
        fi

        rm "./data_o/blur_${image}_test.ppm"

    done

# Final output based on results
if [ $errors_found -eq 1 ]; then
    echo "${red}Errors found during the tests.${reset}"
    status=1
elif [ $warnings_found -eq 1 ]; then
    echo "${yellow}Warnings were found, but no major errors.${reset}"
else
    echo "${green}Success: All tests passed successfully.${reset}"
    echo "${green}"
    echo "        _    _"
    echo "      ( o)--( o)"
    echo "     /    ..    \\"
    echo "    (  (\\____/)  )"
    echo "     \\   '--'   /"
    echo "       '--'--'"
    echo "   Happy Frog!"
    echo "${reset}"
fi

exit $status