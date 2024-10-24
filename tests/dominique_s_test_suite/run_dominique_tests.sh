ERROR=0
function error_handling() {
    ERROR=1
}

"./dominic_test" -in ./tests_in/test01.bmp -out ./tests_out/test01.bmp || error_handling
"./dominic_test" -in ./tests_in/test02.bmp -out ./tests_out/test02.bmp || error_handling
"./dominic_test" -in ./tests_in/test03.bmp -out ./tests_out/test03.bmp || error_handling
"./dominic_test" -in ./tests_in/test04.bmp -out ./tests_out/test04.bmp || error_handling
"./dominic_test" -in ./tests_in/test01.gif -out ./tests_out/test01.gif || error_handling
"./dominic_test" -in ./tests_in/test01.jpg -out ./tests_out/test01.jpg || error_handling
"./dominic_test" -in ./tests_in/test02.jpg -out ./tests_out/test02.jpg || error_handling
"./dominic_test" -in ./tests_in/test01.png -out ./tests_out/test01.png || error_handling
"./dominic_test" -in ./tests_in/test01.tif -out ./tests_out/test01.tif || error_handling
"./dominic_test" -in ./tests_in/test02.tif -out ./tests_out/test02.tif || error_handling
"./dominic_test" -in ./tests_in/test01.jp2 -out ./tests_out/test01.jp2 || error_handling
"./dominic_test" -in ./tests_in/test01.exr -out ./tests_out/test01.exr || error_handling
"./dominic_test" -in ./tests_in/test01.hdr -out ./tests_out/test01.hdr || error_handling

exit $ERROR