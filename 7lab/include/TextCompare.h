#pragma once

#include <string>
#include <vector>
#include <set>
#include <cstddef>

struct FileComparisonResult
{
    bool identical = false;
    std::size_t lines1 = 0;
    std::size_t lines2 = 0;
    std::size_t differenceCount = 0;
    std::size_t firstDifferenceLine = 0;
    double similarity = 1.0;
    std::size_t commonWordCount = 0;
};

class TextCompare
{
public:
    static constexpr std::size_t MAX_TEXTS = 100;
    static constexpr std::size_t MAX_TEXT_LENGTH = 10'000'000;
    static constexpr std::size_t MAX_LCS_MATRIX_CELLS = 25'000'000;

private:
    std::vector<std::string> texts_;

    static void validateSingleText(const std::string& text);
    static std::vector<std::string> splitLines(const std::string& text);
    static std::set<std::string> extractWordSet(const std::string& text);
    static std::string readFileUtf8(const std::string& filePath);

public:
    TextCompare() noexcept = default;
    explicit TextCompare(const std::vector<std::string>& texts);

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    const std::vector<std::string>& texts() const noexcept;

    const std::string& textAt(std::size_t index) const;
    void addText(const std::string& text);
    void addFile(const std::string& filePath);
    void replaceTexts(const std::vector<std::string>& newTexts);
    void replaceTextsUnsafe(const std::vector<std::string>& newTexts);

    std::vector<std::string> commonWords(std::size_t idx1, std::size_t idx2) const;
    double similarity(std::size_t idx1, std::size_t idx2) const;
    std::size_t differenceCount(std::size_t idx1, std::size_t idx2) const;
    FileComparisonResult compareFiles(const std::string& path1, const std::string& path2) const;

    static std::vector<std::string> commonWordsFromStrings(const std::string& t1, const std::string& t2);
    static double similarityFromStrings(const std::string& t1, const std::string& t2);
    static std::size_t differenceCountFromStrings(const std::string& t1, const std::string& t2);
};
