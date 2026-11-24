################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/Algorithmic_Filtering.c \
../code/Angle_Encoder.c \
../code/CI1302.c \
../code/Color_discirmination.c \
../code/Common_peripheral.c \
../code/Encoder.c \
../code/GPS.c \
../code/HotRC.c \
../code/IMU.c \
../code/Motor.c \
../code/PID.c \
../code/PID_Steer.c \
../code/Project2.c \
../code/Serial.c \
../code/Subject_Two.c \
../code/Updata_math.c \
../code/blue_square.c \
../code/by9301.c \
../code/flash.c \
../code/kalman.c \
../code/matrix.c \
../code/mcu_dmp.c \
../code/menu.c \
../code/navigation.c \
../code/path_diagram.c \
../code/project3.c \
../code/square_kalman_filter.c \
../code/subject.c \
../code/user_display_gps.c \
../code/vofa.c \
../code/zf_device_dot_matrix_screen.c \
../code/zf_device_tld7002.c 

COMPILED_SRCS += \
code/Algorithmic_Filtering.src \
code/Angle_Encoder.src \
code/CI1302.src \
code/Color_discirmination.src \
code/Common_peripheral.src \
code/Encoder.src \
code/GPS.src \
code/HotRC.src \
code/IMU.src \
code/Motor.src \
code/PID.src \
code/PID_Steer.src \
code/Project2.src \
code/Serial.src \
code/Subject_Two.src \
code/Updata_math.src \
code/blue_square.src \
code/by9301.src \
code/flash.src \
code/kalman.src \
code/matrix.src \
code/mcu_dmp.src \
code/menu.src \
code/navigation.src \
code/path_diagram.src \
code/project3.src \
code/square_kalman_filter.src \
code/subject.src \
code/user_display_gps.src \
code/vofa.src \
code/zf_device_dot_matrix_screen.src \
code/zf_device_tld7002.src 

C_DEPS += \
code/Algorithmic_Filtering.d \
code/Angle_Encoder.d \
code/CI1302.d \
code/Color_discirmination.d \
code/Common_peripheral.d \
code/Encoder.d \
code/GPS.d \
code/HotRC.d \
code/IMU.d \
code/Motor.d \
code/PID.d \
code/PID_Steer.d \
code/Project2.d \
code/Serial.d \
code/Subject_Two.d \
code/Updata_math.d \
code/blue_square.d \
code/by9301.d \
code/flash.d \
code/kalman.d \
code/matrix.d \
code/mcu_dmp.d \
code/menu.d \
code/navigation.d \
code/path_diagram.d \
code/project3.d \
code/square_kalman_filter.d \
code/subject.d \
code/user_display_gps.d \
code/vofa.d \
code/zf_device_dot_matrix_screen.d \
code/zf_device_tld7002.d 

OBJS += \
code/Algorithmic_Filtering.o \
code/Angle_Encoder.o \
code/CI1302.o \
code/Color_discirmination.o \
code/Common_peripheral.o \
code/Encoder.o \
code/GPS.o \
code/HotRC.o \
code/IMU.o \
code/Motor.o \
code/PID.o \
code/PID_Steer.o \
code/Project2.o \
code/Serial.o \
code/Subject_Two.o \
code/Updata_math.o \
code/blue_square.o \
code/by9301.o \
code/flash.o \
code/kalman.o \
code/matrix.o \
code/mcu_dmp.o \
code/menu.o \
code/navigation.o \
code/path_diagram.o \
code/project3.o \
code/square_kalman_filter.o \
code/subject.o \
code/user_display_gps.o \
code/vofa.o \
code/zf_device_dot_matrix_screen.o \
code/zf_device_tld7002.o 


