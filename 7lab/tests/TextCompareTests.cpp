#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "TextCompare.h"
#include "TextCompareExceptions.h"

namespace fs = std::filesystem;

TEST(TextCompareTest, CreatesValidCollectionAndPreservesInvariants)
{
    TextCompare emptyTc;
    EXPECT_TRUE(emptyTc.empty());
    EXPECT_EQ(emptyTc.size(), 0u);

    TextCompare tc({"Hello world", "Hello C++"});
    EXPECT_FALSE(tc.empty());
    ASSERT_EQ(tc.size(), 2u);
    EXPECT_EQ(tc.textAt(0), "Hello world");
    EXPECT_EQ(tc.textAt(1), "Hello C++");
}

TEST(TextCompareTest, ComputesJaccardSimilarityWithTolerance)
{
    EXPECT_NEAR(TextCompare::similarityFromStrings("a b c", "b c d"), 0.5, 1e-12);
    EXPECT_NEAR(TextCompare::similarityFromStrings("a b c", "b c"), 2.0 / 3.0, 1e-12);
    EXPECT_NEAR(TextCompare::similarityFromStrings("", ""), 1.0, 1e-12);
}

TEST(TextCompareTest, ComputesCommonWordsAndLineDifferences)
{
    TextCompare tc({
        "Бібліотека C++\nДругий рядок\nТретій рядок",
        "БІБЛІОТЕКА C++\nЗмінений рядок\nТретій рядок"
    });

    auto common = tc.commonWords(0, 1);
    ASSERT_EQ(common.size(), 4u);
    EXPECT_EQ(common[0], "c");
    EXPECT_EQ(common[1], "бібліотека");
    EXPECT_EQ(common[2], "рядок");

    EXPECT_EQ(tc.differenceCount(0, 1), 4u);
}

TEST(TextCompareTest, ComparesFilesOnDiskCorrectly)
{
    const fs::path p1 = fs::temp_directory_path() / "tc_test_1.txt";
    const fs::path p2 = fs::temp_directory_path() / "tc_test_2.txt";

    {
        std::ofstream f1(p1);
        f1 << "Line 1\nLine 2\nLine 3\n";
        std::ofstream f2(p2);
        f2 << "Line 1\nModified Line 2\nLine 3\n";
    }

    TextCompare tc;
    FileComparisonResult res = tc.compareFiles(p1.string(), p2.string());

    EXPECT_FALSE(res.identical);
    EXPECT_EQ(res.lines1, 3u);
    EXPECT_EQ(res.lines2, 3u);
    EXPECT_EQ(res.firstDifferenceLine, 2u);
    EXPECT_EQ(res.differenceCount, 2u);

    fs::remove(p1);
    fs::remove(p2);
}

TEST(TextCompareTest, RejectsEmptyFilePath)
{
    TextCompare tc;
    EXPECT_THROW(tc.addFile(""), std::invalid_argument);
    EXPECT_THROW(tc.compareFiles("", "some.txt"), std::invalid_argument);
}

TEST(TextCompareTest, ThrowsFileAccessExceptionOnMissingFileOrDirectory)
{
    TextCompare tc;
    EXPECT_THROW(tc.addFile("definitely_missing_file_12345.txt"), FileAccessException);
    EXPECT_THROW(
        tc.compareFiles(fs::temp_directory_path().string(), "other.txt"),
        FileAccessException);
}

TEST(TextCompareTest, ThrowsOutOfRangeOnInvalidTextIndex)
{
    TextCompare tc({"Single text"});
    EXPECT_THROW(tc.textAt(1), std::out_of_range);
    EXPECT_THROW(tc.similarity(0, 5), std::out_of_range);
    EXPECT_THROW(tc.commonWords(2, 0), std::out_of_range);
    EXPECT_THROW(tc.differenceCount(0, 1), std::out_of_range);
}

TEST(TextCompareTest, RejectsEmbeddedNulCharacter)
{
    std::string badText = "abc";
    badText.push_back('\0');
    badText += "def";

    TextCompare tc;
    EXPECT_THROW(tc.addText(badText), InvalidTextParameterException);
    EXPECT_THROW(TextCompare::similarityFromStrings(badText, "abc"), InvalidTextParameterException);
}

TEST(TextCompareTest, ThrowsTextSizeLimitExceptionWhenLimitsExceeded)
{
    std::vector<std::string> tooMany(TextCompare::MAX_TEXTS + 1, "item");
    EXPECT_THROW(TextCompare tc(tooMany), TextSizeLimitException);

    std::string manyLines1(5500, '\n');
    std::string manyLines2(5500, '\n');
    EXPECT_THROW(
        TextCompare::differenceCountFromStrings(manyLines1, manyLines2),
        TextSizeLimitException);
}

TEST(TextCompareTest, ReplaceTextsPreservesStateOnException)
{
    TextCompare tc({"Original doc 1", "Original doc 2"});

    std::string invalidDoc = "broken";
    invalidDoc.push_back('\0');

    std::vector<std::string> invalidUpdate = {"New doc 1", invalidDoc};

    EXPECT_THROW(tc.replaceTexts(invalidUpdate), InvalidTextParameterException);

    ASSERT_EQ(tc.size(), 2u);
    EXPECT_EQ(tc.textAt(0), "Original doc 1");
    EXPECT_EQ(tc.textAt(1), "Original doc 2");
}

TEST(TextCompareTest, DISABLED_UnsafeReplaceTextsViolatesStrongGuarantee)
{
    TextCompare tc({"Original doc 1", "Original doc 2"});

    std::string invalidDoc = "broken";
    invalidDoc.push_back('\0');

    std::vector<std::string> invalidUpdate = {"New doc 1", invalidDoc};

    EXPECT_THROW(tc.replaceTextsUnsafe(invalidUpdate), InvalidTextParameterException);

    ASSERT_EQ(tc.size(), 2u);
    EXPECT_EQ(tc.textAt(0), "Original doc 1");
    EXPECT_EQ(tc.textAt(1), "Original doc 2");
}
