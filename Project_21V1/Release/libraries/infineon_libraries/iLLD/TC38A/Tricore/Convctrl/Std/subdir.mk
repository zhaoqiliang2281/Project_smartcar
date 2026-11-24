################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.c 

COMPILED_SRCS += \
libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.src 

C_DEPS += \
libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.d 

OBJS += \
libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.src: ../libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.c libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc38x "-fH:/ADS_21_Project/TC387Code/tc387-library/TC387Project/Project/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<"
libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.o: libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.src libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC38A-2f-Tricore-2f-Convctrl-2f-Std

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC38A-2f-Tricore-2f-Convctrl-2f-Std:
	-$(RM) libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.d libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.o libraries/infineon_libraries/iLLD/TC38A/Tricore/Convctrl/Std/IfxConvctrl.src

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC38A-2f-Tricore-2f-Convctrl-2f-Std

