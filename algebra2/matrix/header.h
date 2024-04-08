#include <algorithm>    // Standard library header for algorithms
#include <cstdio>       // Standard I/O operations
#include <expected>     // Expected utilities
#include <filesystem>   // Filesystem operations
#include <format>       // Formatting utilities
#include <iomanip>      // Input/output manipulators
#include <iostream>     // Standard I/O streams
#include <mdspan>       // Multidimensional array span
#include <ranges>       // Range-based utilities
#include <sstream>      // String stream
#include <string_view>  // String view
#include <fstream>      // File operations (added for file writing)
#include <span>         // Span (added for span support)
#include <print>        // Print utilities (needed for writing out)

namespace ranges {

    // Define a custom view for numeric operations on ranges
    template <std::ranges::viewable_range R>
    class numeric_view
        : public std::ranges::subrange<std::ranges::iterator_t<R>,
        std::ranges::sentinel_t<R>> {

    private:
        using value_type = std::ranges::range_value_t<R>;
        using base_type =
            std::ranges::subrange<std::ranges::iterator_t<R>,
            std::ranges::sentinel_t<R>>;

    public:
        // Constructors
        constexpr numeric_view(R&& v) : base_type{ v } {}
        constexpr numeric_view(R& v) : base_type{ v } {}

        // Coordinate-wise addition
        template <std::ranges::viewable_range _R>
            requires(std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator+=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) {
                a += b;
            }
            return *this;
        }

        // Coordinate-wise subtraction
        template <std::ranges::viewable_range _R>
            requires(std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator-=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) {
                a -= b;
            }
            return *this;
        }

        // Coordinate-wise multiplication
        template <std::ranges::viewable_range _R>
            requires(std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator*=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) {
                a *= b;
            }
            return *this;
        }

        // Coordinate-wise division
        template <std::ranges::viewable_range _R>
            requires(std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator/=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) {
                a /= b;
            }
            return *this;
        }

        // Scalar addition to each coordinate
        constexpr auto& operator+=(std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(),
                [scalar](auto val) { return val + scalar; });
            return *this;
        }

        // Scalar subtraction from each coordinate
        constexpr auto& operator-=(std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(),
                [scalar](auto val) { return val - scalar; });
            return *this;
        }

        // Scalar multiplication to each coordinate
        constexpr auto& operator*=(std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(),
                [scalar](auto val) { return val * scalar; });
            return *this;
        }

        // Scalar division from each coordinate
        constexpr auto& operator/=(std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(),
                [scalar](auto val) { return val / scalar; });
            return *this;
        }
    };

}  // namespace ranges

namespace ranges {

    // Define a custom view for matrix operations
    template <typename T, typename LP>
        requires(std::same_as<LP, std::layout_right> ||
    std::same_as<LP, std::layout_left>) &&
        (true)
        class matrix_view : public std::mdspan<T,
        std::dextents<std::size_t, 2>,
        LP,
        std::default_accessor<T>>,
        public std::span<T, std::dynamic_extent> {

        private:
            using mdspan_type = std::mdspan<T,
                std::dextents<std::size_t, 2>,
                LP,
                std::default_accessor<T>>;
            using span_type = std::span<T, std::dynamic_extent>;
            using data_handle_type = typename mdspan_type::data_handle_type;

        public:
            // Constructors
            constexpr matrix_view(T* data,
                std::size_t rows,
                std::size_t columns,
                LP layout = std::layout_right{})
                : mdspan_type{ data, rows, columns },
                span_type{ data, rows * columns } {};

            template <std::ranges::contiguous_range R, typename _LP>
            constexpr matrix_view(R& data,
                std::size_t rows,
                std::size_t columns,
                _LP layout)
                : mdspan_type{ data.data(), rows, columns },
                span_type{ data.data(), rows * columns } {};

        public:
            using mdspan_type::extent;

        public:
            using mdspan_type::operator[];

            // Depending on layout, returns ith row or ith column
            constexpr auto operator[](std::size_t i) {
                static constexpr std::size_t extent{
                    std::same_as<LP, std::layout_right> ? 1 : 0 };
                auto dim{ this->extent(extent) };
                return numeric_view{
                    std::ranges::subrange{this->begin() + i * dim,
                                          this->begin() + (i + 1) * dim} };
            }
    };

