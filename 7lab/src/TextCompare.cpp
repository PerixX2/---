#include "TextCompare.h"
#include "TextCompareExceptions.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    bool isWordByte(unsigned char ch) noexcept
    {
        return std::isalnum(ch) != 0 || ch >= 0x80;
    }

    std::string normalizeWord(const std::string& raw)
    {
        std::string res;
        res.reserve(raw.size());
        for (std::size_t i = 0; i < raw.size(); ++i)
        {
            const unsigned char c = static_cast<unsigned char>(raw[i]);
            if (c < 0x80)
            {
                res.push_back(static_cast<char>(std::tolower(c)));
            }
            else if (i + 1 < raw.size())
            {
                const unsigned char c2 = static_cast<unsigned char>(raw[i + 1]);
                if (c == 0xD0 && c2 >= 0x90 && c2 <= 0x9F)
                {
                    res.push_back(static_cast<char>(0xD0));
                    res.push_back(static_cast<char>(c2 + 0x20));
                    ++i;
                }
                else if (c == 0xD0 && c2 >= 0xA0 && c2 <= 0xAF)
                {
                    res.push_back(static_cast<char>(0xD1));
                    res.push_back(static_cast<char>(c2 - 0x20));
                    ++i;
                }
                else if (c == 0xD0 && c2 == 0x81) 
                {
                    res.push_back(static_cast<char>(0xD1));
                    res.push_back(static_cast<char>(0x91));
                    ++i;
                }
                else if (c == 0xD0 && (c2 == 0x84 || c2 == 0x86 || c2 == 0x87)) 
                {
                    res.push_back(static_cast<char>(0xD1));
                    res.push_back(static_cast<char>(c2 + 0x10));
                    ++i;
                }
                else if (c == 0xD2 && c2 == 0x90) 
                {
                    res.push_back(static_cast<char>(0xD2));
                    res.push_back(static_cast<char>(0x91));
                    ++i;
                }
                else
                {
                    res.push_back(static_cast<char>(c));
                }
            }
            else
            {
                res.push_back(static_cast<char>(c));
            }
        }
        return res;
    }

    std::size_t computeLcsLength(
        const std::vector<std::string>& a,
        const std::vector<std::string>& b)
    {
        if (a.empty() || b.empty())
        {
            return 0;
        }

        const std::size_t n = a.size();
        const std::size_t m = b.size();

        if (n > TextCompare::MAX_LCS_MATRIX_CELLS / m)
        {
            throw TextSizeLimitException("LCS line matrix (n * m)", TextCompare::MAX_LCS_MATRIX_CELLS, n * m);
        }

        std::vector<std::size_t> prev(m + 1, 0);
        std::vector<std::size_t> curr(m + 1, 0);

        for (std::size_t i = 1; i <= n; ++i)
        {
            for (std::size_t j = 1; j <= m; ++j)
            {
                if (a[i - 1] == b[j - 1])
                {
                    curr[j] = prev[j - 1] + 1;
                }
                else
                {
                    curr[j] = std::max(prev[j], curr[j - 1]);
                }
            }
            std::swap(prev, curr);
            std::fill(curr.begin(), curr.end(), 0);
        }

        return prev[m];
    }
}

void TextCompare::validateSingleText(const std::string& text)
{
    if (text.size() > MAX_TEXT_LENGTH)
    {
        throw TextSizeLimitException("text length (bytes)", MAX_TEXT_LENGTH, text.size());
    }
    if (text.find('\0') != std::string::npos)
    {
        throw InvalidTextParameterException("text", "Binary NUL ('\\0') character is not allowed in plain text");
    }
}

std::vector<std::string> TextCompare::splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    if (text.empty())
    {
        return lines;
    }

    std::size_t start = 0;
    while (start <= text.size())
    {
        const std::size_t pos = text.find('\n', start);
        if (pos == std::string::npos)
        {
            if (start < text.size())
            {
                std::string line = text.substr(start);
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                lines.push_back(std::move(line));
            }
            break;
        }
        else
        {
            std::string line = text.substr(start, pos - start);
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            lines.push_back(std::move(line));
            start = pos + 1;
        }
    }

    return lines;
}

std::set<std::string> TextCompare::extractWordSet(const std::string& text)
{
    std::set<std::string> words;
    std::string current;

    for (unsigned char ch : text)
    {
        if (isWordByte(ch))
        {
            current.push_back(static_cast<char>(ch));
        }
        else if (!current.empty())
        {
            words.insert(normalizeWord(current));
            current.clear();
        }
    }
    if (!current.empty())
    {
        words.insert(normalizeWord(current));
    }

    return words;
}

