################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/Algorithmic_Filtering.c \
../code/Common_peripheral.c \
../code/Encoder.c \
../code/GPS.c \
../code/HotRC.c \
../code/Model.c \
../code/Motor.c \
../code/NRF24L01.c \
../code/PID.c \
../code/PID_Steer.c \
../code/Serial.c \
../code/flash.c \
../code/path_diagram.c \
../code/quaternion.c \
../code/subject.c \
../code/user_display_gps.c 

COMPILED_SRCS += \
code/Algorithmic_Filtering.src \
code/Common_peripheral.src \
code/Encoder.src \
code/GPS.src \
code/HotRC.src \
code/Model.src \
code/Motor.src \
code/NRF24L01.src \
code/PID.src \
code/PID_Steer.src \
code/Serial.src \
code/flash.src \
code/path_diagram.src \
code/quaternion.src \
code/subject.src \
code/user_display_gps.src 

C_DEPS += \
code/Algorithmic_Filtering.d \
code/Common_peripheral.d \
code/Encoder.d \
code/GPS.d \
code/HotRC.d \
code/Model.d \
code/Motor.d \
code/NRF24L01.d \
code/PID.d \
code/PID_Steer.d \
code/Serial.d \
code/flash.d \
code/path_diagram.d \
code/quaternion.d \
code/subject.d \
code/user_display_gps.d 

OBJS += \
code/Algorithmic_Filtering.o \
code/Common_peripheral.o \
code/Encoder.o \
code/GPS.o \
code/HotRC.o \
code/Model.o \
code/Motor.o \
code/NRF24L01.o \
code/PID.o \
code/PID_Steer.o \
code/Serial.o \
code/flash.o \
code/path_diagram.o \
code/quaternion.o \
code/subject.o \
code/user_display_gps.o 


# Each subdirectory must supply rules for building sources it contributes
code/Algorithmic_Filtering.src: ../code/Algorithmic_Filtering.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Algorithmic_Filtering.o: code/Algorithmic_Filtering.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Common_peripheral.src: ../code/Common_peripheral.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Common_peripheral.o: code/Common_peripheral.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Encoder.src: ../code/Encoder.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Encoder.o: code/Encoder.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/GPS.src: ../code/GPS.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/GPS.o: code/GPS.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/HotRC.src: ../code/HotRC.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/HotRC.o: code/HotRC.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Model.src: ../code/Model.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Model.o: code/Model.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Motor.src: ../code/Motor.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Motor.o: code/Motor.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/NRF24L01.src: ../code/NRF24L01.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/NRF24L01.o: code/NRF24L01.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID.src: ../code/PID.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID.o: code/PID.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/PID_Steer.src: ../code/PID_Steer.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/PID_Steer.o: code/PID_Steer.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/Serial.src: ../code/Serial.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/Serial.o: code/Serial.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/flash.src: ../code/flash.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/flash.o: code/flash.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/path_diagram.src: ../code/path_diagram.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/path_diagram.o: code/path_diagram.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/quaternion.src: ../code/quaternion.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/quaternion.o: code/quaternion.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/subject.src: ../code/subject.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/subject.o: code/subject.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
code/user_display_gps.src: ../code/user_display_gps.c code/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/project/Project/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
code/user_display_gps.o: code/user_display_gps.src code/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) code/Algorithmic_Filtering.d code/Algorithmic_Filtering.o code/Algorithmic_Filtering.src code/Common_peripheral.d code/Common_peripheral.o code/Common_peripheral.src code/Encoder.d code/Encoder.o code/Encoder.src code/GPS.d code/GPS.o code/GPS.src code/HotRC.d code/HotRC.o code/HotRC.src code/Model.d code/Model.o code/Model.src code/Motor.d code/Motor.o code/Motor.src code/NRF24L01.d code/NRF24L01.o code/NRF24L01.src code/PID.d code/PID.o code/PID.src code/PID_Steer.d code/PID_Steer.o code/PID_Steer.src code/Serial.d code/Serial.o code/Serial.src code/flash.d code/flash.o code/flash.src code/path_diagram.d code/path_diagram.o code/path_diagram.src code/quaternion.d code/quaternion.o code/quaternion.src code/subject.d code/subject.o code/subject.src code/user_display_gps.d code/user_display_gps.o code/user_display_gps.src

.PHONY: clean-code

