//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

//=====[Declaration and initialization of public global objects]===============

DigitalIn enterButton(BUTTON1);
DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);
DigitalIn aButton(D4);
DigitalIn bButton(D5);
DigitalIn cButton(D6);
DigitalIn dButton(D7);

DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============

Ticker alarmstateTicker;

bool alarmState = OFF;
bool gasAlarm = OFF;
bool tempAlarm = OFF;
int numberOfIncorrectCodes = 0;

//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit();

void alarmActivationUpdate();
void alarmDeactivationUpdate();

void uartTask();
void availableCommands();
void AlarmstateTrans();  // Declaration of AlarmstateTrans function

//=====[Main function, the program entry point after power on or reset]========

int main()
{
    inputsInit();
    outputsInit();

    alarmstateTicker.attach(&AlarmstateTrans, 10.0);

    while (true) {
        alarmActivationUpdate();
        alarmDeactivationUpdate();
        uartTask();
    }
}

//=====[Implementations of public functions]===================================

void AlarmstateTrans()
{
    if (alarmState) {
        uartUsb.write("\n The alarm is activated!\r\n", 28);
    } else {
        uartUsb.write("\n The alarm is not activated!\r\n", 33);
    }
}

void inputsInit()
{
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);
}

void outputsInit()
{
    alarmLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
}

void alarmActivationUpdate()
{
    if (gasDetector || overTempDetector) {
        alarmState = ON;
    }
    alarmLed = alarmState;

    if (gasDetector) {
        gasAlarm = ON;
    }
    if (overTempDetector) {
        tempAlarm = ON;
    }
}

void alarmDeactivationUpdate()
{
    if (numberOfIncorrectCodes < 5) {
        if (aButton && bButton && cButton && dButton && !enterButton) {
            incorrectCodeLed = OFF;
        }
        if (enterButton && !incorrectCodeLed && alarmState) {
            if (aButton && bButton && !cButton && !dButton) {
                alarmState = OFF;
                numberOfIncorrectCodes = 0;
            } else {
                incorrectCodeLed = ON;
                numberOfIncorrectCodes = numberOfIncorrectCodes + 1;
            }
        }
    } else {
        systemBlockedLed = ON;
    }
}

void uartTask()
{
    char receivedChar = '\0';
    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);

        switch (receivedChar) {
            case '1':
                if (alarmState) {
                    uartUsb.write("\n The alarm is activated!\r\n", 28);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                } else {
                    uartUsb.write("\n The alarm is not activated!\r\n", 33);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                }
                break;

            case '2':
                if (gasAlarm) {
                    uartUsb.write("\n Gas detector is triggered!\r\n", 31);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                } else {
                    uartUsb.write("\n Gas detector is not triggered!\r\n", 36);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                }
                break;

            case '3':
                if (tempAlarm) {
                    uartUsb.write("\n Over temperature detector is triggered!\r\n", 45);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                } else {
                    uartUsb.write("\n Over temperature detector is not triggered!\r\n", 50);
                    uartUsb.write("\n Make another selection?\r\n", 28);
                }
                break;

            default:
                availableCommands();
                break;
        }
    }
}

void availableCommands()
{
    uartUsb.write("\nAvailable commands:\r\n", 24 );
    uartUsb.write("Press '1' to get the alarm state\r\n", 36 );
    uartUsb.write("Press '2' to get Gas Detector state\r\n", 40 );
    uartUsb.write("Press '3' to get Temperature Detector state\r\n\r\n", 46 );
}
