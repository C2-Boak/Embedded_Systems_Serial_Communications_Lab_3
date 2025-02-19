

#include "mbed.h"
#include "arm_book_lib.h"



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


Ticker alarmstateTicker;

bool alarmState = OFF;
bool gasAlarm = OFF;
bool tempAlarm = OFF;
int numberOfIncorrectCodes = 0;



void inputsInit();
void outputsInit();

void alarmActivationUpdate();
void alarmDeactivationUpdate();

void uartTask();
void availableCommands();
void AlarmstateTrans();

float Gas_LEVELS = 0.673;
float TEMP = 23.00;



int main()
{
    inputsInit();
    outputsInit();

    alarmstateTicker.attach(&AlarmstateTrans, (6s));

    while (true) {
        alarmActivationUpdate();
        alarmDeactivationUpdate();
        uartTask();
    }
}


void AlarmstateTrans()
{
    if (alarmState) {
        uartUsb.write("\n The alarm is activated!\r\n", 28);
    } else {
        uartUsb.write("\n The alarm is not activated!\r\n", 32);
    }


    int tempInt = (int)TEMP;
    int tempFrac = (int)((TEMP - tempInt) * 100);
    char tempString[20];
    int tempLength = sprintf(tempString, "%d.%02d", tempInt, tempFrac);
    uartUsb.write("\n Temperature: ", 15);
    uartUsb.write(tempString, tempLength);
    uartUsb.write(" °C\r\n", 5);


    if (TEMP > 42.0) {
        uartUsb.write("Warning: Temperature exceeds 42°C!\r\n", 36);
    } else if (TEMP < 5.0) {
        uartUsb.write("Warning: Temperature is below 5°C!\r\n", 36);
    }


    int gasInt = (int)Gas_LEVELS;
    int gasFrac = (int)((Gas_LEVELS - gasInt) * 100);
    char gasString[20];
    int gasLength = sprintf(gasString, "%d.%02d", gasInt, gasFrac);
    uartUsb.write("\n Gas level: ", 13);
    uartUsb.write(gasString, gasLength);
    uartUsb.write(" ml\r\n", 5);

    if (Gas_LEVELS > 5.0) {
        uartUsb.write("Warning: Gas level exceeds 5 ml!\r\n", 33);
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
