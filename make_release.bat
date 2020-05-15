@echo off

echo ***********************************************************************     
echo ************************* donot suppot record *************************
echo ***********************************************************************

set version=2.0.115
set recordv=R

make.exe clean

make.exe -f makefile DEBUG_SYMBOLS=0 APP_BUILD_TIME="%date:~,10% %time:~,8%" APP_VERSION=%version%

if %errorlevel% == 0 (

	rd /s /q output

	copy build\GmAppMain.bin build\GmAppMain.ex

	perl filecreat.pl %version%
	
	if %errorlevel% == 0 (
		
		echo ***********************************************************************     
		echo ************************* support record ******************************
		echo ***********************************************************************

		make.exe clean

		make.exe -f makefile DEBUG_SYMBOLS=0 APP_BUILD_TIME="%date:~,10% %time:~,8%" APP_VERSION=%version%%recordv% SUPPORT_RECORD=1
		
		if %errorlevel% == 0 (
		
			rd /s /q output

			copy build\GmAppMain.bin build\GmAppMain.ex

			perl filecreat.pl %version%%recordv%
			
		)		
	)
)


