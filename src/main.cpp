#include "Common.h"
#include "Application.h"

int main()
{
    Log::setLevel(LogLevel::Error);

    Application application;
    application.run();

    return 0;
}
