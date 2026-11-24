/*
 * Model.c
 *
 *  Created on: 2026Äê3ÔÂ24ÈÕ
 *      Author: liang2281
 */

#include "Model.h"

uint8_t CurrMode, NextMode = 1;

void Mode1_Init(void)
{

}
void Mode1_Loop(void)
{

}
void Mode1_Exit(void)
{

}
//
void Mode2_Init(void)
{

}
void Mode2_Loop(void)
{

}
void Mode2_Exit(void)
{

}

void loop_Mode(void)
{
    if (CurrMode == NextMode)
    {
        switch (CurrMode)
        {
            case 1: Mode1_Loop(); break;
            case 2: Mode2_Loop(); break;
        }
    }
    else
    {
        switch (CurrMode)
        {
            case 1: Mode1_Exit(); break;
            case 2: Mode2_Exit(); break;
        }
        switch (NextMode)
        {
            case 1: Mode1_Init(); break;
            case 2: Mode2_Init(); break;
        }
        CurrMode = NextMode;
    }
}

void Mode_Tick(void)
{
    if (CurrMode == NextMode)
    {
        switch (CurrMode)
        {
            case 1: ; break;
        }
    }
}




