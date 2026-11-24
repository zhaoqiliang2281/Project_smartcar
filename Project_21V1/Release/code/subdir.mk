################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/Algorithmic_Filtering.c \
../code/Angle_Encoder.c \
../code/Common_peripheral.c \
../code/Encoder.c \
../code/GPS.c \
../code/HotRC.c \
../code/IMU.c \
../code/Imag.c \
../code/Model.c \
../code/Motor.c \
../code/NRF24L01.c \
../code/PID.c \
../code/PID_Steer.c \
../code/Project2.c \
../code/Serial.c \
../code/Subject_Two.c \
../code/Voice.c \
../code/WIFI_UART.c \
../code/by9301.c \
../code/flash.c \
../code/menu.c \
../code/miscellaneous.c \
../code/navigation.c \
../code/path_diagram.c \
../code/quaternion.c \
../code/subject.c \
../code/user_display_gps.c \
../code/zf_device_dot_matrix_screen.c \
../code/zf_device_tld7002.c 

COMPILED_SRCS += \
code/Algorithmic_Filtering.src \
code/Angle_Encoder.src \
code/Common_peripheral.src \
code/Encoder.src \
code/GPS.src \
code/HotRC.src \
code/IMU.src \
code/Imag.src \
code/Model.src \
code/Motor.src \
code/NRF24L01.src \
code/PID.src \
code/PID_Steer.src \
code/Project2.src \
code/Serial.src \
code/Subject_Two.src \
code/Voice.src \
code/WIFI_UART.src \
code/by9301.src \
code/flash.src \
code/menu.src \
code/miscellaneous.src \
code/navigation.src \
code/path_diagram.src \
code/quaternion.src \
code/subject.src \
code/user_display_gps.src \
code/zf_device_dot_matrix_screen.src \
code/zf_device_tld7002.src 

C_DEPS += \
code/Algorithmic_Filtering.d \
code/Angle_Encoder.d \
code/Common_peripheral.d \
code/Encoder.d \
code/GPS.d \
code/HotRC.d \
code/IMU.d \
code/Imag.d \
code/Model.d \
code/Motor.d \
code/NRF24L01.d \
code/PID.d \
code/PID_Steer.d \
code/Project2.d \
code/Serial.d \
code/Subject_Two.d \
code/Voice.d \
code/WIFI_UART.d \
code/by9301.d \
code/flash.d \
code/menu.d \
code/miscellaneous.d \
code/navigation.d \
code/path_diagram.d \
code/quaternion.d \
code/subject.d \
code/user_display_gps.d \
code/zf_device_dot_matrix_screen.d \
code/zf_device_tld7002.d 

OBJS += \
code/Algorithmic_Filtering.o \
code/Angle_Encoder.o \
code/Common_peripheral.o \
code/Encoder.o \
code/GPS.o \
code/HotRC.o \
code/IMU.o \
code/Imag.o \
code/Model.o \
code/Motor.o \
code/NRF24L01.o \
code/PID.o \
code/PID_Steer.o \
code/Project2.o \
code/Serial.o \
code/Subject_Two.o \
code/Voice.o \
code/WIFI_UART.o \
code/by9301.o \
code/flash.o \
code/menu.o \
code/miscellaneous.o \
code/navigation.o \
code/path_diagram.o \
code/quaternion.o \
code/subject.o \
code/user_display_gps.o \
code/zf_device_dot_matrix_screen.o \
code/zf_device_tld7002.o 


