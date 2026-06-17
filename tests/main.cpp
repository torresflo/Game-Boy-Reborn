#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "DebugHelpers.h"

int main(int argc, char** argv)
{
    // Quiet the per-instruction Debug trace from CentralProcessingUnit::step()
    // so test output stays readable; Warning/Error/Fatal still print.
    Log::setLevel(LogLevel::Warning);

    doctest::Context context(argc, argv);
    return context.run();
}