# Each subdirectory must supply rules for building sources it contributes
code/Algorithmic_Filtering.src: ../code/Algorithmic_Filtering.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Algorithmic_Filtering.o: code/Algorithmic_Filtering.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Angle_Encoder.src: ../code/Angle_Encoder.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Angle_Encoder.o: code/Angle_Encoder.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/CI1302.src: ../code/CI1302.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/CI1302.o: code/CI1302.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Color_discirmination.src: ../code/Color_discirmination.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Color_discirmination.o: code/Color_discirmination.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Common_peripheral.src: ../code/Common_peripheral.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Common_peripheral.o: code/Common_peripheral.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Encoder.src: ../code/Encoder.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Encoder.o: code/Encoder.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/GPS.src: ../code/GPS.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/GPS.o: code/GPS.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/HotRC.src: ../code/HotRC.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/HotRC.o: code/HotRC.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/IMU.src: ../code/IMU.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/IMU.o: code/IMU.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Motor.src: ../code/Motor.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Motor.o: code/Motor.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID.src: ../code/PID.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID.o: code/PID.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID_Steer.src: ../code/PID_Steer.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID_Steer.o: code/PID_Steer.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Project2.src: ../code/Project2.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Project2.o: code/Project2.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Serial.src: ../code/Serial.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Serial.o: code/Serial.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Subject_Two.src: ../code/Subject_Two.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Subject_Two.o: code/Subject_Two.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Updata_math.src: ../code/Updata_math.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Updata_math.o: code/Updata_math.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/blue_square.src: ../code/blue_square.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/blue_square.o: code/blue_square.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/by9301.src: ../code/by9301.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/by9301.o: code/by9301.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/flash.src: ../code/flash.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/flash.o: code/flash.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/kalman.src: ../code/kalman.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/kalman.o: code/kalman.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/matrix.src: ../code/matrix.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/matrix.o: code/matrix.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/mcu_dmp.src: ../code/mcu_dmp.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/mcu_dmp.o: code/mcu_dmp.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/menu.src: ../code/menu.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/menu.o: code/menu.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/navigation.src: ../code/navigation.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/navigation.o: code/navigation.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/path_diagram.src: ../code/path_diagram.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/path_diagram.o: code/path_diagram.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/project3.src: ../code/project3.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/project3.o: code/project3.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/square_kalman_filter.src: ../code/square_kalman_filter.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/square_kalman_filter.o: code/square_kalman_filter.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/subject.src: ../code/subject.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/subject.o: code/subject.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/user_display_gps.src: ../code/user_display_gps.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/user_display_gps.o: code/user_display_gps.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/vofa.src: ../code/vofa.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/vofa.o: code/vofa.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/zf_device_dot_matrix_screen.src: ../code/zf_device_dot_matrix_screen.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/zf_device_dot_matrix_screen.o: code/zf_device_dot_matrix_screen.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/zf_device_tld7002.src: ../code/zf_device_tld7002.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/Users/liang2281/Desktop/project/kadingcheguosaidai/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=2 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/zf_device_tld7002.o: code/zf_device_tld7002.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) code/Algorithmic_Filtering.d code/Algorithmic_Filtering.o code/Algorithmic_Filtering.src code/Angle_Encoder.d code/Angle_Encoder.o code/Angle_Encoder.src code/CI1302.d code/CI1302.o code/CI1302.src code/Color_discirmination.d code/Color_discirmination.o code/Color_discirmination.src code/Common_peripheral.d code/Common_peripheral.o code/Common_peripheral.src code/Encoder.d code/Encoder.o code/Encoder.src code/GPS.d code/GPS.o code/GPS.src code/HotRC.d code/HotRC.o code/HotRC.src code/IMU.d code/IMU.o code/IMU.src code/Motor.d code/Motor.o code/Motor.src code/PID.d code/PID.o code/PID.src code/PID_Steer.d code/PID_Steer.o code/PID_Steer.src code/Project2.d code/Project2.o code/Project2.src code/Serial.d code/Serial.o code/Serial.src code/Subject_Two.d code/Subject_Two.o code/Subject_Two.src code/Updata_math.d code/Updata_math.o code/Updata_math.src code/blue_square.d code/blue_square.o code/blue_square.src code/by9301.d code/by9301.o code/by9301.src code/flash.d code/flash.o code/flash.src code/kalman.d code/kalman.o code/kalman.src code/matrix.d code/matrix.o code/matrix.src code/mcu_dmp.d code/mcu_dmp.o code/mcu_dmp.src code/menu.d code/menu.o code/menu.src code/navigation.d code/navigation.o code/navigation.src code/path_diagram.d code/path_diagram.o code/path_diagram.src code/project3.d code/project3.o code/project3.src code/square_kalman_filter.d code/square_kalman_filter.o code/square_kalman_filter.src code/subject.d code/subject.o code/subject.src code/user_display_gps.d code/user_display_gps.o code/user_display_gps.src code/vofa.d code/vofa.o code/vofa.src code/zf_device_dot_matrix_screen.d code/zf_device_dot_matrix_screen.o code/zf_device_dot_matrix_screen.src code/zf_device_tld7002.d code/zf_device_tld7002.o code/zf_device_tld7002.src

.PHONY: clean-code

