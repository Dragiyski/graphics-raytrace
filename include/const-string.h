#ifndef STRING_HELPER_H
#define STRING_HELPER_H

namespace const_string {
    namespace {
        template<char ... chars>
        using string_sequence = std::integer_sequence<char, chars...>;

        template<typename>
        struct string_struct;

        template<char ... chars>
        struct string_struct<string_sequence<chars...>> {
            static constexpr auto length = sizeof...(chars);

            static const char *to_string() {
                static constexpr char string[sizeof...(chars) + 1] = {chars..., '\0'};
                return string;
            }
        };

        template<typename...>
        struct string_concat;

        template<char ... left>
        struct string_concat<string_sequence<left...>> {
            using type = string_sequence<left...>;
        };

        template<char ... left, char ... right, typename ... others>
        struct string_concat<string_sequence<left...>, string_sequence<right...>, others...> {
            using type = typename string_concat<string_sequence<left..., right...>, others...>::type;
        };

        template<size_t start, char ... chars, size_t ... indices>
        constexpr auto string_substr(string_sequence<chars...> string, std::index_sequence<indices...>) {
            constexpr std::array<char, sizeof...(chars)> charArray = {chars...};
            return string_sequence<std::get<start + indices>(charArray)...>{};
        }
    }

    template<typename T, T... chars>
    constexpr string_sequence<chars...> operator "" _const() { return {}; }

    template<char ... chars>
    constexpr auto length(string_sequence<chars...> string) {
        return string_struct<decltype(string)>::length;
    }

    template<char ... chars>
    constexpr const char *to_string(string_sequence<chars...> string) {
        return string_struct<decltype(string)>::to_string();
    }

    template<typename ... params>
    constexpr typename string_concat<params...>::type concat(params ...) {
        return {};
    }

    template<size_t start, char ... chars>
    constexpr auto substr(string_sequence<chars...> string) {
        return string_substr<start>(string, std::make_index_sequence<sizeof...(chars) - start>{});
    }
};


#endif //NODE_SDL2_STRING_HELPER_H
