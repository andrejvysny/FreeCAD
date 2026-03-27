// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#ifndef FC_SOURCE_DIR
# error FC_SOURCE_DIR must be defined by CMake
#endif

namespace
{

bool isComponentLibSource(const std::filesystem::path& path)
{
    const auto extension = path.extension().string();
    return extension == ".h" || extension == ".hpp" || extension == ".cpp";
}

std::vector<std::string> findForbiddenIncludes()
{
    const std::filesystem::path root = std::filesystem::path(FC_SOURCE_DIR)
        / "src"
        / "Libs"
        / "FCComponentLib";
    const std::regex forbidden(R"(^\s*#\s*include\s*[<\"]\s*(App/|Base/|Gui/|Inventor/).*)");

    std::vector<std::string> violations;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file() || !isComponentLibSource(entry.path())) {
            continue;
        }

        std::ifstream stream(entry.path());
        if (!stream.good()) {
            continue;
        }

        std::string line;
        int lineNumber = 0;
        while (std::getline(stream, line)) {
            ++lineNumber;
            if (std::regex_match(line, forbidden)) {
                violations.emplace_back(
                    entry.path().string() + ":" + std::to_string(lineNumber) + ": " + line
                );
            }
        }
    }

    return violations;
}

}

TEST(ForbiddenIncludes, FCComponentLibStaysPureQt)
{
    const auto violations = findForbiddenIncludes();
    EXPECT_TRUE(violations.empty()) << testing::PrintToString(violations);
}