# Each subdirectory must supply rules for building sources it contributes
code/Algorithmic_Filtering.src: ../code/Algorithmic_Filtering.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Algorithmic_Filtering.o: code/Algorithmic_Filtering.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Angle_Encoder.src: ../code/Angle_Encoder.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Angle_Encoder.o: code/Angle_Encoder.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Common_peripheral.src: ../code/Common_peripheral.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Common_peripheral.o: code/Common_peripheral.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Encoder.src: ../code/Encoder.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Encoder.o: code/Encoder.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/GPS.src: ../code/GPS.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/GPS.o: code/GPS.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/HotRC.src: ../code/HotRC.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/HotRC.o: code/HotRC.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/IMU.src: ../code/IMU.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/IMU.o: code/IMU.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Imag.src: ../code/Imag.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Imag.o: code/Imag.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Model.src: ../code/Model.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Model.o: code/Model.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Motor.src: ../code/Motor.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Motor.o: code/Motor.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/NRF24L01.src: ../code/NRF24L01.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/NRF24L01.o: code/NRF24L01.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID.src: ../code/PID.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID.o: code/PID.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID_Steer.src: ../code/PID_Steer.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID_Steer.o: code/PID_Steer.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Project2.src: ../code/Project2.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Project2.o: code/Project2.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Serial.src: ../code/Serial.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Serial.o: code/Serial.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Subject_Two.src: ../code/Subject_Two.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Subject_Two.o: code/Subject_Two.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Voice.src: ../code/Voice.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Voice.o: code/Voice.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/WIFI_UART.src: ../code/WIFI_UART.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/WIFI_UART.o: code/WIFI_UART.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/by9301.src: ../code/by9301.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/by9301.o: code/by9301.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/flash.src: ../code/flash.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/flash.o: code/flash.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/menu.src: ../code/menu.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/menu.o: code/menu.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/miscellaneous.src: ../code/miscellaneous.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/miscellaneous.o: code/miscellaneous.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/navigation.src: ../code/navigation.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/navigation.o: code/navigation.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/path_diagram.src: ../code/path_diagram.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/path_diagram.o: code/path_diagram.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/quaternion.src: ../code/quaternion.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/quaternion.o: code/quaternion.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/subject.src: ../code/subject.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/subject.o: code/subject.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/user_display_gps.src: ../code/user_display_gps.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/user_display_gps.o: code/user_display_gps.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/zf_device_dot_matrix_screen.src: ../code/zf_device_dot_matrix_screen.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/zf_device_dot_matrix_screen.o: code/zf_device_dot_matrix_screen.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/zf_device_tld7002.src: ../code/zf_device_tld7002.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/zf_device_tld7002.o: code/zf_device_tld7002.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) code/Algorithmic_Filtering.d code/Algorithmic_Filtering.o code/Algorithmic_Filtering.src code/Angle_Encoder.d code/Angle_Encoder.o code/Angle_Encoder.src code/Common_peripheral.d code/Common_peripheral.o code/Common_peripheral.src code/Encoder.d code/Encoder.o code/Encoder.src code/GPS.d code/GPS.o code/GPS.src code/HotRC.d code/HotRC.o code/HotRC.src code/IMU.d code/IMU.o code/IMU.src code/Imag.d code/Imag.o code/Imag.src code/Model.d code/Model.o code/Model.src code/Motor.d code/Motor.o code/Motor.src code/NRF24L01.d code/NRF24L01.o code/NRF24L01.src code/PID.d code/PID.o code/PID.src code/PID_Steer.d code/PID_Steer.o code/PID_Steer.src code/Project2.d code/Project2.o code/Project2.src code/Serial.d code/Serial.o code/Serial.src code/Subject_Two.d code/Subject_Two.o code/Subject_Two.src code/Voice.d code/Voice.o code/Voice.src code/WIFI_UART.d code/WIFI_UART.o code/WIFI_UART.src code/by9301.d code/by9301.o code/by9301.src code/flash.d code/flash.o code/flash.src code/menu.d code/menu.o code/menu.src code/miscellaneous.d code/miscellaneous.o code/miscellaneous.src code/navigation.d code/navigation.o code/navigation.src code/path_diagram.d code/path_diagram.o code/path_diagram.src code/quaternion.d code/quaternion.o code/quaternion.src code/subject.d code/subject.o code/subject.src code/user_display_gps.d code/user_display_gps.o code/user_display_gps.src code/zf_device_dot_matrix_screen.d code/zf_device_dot_matrix_screen.o code/zf_device_dot_matrix_screen.src code/zf_device_tld7002.d code/zf_device_tld7002.o code/zf_device_tld7002.src

.PHONY: clean-code

