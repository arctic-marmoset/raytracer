/**
 * @file raytracer.cpp
 * @brief Contains functions that handle set-up and execution of the raytracer program.
 */

#include "png.hpp"
#include "rng.hpp"

#include <fmt/core.h>

#include <exception>

/**
 * @brief The real entry point of the program.
 * @throws std::exception Thrown if a fatal error occurs.
 */
void run();

/**
 * @brief Wraps run() in a try-catch to handle printing fatal errors.
 */
int main()
{
    try
    {
        run();
    }
    catch (const std::exception &e)
    {
        fmt::print(stderr, "fatal error: {}\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void run()
{
    throw std::exception("error in run(): not implemented");
}
