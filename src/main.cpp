#include "Common.h"
#include "Application.h"

int main()
{
    Log::setLevel(LogLevel::None);

    Application application;
    application.run();

    return 0;
}
