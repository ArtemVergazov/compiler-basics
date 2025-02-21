#pragma once

#include <cstddef>
#include <optional>
#include <utility>

template <typename TextStream>
class TextReader {
public:
    explicit TextReader(TextStream &&textStream) : mTextStream(std::move(textStream)) {}

protected:
    [[nodiscard]] std::optional<typename TextStream::value_type> peek(int offset = 0) const {
        if (mIndex + offset >= mTextStream.size()) {
            return {};
        }

        return mTextStream[mIndex + offset];
    }

    TextStream::value_type consume() { return mTextStream[mIndex++]; }

    const TextStream mTextStream{};
    std::size_t mIndex = 0;
};
