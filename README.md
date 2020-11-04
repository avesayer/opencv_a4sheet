# opencv_a4sheet
This is a source code for a program that detect a4 sheet on an image

Usage: opencv_test [params]

        -h, --help
                print this message
        -p, --path
                path to input image
        -s, --show (value:false)
                flag for showing result images

By default this program parses all jpg images inside executable folder and prints out the result.
Example:
	openct_test -s
	opencv_test -path="sample1.jpg"
	opencv_test -path="sample2.jpg" -s