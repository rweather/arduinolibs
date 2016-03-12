/*
This example demonstrates how to create a simple shell on the serial port.

This example is placed into the public domain.
*/

#include <Shell.h>

Shell shell;

int ledPin = 13;

void cmdLed(Shell &shell, int argc, const ShellArguments &argv)
{
    if (argc > 1 && !strcmp(argv[1], "on"))
        digitalWrite(ledPin, HIGH);
    else
        digitalWrite(ledPin, LOW);
}

ShellCommand(led, "Turns the status LED on or off", cmdLed);

void setup()
{
    pinMode(ledPin, OUTPUT);

    Serial.begin(9600);
    shell.setPrompt("Command: ");
    shell.begin(Serial, 5);
}

void loop()
{
    shell.loop();
}
