#ifndef CONST_STRING_LITERAL_H
#define CONST_STRING_LITERAL_H

#include <array>
#include <utility>

namespace literal {
    template<char ... chars>
    class string;

    namespace {
        template<char ... chars>
        using string_sequence = std::integer_sequence<char, chars...>;

        template<typename...>
        struct _string_concat;

        template<char ... left>
        struct _string_concat<string<left...>> {
            using type = string<left...>;
        };

        template<char ... left, char ... right, typename ... others>
        struct _string_concat<string<left...>, string<right...>, others...> {
            using type = typename _string_concat<string<left..., right...>, others...>::type;
        };

        template<size_t start, char ... chars, size_t ... indices>
        constexpr auto _string_substr(string<chars...> source, std::index_sequence<indices...>) {
            constexpr std::array<char, sizeof...(chars)> charArray = {chars...};
            return string(string_sequence<std::get<start + indices>(charArray)...>{});
        }

        template<typename...>
        struct _string_compare;

        template<>
        struct _string_compare<string<>, string<>> {
            static constexpr bool equal = true;
            static constexpr bool less_than = false;
        };

        template<char left, char ... left_more>
        struct _string_compare<string<left, left_more...>, string<>> {
            static constexpr bool equal = false;
            static constexpr bool less_than = false;
        };

        template<char right, char ... right_more>
        struct _string_compare<string<>, string<right, right_more...>> {
            static constexpr bool equal = false;
            static constexpr bool less_than = true;
        };

        template<char left, char ... left_more, char right, char ... right_more>
        struct _string_compare<string<left, left_more...>, string<right, right_more...>> {
            static constexpr bool equal = left == right && _string_compare<string<left_more...>, string<right_more...>>::equal;
            static constexpr bool less_than = left < right || left <= right && _string_compare<string<left_more...>, string<right_more...>>::less_than;
        };
    }

    template<char ... chars>
    class string {
    public:
        constexpr string(string_sequence<chars...>) {} // NOLINT(google-explicit-constructor)

        constexpr auto length() {
            return sizeof...(chars);
        }

        operator const char *() { // NOLINT(google-explicit-constructor)
            static constexpr char string[sizeof...(chars) + 1] = {chars..., '\0'};
            return string;
        }

        auto value() {
            return operator const char *();
        }

        template<char ... others>
        constexpr auto concat(string<others...> other) {
            return string(string_sequence<chars..., others...>{});
        }

        template<size_t index>
        constexpr auto after() {
            static_assert(index <= sizeof...(chars), "<index> must be less than or equal to {string.length}");
            return _string_substr<index>(*this, std::make_index_sequence<sizeof...(chars) - index>{});
        }

        template<size_t index>
        constexpr auto before() {
            static_assert(index <= sizeof...(chars), "<index> must be less than or equal to {string.length}");
            return _string_substr<0>(*this, std::make_index_sequence<index>{});
        }

        template<size_t start, size_t length>
        constexpr auto substr() {
            static_assert(start + length <= sizeof...(chars), "<start> + <length> must be less than or equal to {string.length}");
            return _string_substr<start>(*this, std::make_index_sequence<length>{});
        }

        template<size_t start, size_t last>
        constexpr auto substring() {
            static_assert(last <= sizeof...(chars), "<last> must be less than or equal to {string.length}");
            static_assert(last >= start, "<last> must be greater than or equal <start>");
            return _string_substr<start>(*this, std::make_index_sequence<last - start>{});
        }

        template<char ... other>
        constexpr bool operator==(string<other...> _other) {
            return _string_compare<string<chars...>, string<other...>>::equal;
        }

        template<char ... other>
        constexpr bool operator!=(string<other...> _other) {
            return !_string_compare<string<chars...>, string<other...>>::equal;
        }

        template<char ... other>
        constexpr bool operator<(string<other...> _other) {
            return _string_compare<string<chars...>, string<other...>>::less_than;
        }

        template<char ... other>
        constexpr bool operator<=(string<other...> _other) {
            return _string_compare<string<chars...>, string<other...>>::less_than || _string_compare<string<chars...>, string<other...>>::equal;
        }

        template<char ... other>
        constexpr bool operator>=(string<other...> _other) {
            return !_string_compare<string<chars...>, string<other...>>::less_than;
        }

        template<char ... other>
        constexpr bool operator>(string<other...> _other) {
            return !_string_compare<string<chars...>, string<other...>>::less_than && !_string_compare<string<chars...>, string<other...>>::equal;
        }
    };

    template<typename ... params>
    constexpr auto string_concat(params ...) {
        return typename _string_concat<params...>::type({});
    }

    template<typename T, T... chars>
    constexpr string<chars...> operator "" _string() { return string<chars...>({}); }
}

#endif //CONST_STRING_LITERAL_H