std::string TextCompare::readFileUtf8(const std::string& filePath)
{
    if (filePath.empty())
    {
        throw std::invalid_argument("File path cannot be empty");
    }

    const fs::path path = fs::u8path(filePath);
    std::error_code ec;

    if (!fs::exists(path, ec))
    {
        throw FileAccessException(filePath, "File does not exist");
    }
    if (fs::is_directory(path, ec))
    {
        throw FileAccessException(filePath, "Path points to a directory, not a regular file");
    }

    const auto fileSize = fs::file_size(path, ec);
    if (!ec && fileSize > MAX_TEXT_LENGTH)
    {
        throw TextSizeLimitException("file size (bytes)", MAX_TEXT_LENGTH, static_cast<std::size_t>(fileSize));
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        throw FileAccessException(filePath, "Failed to open file for reading");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (file.bad())
    {
        throw FileAccessException(filePath, "I/O error occurred while reading file");
    }

    std::string content = buffer.str();
    validateSingleText(content);
    return content;
}

TextCompare::TextCompare(const std::vector<std::string>& texts)
{
    if (texts.size() > MAX_TEXTS)
    {
        throw TextSizeLimitException("texts collection size", MAX_TEXTS, texts.size());
    }

    std::vector<std::string> validated;
    validated.reserve(texts.size());
    for (const auto& t : texts)
    {
        validateSingleText(t);
        validated.push_back(t);
    }

    texts_ = std::move(validated);
}

std::size_t TextCompare::size() const noexcept
{
    return texts_.size();
}

bool TextCompare::empty() const noexcept
{
    return texts_.empty();
}

const std::vector<std::string>& TextCompare::texts() const noexcept
{
    return texts_;
}

const std::string& TextCompare::textAt(std::size_t index) const
{
    if (index >= texts_.size())
    {
        throw std::out_of_range("Text index is out of range");
    }
    return texts_[index];
}

void TextCompare::addText(const std::string& text)
{
    if (texts_.size() >= MAX_TEXTS)
    {
        throw TextSizeLimitException("texts collection size", MAX_TEXTS, texts_.size() + 1);
    }
    validateSingleText(text);
    texts_.push_back(text);
}

void TextCompare::addFile(const std::string& filePath)
{
    if (texts_.size() >= MAX_TEXTS)
    {
        throw TextSizeLimitException("texts collection size", MAX_TEXTS, texts_.size() + 1);
    }
    std::string content = readFileUtf8(filePath);
    texts_.push_back(std::move(content));
}

void TextCompare::replaceTexts(const std::vector<std::string>& newTexts)
{
    TextCompare temp(newTexts);

    texts_.swap(temp.texts_);
}

void TextCompare::replaceTextsUnsafe(const std::vector<std::string>& newTexts)
{
    texts_.clear();
    for (const auto& t : newTexts)
    {
        validateSingleText(t);
        if (texts_.size() >= MAX_TEXTS)
        {
            throw TextSizeLimitException("texts collection size", MAX_TEXTS, texts_.size() + 1);
        }
        texts_.push_back(t);
    }
}

std::vector<std::string> TextCompare::commonWordsFromStrings(const std::string& t1, const std::string& t2)
{
    validateSingleText(t1);
    validateSingleText(t2);

    const std::set<std::string> s1 = extractWordSet(t1);
    const std::set<std::string> s2 = extractWordSet(t2);

    std::vector<std::string> result;
    std::set_intersection(
        s1.begin(), s1.end(),
        s2.begin(), s2.end(),
        std::back_inserter(result));
    return result;
}

double TextCompare::similarityFromStrings(const std::string& t1, const std::string& t2)
{
    validateSingleText(t1);
    validateSingleText(t2);

    const std::set<std::string> s1 = extractWordSet(t1);
    const std::set<std::string> s2 = extractWordSet(t2);

    if (s1.empty() && s2.empty())
    {
        return 1.0;
    }

    std::size_t intersectionCount = 0;
    for (const auto& w : s1)
    {
        if (s2.count(w) > 0)
        {
            ++intersectionCount;
        }
    }

    const std::size_t unionCount = s1.size() + s2.size() - intersectionCount;
    return (unionCount > 0)
               ? static_cast<double>(intersectionCount) / static_cast<double>(unionCount)
               : 0.0;
}

std::size_t TextCompare::differenceCountFromStrings(const std::string& t1, const std::string& t2)
{
    validateSingleText(t1);
    validateSingleText(t2);

    const std::vector<std::string> lines1 = splitLines(t1);
    const std::vector<std::string> lines2 = splitLines(t2);

    const std::size_t lcs = computeLcsLength(lines1, lines2);
    return (lines1.size() - lcs) + (lines2.size() - lcs);
}

std::vector<std::string> TextCompare::commonWords(std::size_t idx1, std::size_t idx2) const
{
    return commonWordsFromStrings(textAt(idx1), textAt(idx2));
}

double TextCompare::similarity(std::size_t idx1, std::size_t idx2) const
{
    return similarityFromStrings(textAt(idx1), textAt(idx2));
}

std::size_t TextCompare::differenceCount(std::size_t idx1, std::size_t idx2) const
{
    return differenceCountFromStrings(textAt(idx1), textAt(idx2));
}

FileComparisonResult TextCompare::compareFiles(const std::string& path1, const std::string& path2) const
{
    const std::string content1 = readFileUtf8(path1);
    const std::string content2 = readFileUtf8(path2);

    const std::vector<std::string> lines1 = splitLines(content1);
    const std::vector<std::string> lines2 = splitLines(content2);

    FileComparisonResult res;
    res.identical = (content1 == content2);
    res.lines1 = lines1.size();
    res.lines2 = lines2.size();
    res.differenceCount = differenceCountFromStrings(content1, content2);
    res.similarity = similarityFromStrings(content1, content2);
    res.commonWordCount = commonWordsFromStrings(content1, content2).size();

    const std::size_t minLines = std::min(lines1.size(), lines2.size());
    for (std::size_t i = 0; i < minLines; ++i)
    {
        if (lines1[i] != lines2[i])
        {
            res.firstDifferenceLine = i + 1;
            return res;
        }
    }
    if (lines1.size() != lines2.size())
    {
        res.firstDifferenceLine = minLines + 1;
    }
    else
    {
        res.firstDifferenceLine = 0;
    }

    return res;
}
