#pragma once

#include <cstdlib> // size_t
#include <optional>
#include <utility> // std::move

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

    const TextStream mTextStream;
    size_t mIndex{};
};
