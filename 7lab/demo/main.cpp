#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include "TextCompare.h"
#include "TextCompareExceptions.h"
#include "ScopeLogger.h"

void level3_updateStorage(TextCompare& comparator)
{
    ScopeLogger logger("level3_updateStorage");
    std::cout << "Preparing invalid batch with embedded NUL byte...\n";

    std::string corruptedText = "Valid prefix";
    corruptedText.push_back('\0');
    corruptedText += "hidden tail";

    std::vector<std::string> badBatch = {
        "First valid document",
        corruptedText
    };

    comparator.replaceTexts(badBatch);
}

void level2_processBatch(TextCompare& comparator)
{
    ScopeLogger logger("level2_processBatch");
    try
    {
        level3_updateStorage(comparator);
    }
    catch (const std::exception& ex)
    {
        std::cout << "[LOG] level2_processBatch caught: " << ex.what() << '\n';
        std::cout << "[LOG] Rethrowing via 'throw;' to preserve dynamic type...\n";
        throw;
    }
}

void level1_pipeline(TextCompare& comparator)
{
    ScopeLogger logger("level1_pipeline");
    level2_processBatch(comparator);
}

int main()
{
    std::cout << "=== 1. Normal TextCompare Operations ===\n";
    TextCompare tc({
        "C++ exceptions ensure reliable error handling\nRAII manages resources automatically",
        "C++ exceptions ensure reliable error handling\nStrong guarantee protects object invariants"
    });

    std::cout << "Stored documents: " << tc.size() << '\n';
    std::cout << "Similarity (Jaccard): " << tc.similarity(0, 1) << '\n';
    std::cout << "Difference count (lines): " << tc.differenceCount(0, 1) << '\n';
    std::cout << "Common words: ";
    for (const auto& w : tc.commonWords(0, 1))
    {
        std::cout << w << ' ';
    }
    std::cout << "\n\n";

    std::cout << "=== 2. Stack Unwinding, RAII & Strong Guarantee Demo ===\n";
    std::cout << "Documents count before failed replaceTexts: " << tc.size() << '\n';
    std::cout << "First document before: " << tc.textAt(0).substr(0, 25) << "...\n";

    try
    {
        level1_pipeline(tc);
    }
    catch (const FileAccessException& ex)
    {
        std::cout << "[MAIN] FileAccessException: " << ex.what()
                  << " | path=" << ex.filePath() << '\n';
    }
    catch (const TextSizeLimitException& ex)
    {
        std::cout << "[MAIN] TextSizeLimitException: " << ex.what()
                  << " | max=" << ex.maxAllowed() << ", actual=" << ex.actualSize() << '\n';
    }
    catch (const InvalidTextParameterException& ex)
    {
        std::cout << "[MAIN] InvalidTextParameterException: " << ex.what()
                  << " | param=" << ex.parameterName() << '\n';
    }
    catch (const TextCompareException& ex)
    {
        std::cout << "[MAIN] TextCompareException: " << ex.what() << '\n';
    }
    catch (const std::exception& ex)
    {
        std::cout << "[MAIN] std::exception: " << ex.what() << '\n';
    }

    std::cout << "Documents count after failed replaceTexts: " << tc.size() << '\n';
    std::cout << "First document after (Strong Guarantee): " << tc.textAt(0).substr(0, 25) << "...\n\n";

    std::cout << "=== 3. File Access Exception Demo ===\n";
    try
    {
        tc.compareFiles("non_existent_file_1.txt", "non_existent_file_2.txt");
    }
    catch (const FileAccessException& ex)
    {
        std::cout << "Caught FileAccessException: " << ex.what() << '\n';
    }

    std::cout << "\nProgram finished normally.\n";
    return 0;
}
