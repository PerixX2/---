#include <iostream>
#include <string>
#include <stdexcept>
#include <exception>

void throwIntegerCode()
{
    throw -1;
}

void throwStringMessage()
{
    throw std::string("File not found in TextCompare");
}

void throwStandardHierarchy()
{
    throw std::invalid_argument("File path cannot be empty");
}

void rethrowWithSlicing()
{
    try
    {
        throw std::invalid_argument("Invalid file path parameter");
    }
    catch (const std::exception& ex)
    {
        throw ex;
    }
}

int main()
{
    std::cout << "=== Example 1: int exception ===\n";
    try
    {
        throwIntegerCode();
    }
    catch (const std::exception& ex)
    {
        std::cout << "Caught std::exception: " << ex.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Caught via catch (...) [int is NOT caught by catch(const std::exception&)]\n";
    }

    std::cout << "\n=== Example 2: std::string exception ===\n";
    try
    {
        throwStringMessage();
    }
    catch (const std::exception& ex)
    {
        std::cout << "Caught std::exception: " << ex.what() << '\n';
    }
    catch (const std::string& msg)
    {
        std::cout << "Caught std::string explicitly: " << msg << '\n';
    }

    std::cout << "\n=== Example 3: standard exception hierarchy ===\n";
    try
    {
        throwStandardHierarchy();
    }
    catch (const std::exception& ex)
    {
        std::cout << "Caught std::exception: " << ex.what() << '\n';
    }

    std::cout << "\n=== Example 4: object slicing with 'throw ex;' ===\n";
    try
    {
        rethrowWithSlicing();
    }
    catch (const std::invalid_argument& ex)
    {
        std::cout << "Caught std::invalid_argument: " << ex.what() << '\n';
    }
    catch (const std::exception& ex)
    {
        std::cout << "Caught sliced base std::exception: " << ex.what() << '\n';
    }

    return 0;
}
