
"Debug\dominic_test.exe" -in ./tests_in/test01.bmp -out ./tests_out/test01.bmp
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test02.bmp -out ./tests_out/test02.bmp
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test03.bmp -out ./tests_out/test03.bmp
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test04.bmp -out ./tests_out/test04.bmp
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.gif -out ./tests_out/test01.gif
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.jpg -out ./tests_out/test01.jpg
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test02.jpg -out ./tests_out/test02.jpg
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.png -out ./tests_out/test01.png
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.tif -out ./tests_out/test01.tif
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test02.tif -out ./tests_out/test02.tif
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.jp2 -out ./tests_out/test01.jp2
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.exr -out ./tests_out/test01.exr
CALL :err_handle
"Debug\dominic_test.exe" -in ./tests_in/test01.hdr -out ./tests_out/test01.hdr

echo "Finished with error code : "
echo %err_tracker%
EXIT %err_tracker%

:err_handle
if %ERRORLEVEL% NEQ 0 set /A err_tracker=1

EXIT /B 0