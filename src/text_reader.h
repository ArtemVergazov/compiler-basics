#pragma once

#include <cstddef>
#include <optional>
#include <utility>

template <typename TextStream>
class TextReader {
public:
    explicit TextReader(TextStream &&textStream) : mTextStream(std::move(textStream)) {}

protected:
    [[nodiscard]] std::optional<typename TextStream::value_type> peek(int ahead = 1) const {
        if (mIndex + ahead > mTextStream.size()) {
            return {};
        }

        return mTextStream[mIndex];
    }

    TextStream::value_type consume() { return mTextStream[mIndex++]; }

    const TextStream mTextStream{};
    std::size_t mIndex = 0;
};
