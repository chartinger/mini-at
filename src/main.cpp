#include <Arduino.h>
/*
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
  Based on the ATCommands example
*/
#include <ATCommands.h>
#define AT_COMMANDS_DEBUG
#define WORKING_BUFFER_SIZE 255 // The size of the working buffer (ie: the expected length of the input string)

ATCommands AT; // create an instance of the class
String str1;
String str2;

/**
 * @brief at_run_cmd_print
 * This is called when AT+PRINT is sent and is intended to invoke a function that does
 * not require parameters or has already had them set via WRITE (see other examples)
 * @param sender 
 * @return true 
 * @return false 
 */
AT_COMMAND_RETURN_TYPE at_run_cmd_print(ATCommands *sender)
{
    if (str1.length() > 0 && str2.length() > 0)
    {
        sender->serial->print(str1);
        sender->serial->print(" ");
        sender->serial->println(str2);
        return 0; // tells ATCommands to print OK
    }
    return -1;
}

/**
 * @brief at_test_cmd_print
 * This is called when a test command is received (AT+TEST=?) and is usually invoked when
 * information needs to be retrieved (such as a list of SSIDs for WIFI) or other tests
 * not requiring parameters.
 * @param sender 
 * @return true 
 * @return false 
 */
AT_COMMAND_RETURN_TYPE at_test_cmd_print(ATCommands *sender)
{
    sender->serial->print(sender->command);
    Serial.println(F("=<TEXT:STRING[RO]>"));
    Serial.println(F("Prints \"Hello World\" to the terminal"));
    return 0; // tells ATCommands to print OK
}

/**
 * @brief at_write_cmd_print
 * This is called when a command is received (AT+TEST=param1,param2) and is usually invoked when
 * information needs to be recorded (such as variables being set etc).  The sender object
 * will provide the list of parameters.
 * not requiring parameters.
 * @param sender 
 * @return true 
 * @return false 
 */
AT_COMMAND_RETURN_TYPE at_write_cmd_print(ATCommands *sender)
{
    // sender->next() is NULL terminated ('\0') if there are no more parameters
    // so check for that or a length of 0.
    str1 = sender->next();
    str2 = sender->next();
    Serial.println(str1);
    Serial.println(str2);
    return 0; // tells ATCommands to print OK
}

AT_COMMAND_RETURN_TYPE ping(ATCommands *sender)
{
    // sender->next() is NULL terminated ('\0') if there are no more parameters
    // so check for that or a length of 0.
    Serial.println(F("pong"));
    return 2; // tells ATCommands to print OK
}

AT_COMMAND_RETURN_TYPE passthrough(ATCommands *sender)
{
    // sender->next() is NULL terminated ('\0') if there are no more parameters
    // so check for that or a length of 0.
    Serial.println(F("BUFFER START"));
    Serial.println(sender->getBuffer());
    Serial.println(F("BUFFER END"));
    return 0; // tells ATCommands to print OK!
}

// declare the commands in an array to be passed during initialization
static at_command_t commands[] = {
    {"+PRINT", at_run_cmd_print, at_test_cmd_print, nullptr, at_write_cmd_print, nullptr},
    {"+GMR", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIFSR", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIPMUX", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIPRECVMODE", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIPSERVER", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CIPSEND", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CWHOSTNAME", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CWJAP", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CWJAP_CUR", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CWMODE", ping, nullptr, nullptr, nullptr, nullptr},
    {"+CWSAP", ping, nullptr, nullptr, nullptr, nullptr},
    {"+MDNS", ping, nullptr, nullptr, nullptr, nullptr},
    {"+RST", ping, nullptr, nullptr, nullptr, passthrough},
};

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    AT.begin(&Serial, commands, sizeof(commands), WORKING_BUFFER_SIZE);
    Serial.println("STARTING");
}

void loop()
{
    // put your main code here, to run repeatedly:
    AT.update();
}