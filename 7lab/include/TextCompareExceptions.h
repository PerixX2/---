#pragma once

#include <stdexcept>
#include <string>
#include <cstddef>

class TextCompareException : public std::runtime_error
{
public:
    explicit TextCompareException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class FileAccessException : public TextCompareException
{
private:
    std::string filePath_;
    std::string reason_;

public:
    FileAccessException(const std::string& filePath, const std::string& reason)
        : TextCompareException("File access error for '" + filePath + "': " + reason),
          filePath_(filePath),
          reason_(reason)
    {
    }

    const std::string& filePath() const noexcept
    {
        return filePath_;
    }

    const std::string& reason() const noexcept
    {
        return reason_;
    }
};

class TextSizeLimitException : public TextCompareException
{
private:
    std::string resourceName_;
    std::size_t maxAllowed_;
    std::size_t actualSize_;

public:
    TextSizeLimitException(
        const std::string& resourceName,
        std::size_t maxAllowed,
        std::size_t actualSize)
        : TextCompareException(
              "Size limit exceeded for " + resourceName +
              " (max=" + std::to_string(maxAllowed) +
              ", actual=" + std::to_string(actualSize) + ")"),
          resourceName_(resourceName),
          maxAllowed_(maxAllowed),
          actualSize_(actualSize)
    {
    }

    const std::string& resourceName() const noexcept
    {
        return resourceName_;
    }

    std::size_t maxAllowed() const noexcept
    {
        return maxAllowed_;
    }

    std::size_t actualSize() const noexcept
    {
        return actualSize_;
    }
};

class InvalidTextParameterException : public TextCompareException
{
private:
    std::string parameterName_;
    std::string details_;

public:
    InvalidTextParameterException(
        const std::string& parameterName,
        const std::string& details)
        : TextCompareException("Invalid parameter [" + parameterName + "]: " + details),
          parameterName_(parameterName),
          details_(details)
    {
    }

    const std::string& parameterName() const noexcept
    {
        return parameterName_;
    }

    const std::string& details() const noexcept
    {
        return details_;
    }
};
