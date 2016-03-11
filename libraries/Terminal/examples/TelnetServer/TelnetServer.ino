
/*
This example demonstrates how to create a simple telnet server.

This example is placed into the public domain.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Shell.h>

byte macAddress[6] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

int ledPin = 13;

EthernetServer server(23);
EthernetClient client;
bool haveClient = false;

Shell shell;

void cmdLed(Shell &shell, int argc, const ShellArguments &argv)
{
    if (argc > 1 && !strcmp(argv[1], "on"))
        digitalWrite(ledPin, HIGH);
    else
        digitalWrite(ledPin, LOW);
}

void cmdExit(Shell &shell, int argc, const ShellArguments &argv)
{
    client.stop();
}

ShellCommand(led, "Turns the status LED on or off", cmdLed);
ShellCommand(exit, "Exit and log out", cmdExit);

void setup()
{
    // Configure I/O.
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Start the serial port for status messages.
    Serial.begin(9600);
    Serial.println();
    Serial.print("Acquiring IP address ... ");

    // Start Ethernet running and get an IP address via DHCP.
    if (Ethernet.begin(macAddress))
        Serial.println(Ethernet.localIP());
    else
        Serial.println("failed");

    // Listen on port 23 for incoming telnet connections.
    server.begin();

    // Configure the shell.  We call Shell::begin() once we have a connection.
    shell.setPrompt("$ ");
}

void loop()
{
    // Maintain the DHCP lease over time.
    Ethernet.maintain();

    // Handle new/disconnecting clients.
    if (!haveClient) {
        // Check for new client connections.
        client = server.available();
        if (client) {
            haveClient = true;
            shell.begin(client, 5, Terminal::Telnet);
        }
    } else if (!client.connected()) {
        // The current client has been disconnected.  Shut down the shell.
        shell.end();
        client.stop();
        client = EthernetClient();
        haveClient = false;
    }

    // Perform periodic shell processing on the active client.
    shell.loop();
}
