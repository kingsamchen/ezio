/*
 @ 0xCCCCCCCC
*/

#include "ezio/io_service_context.h"

#include "kbase/at_exit_manager.h"
#include "kbase/error_exception_util.h"

namespace {

ezio::IOServiceContext* instance = nullptr;

}   // namespace

namespace ezio {

// static
void IOServiceContext::Init()
{
    ENSURE(CHECK, instance == nullptr).Require();
    instance = new IOServiceContext();
    kbase::AtExitManager::RegisterCallback([] {
        delete instance;
        instance = nullptr;
    });
}

// static
const IOServiceContext& IOServiceContext::current()
{
    ENSURE(CHECK, instance != nullptr).Require();
    return *instance;
}

}   // namespace ezio