    // Template deduction guides for matrix_view
    template <typename T, typename LP>
    matrix_view(T*, std::size_t, std::size_t, LP) -> matrix_view<T, LP>;

    template <std::ranges::contiguous_range R, typename _LP>
    matrix_view(R&, std::size_t, std::size_t, _LP)
        -> matrix_view<std::ranges::range_value_t<R>, _LP>;

    // Function to transpose a matrix
    template <typename T, typename LP>
        requires(std::same_as<LP, std::layout_right> ||
    std::same_as<LP, std::layout_left>) &&
        (true)
        inline constexpr auto transpose(matrix_view<T, LP> m) {
        if constexpr (std::same_as<LP, std::layout_right>) {
            return matrix_view{
                m.data(), m.extent(0), m.extent(1), std::layout_left{} };

        }
        else {
            return matrix_view{
                m.data(), m.extent(0), m.extent(1), std::layout_right{} };
        }
    }

}  // namespace ranges

// Function to convert matrix_view to string
template <typename T, typename LP>
auto to_string(::ranges::matrix_view<T, LP> m,
    char column_separator,
    char row_separator) {
    if constexpr (std::same_as<LP, std::layout_right>) {
        std::stringstream out;
        std::size_t max_width = 0;
        for (std::size_t i = 0; i < m.extent(0); ++i) {
            for (std::size_t j = 0; j < m.extent(1); ++j) {
                out << std::setw(7) << m[i][j] << column_separator;
                max_width = std::max(max_width, std::to_string(m[i][j]).size());
            }
            if (row_separator == '\n') {
                out << std::endl;
            }
            else {
                out << row_separator;
            }
        }
        if (row_separator != '\n') {
            // Remove last column separator
            out.seekp(-1, std::ios_base::end);
            out << row_separator;
        }
        return out.str();
    }
    else {
        return ::to_string(::ranges::transpose(m), column_separator, row_separator);
    }
}

// Formatter for matrix_view
template <typename T, typename LP>
struct std::formatter<::ranges::matrix_view<T, LP>, char> {
    char column_separator{ ',' };
    char row_separator{ '\n' };

    // Parsing the format specifier
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto pos = ctx.begin();
        std::size_t i{ 0 };
        while (pos != ctx.end() && *pos != '}') {
            if (i == 0) {
                column_separator = *pos;
            }
            if (i == 1) {
                row_separator = *pos;
            }
            ++i;
            ++pos;
        }
        return pos;  // expect `}` at this position, otherwise, it's error! exception!
    }

    // Formatting the matrix_view
    template <class FmtContext>
    FmtContext::iterator format(::ranges::matrix_view<T, LP> m,
        FmtContext& ctx) const {
        return std::format_to(ctx.out(),
            "{}",
            ::to_string(m, column_separator, row_separator));
    }
};

// Function to save matrix to a file
template <char column_separator = ',',
    char row_separator = '\n',
    typename T,
    typename LP>
[[nodiscard]] inline auto save(ranges::matrix_view<T, LP> m,
    std::filesystem::path file) {
    std::filesystem::create_directories(file.parent_path());

    std::ofstream outfile(file);
    if (outfile.is_open()) {
        outfile << ::to_string(m, column_separator, row_separator);
        std::cout << "Saved matrix to file: " << file << std::endl;
        outfile.close();
    }
    else {
        std::cerr << "Failed to create file " << file << ", reason: " << strerror(errno) << std::endl;
    }
}

// Function to load matrix from a file
template <typename T,
    typename LP,
    char column_separator = ',',
    char row_separator = ';'>
[[nodiscard]] inline auto load(std::filesystem::path)
-> std::pair<std::vector<T>, ranges::matrix_view<T, LP>> {
    std::vector v{ 1, 2, 3, 4, 5, 6 };
    ::ranges::matrix_view m{ v, 2, 3, std::layout_right{} };
    return { std::move(v), std::move(m) };
}
