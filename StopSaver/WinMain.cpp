#include "StopSaverApp.h"
#include "SingleInstanceGuard.h"
#include "Config.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    SingleInstanceGuard instance(L"StopSaverTrayAppMutex");
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<Config> config = std::make_shared<Config>();
    std::wstring logPath = config->getLogFile();

    if (logPath.empty()) {
        MessageBoxW(nullptr, L"Failed to expand log path.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    try {
        logger = spdlog::rotating_logger_mt("stopsaver", logPath, config->getMaxSizeLogSize(), 1);
        spdlog::flush_every(std::chrono::seconds(1));
        logger->set_level(config->getLogLevel());
        logger->set_pattern("%Y-%m-%dT%H:%M:%S.%f%z %l %P: %v");
        logger->info(L"Application started");
    }
    catch (const spdlog::spdlog_ex&) {
        MessageBoxW(nullptr, L"Log initialization failed.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    if (instance.alreadyExists()) {
        logger->info(L"Another instance of the application is already running, exiting.");
        return 1;
    }

    StopSaverApp app(config, logger);
    if (!app.init(hInstance)) {
        logger->info(L"Initialization failed, exiting");
        return 1;
    }

    int rc = app.run();
    logger->info(L"Application exited");
    return rc;
}
