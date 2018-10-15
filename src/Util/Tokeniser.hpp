#pragma once

#include "Logging.hpp"

#include <utility>

#if __has_include(<string_view>)
#include <string_view>
using string_view = std::string_view;
#else
#include <experimental/string_view>
using string_view = std::experimental::string_view;
#endif

namespace Util
{

template<typename Source, typename Token, char Delimiter>
class Tokeniser
{
public:
    template<typename... Args>
    explicit Tokeniser(Args&& ...data)
        : m_data(std::forward<Args>(data)...)
    { }

    Tokeniser(const Tokeniser& copy) = default;
    Tokeniser(Tokeniser&& move) noexcept = default;
    ~Tokeniser() = default;

    Tokeniser& operator=(const Tokeniser& copy) = default;
    Tokeniser& operator=(Tokeniser&& move) noexcept = default;

    bool operator==(const Tokeniser& rhs) const {
        return this == &rhs || m_data == rhs.m_data;
    }
    bool operator!=(const Tokeniser& rhs) const {
        return !(*this == rhs);
    }

    struct ConstIterator : public std::iterator<std::forward_iterator_tag,
                                           Token,
                                           std::ptrdiff_t,
                                           Token*,
                                           Token&>
    {
        explicit ConstIterator(const Source& data)
            : m_data(data)
        {
            moveToNext();
        }
        explicit ConstIterator(Source&& data)
            : m_data(std::move(data))
        {
            moveToNext();
        }
        ConstIterator(const ConstIterator& copy) = default;
        ConstIterator(ConstIterator&& move) noexcept = default;
        ~ConstIterator() = default;

        ConstIterator& operator=(const ConstIterator& copy) = default;
        ConstIterator& operator=(ConstIterator&& move) noexcept = default;

        void moveToNext()
        {
            auto search = m_data.find_first_of(Delimiter);
            if (search != string_view::npos)
            {
                m_token = m_data.substr(0, search);
                m_data.remove_prefix(std::min(search + 1, m_data.size()));
            }
            else
            {
                m_token = m_data;
                m_data.remove_prefix(m_data.size());
            }
        }

        ConstIterator& operator++()
        {
            moveToNext();
            return *this;
        }

        ConstIterator operator++(int)
        {
            auto copy(*this);
            moveToNext();
            return copy;
        }

        const Token& operator*() const {
            return m_token;
        }
        Token& operator*() {
            return m_token;
        }
        const Token* operator->() const {
            return &m_token;
        }
        Token* operator->() {
            return &m_token;
        }

        bool operator==(const ConstIterator& rhs) const
        {
            return this == &rhs || (m_data == rhs.m_data && m_token == rhs.m_token);
        }

        bool operator!=(const ConstIterator& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        Source m_data;
        Token m_token;
    };

    typedef ConstIterator iterator_type;
    typedef ConstIterator const_iterator_type;

    ConstIterator cbegin() { return ConstIterator(m_data); }
    ConstIterator begin() { return cbegin(); }
    ConstIterator cend() {
        auto end = m_data;
        end.remove_prefix(m_data.size());
        return ConstIterator(end);
    }
    ConstIterator end() { return cend(); }

    bool empty() const { return m_data.empty(); }

private:
    Source m_data;
};

using LineTokeniser = Tokeniser<string_view, string_view, '\n'>;
using CommaTokeniser = Tokeniser<string_view, string_view, ','>;
using SpaceTokeniser = Tokeniser<string_view, string_view, ' '>;

}
