#include "Common.h"
#include "Application.h"

int main()
{
    Log::setLevel(LogLevel::Info);

    Application application;
    application.run();

    return 0;
}
