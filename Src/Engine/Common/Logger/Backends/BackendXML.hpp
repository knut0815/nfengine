/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of XML logger backend
 */

#pragma once

#include "../LoggerBackend.hpp"
#include "../../FileSystem/File.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace Common {

/**
 * XML logger backend implementation.
 */
class NFCOMMON_API LoggerBackendXML : public ILoggerBackend
{
    File mFile;
    DynArray<char> mBuffer;

public:
    LoggerBackendXML();
    ~LoggerBackendXML();

    bool Init() override;
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
