/**
 * @file png.hpp
 * @brief Writing 32-bit true-color PNG files.
 */

#ifndef RAYTRACER_PNG_HPP
#define RAYTRACER_PNG_HPP

#include <zlib.h>
#include <fmt/format.h>

#include <array>
#include <bit>
#include <cstdint>
#include <ostream>
#include <span>
#include <vector>

namespace png
{
    namespace detail
    {
        /// Converts a 4-character ASCII string, plus null terminator, into an std::array of 4 bytes.
        template<std::size_t N>
        constexpr std::array<std::uint8_t, 4> to_type(const char (&string)[N])
        {
            static_assert(N == 5, "`string` must be 4 characters followed by a null terminator.");

            std::array<std::uint8_t, 4> type = { };
            std::copy_n(string, type.size(), type.data());
            return type;
        }
    }

    /// The PNG file signature, as defined by
    /// [the PNG specification](http://www.libpng.org/pub/png/spec/1.2/PNG-Rationale.html#R.PNG-file-signature).
    constexpr std::array<std::uint8_t, 8> signature = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    /**
     * @brief An [IHDR](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IHDR) chunk.
     */
    struct header
    {
        /// The size of the packed data.
        static constexpr std::uint32_t size = 13;

        /// The 4-byte chunk type.
        static constexpr std::array type = detail::to_type("IHDR");

        /// The width of the image in pixels.
        std::uint32_t width;
        /// The height of the image in pixels.
        std::uint32_t height;
        /// The bit depth of the image.
        std::uint8_t  bit_depth;
        /// The color type of the image.
        std::uint8_t  color_type;
        /// The compression method used in the IDAT chunk.
        std::uint8_t  compression_method;
        /// The filter method used in the IDAT chunk.
        std::uint8_t  filter_method;
        /// The interlace method used in the IDAT chunk.
        std::uint8_t  interlace_method;

        /**
         * @brief The length of the packed data.
         * @note This function only exists to provide a uniform interface across all chunk types. It is equivalent to
         *       png::header::size.
         */
        constexpr std::uint32_t length() const
        {
            return header::size;
        }

        void write_data(std::ostream &os) const
        {
            std::uint32_t width = this->width;
            if constexpr (std::endian::native == std::endian::little)
            {
                width = std::byteswap(width);
            }

            std::uint32_t height = this->height;
            if constexpr (std::endian::native == std::endian::little)
            {
                height = std::byteswap(height);
            }

            os.write(reinterpret_cast<const char *>(&width), sizeof(width));
            os.write(reinterpret_cast<const char *>(&height), sizeof(height));
            os.put(static_cast<char>(bit_depth));
            os.put(static_cast<char>(color_type));
            os.put(static_cast<char>(compression_method));
            os.put(static_cast<char>(filter_method));
            os.put(static_cast<char>(interlace_method));
        }

        std::uint32_t hash_data(std::uint32_t crc) const
        {
            std::uint32_t width = this->width;
            if constexpr (std::endian::native == std::endian::little)
            {
                width = std::byteswap(width);
            }

            std::uint32_t height = this->height;
            if constexpr (std::endian::native == std::endian::little)
            {
                height = std::byteswap(height);
            }

            crc = crc32(crc, reinterpret_cast<const Bytef *>(&width), sizeof(width));
            crc = crc32(crc, reinterpret_cast<const Bytef *>(&height), sizeof(height));
            crc = crc32(crc, &bit_depth, sizeof(bit_depth));
            crc = crc32(crc, &color_type, sizeof(color_type));
            crc = crc32(crc, &compression_method, sizeof(compression_method));
            crc = crc32(crc, &filter_method, sizeof(filter_method));
            crc = crc32(crc, &interlace_method, sizeof(interlace_method));

            return crc;
        }
    };

    /**
     * @brief An [IDAT](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IDAT) chunk.
     */
    struct data
    {
        /// The 4-byte chunk type.
        static constexpr std::array type = detail::to_type("IDAT");

        /// The compressed image data.
        std::vector<std::uint8_t> bytes;

        /**
         * @brief Constructs a new png::data object by compressing the provided raw image bytes.
         * @param[in] width The width of the image.
         * @param[in] height The height of the image.
         * @param[in] raw_bytes The raw RGBA bytes.
         * @throws std::runtime_error Thrown if the compression process fails.
         */
        explicit data(std::size_t width, std::size_t height, std::span<const std::uint8_t> raw_bytes)
            : bytes(compressBound(static_cast<uInt>(raw_bytes.size())))
        {
            z_stream stream = {
                .next_out  = bytes.data(),
                .avail_out = static_cast<uInt>(bytes.size()),
            };

            if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK)
            {
                throw std::runtime_error(
                    fmt::format("failed on deflateInit() with error: {}", stream.msg)
                );
            }

            const std::size_t stride = width * 4;
            for (std::size_t row = 0; row < height; ++row)
            {
                std::uint8_t filter = 0;
                stream.next_in = &filter;
                stream.avail_in = sizeof(filter);
                if (deflate(&stream, Z_NO_FLUSH) != Z_OK)
                {
                    throw std::runtime_error(
                        fmt::format("failed on deflate(Z_NO_FLUSH) with error: {}", stream.msg)
                    );
                }

                const auto bytes = raw_bytes.subspan(row * stride, stride);
                stream.next_in = const_cast<Bytef *>(bytes.data());
                stream.avail_in = static_cast<uInt>(bytes.size());
                if (deflate(&stream, Z_NO_FLUSH) != Z_OK)
                {
                    throw std::runtime_error(
                        fmt::format("failed on deflate(Z_NO_FLUSH) with error: {}", stream.msg)
                    );
                }
            }

            if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
            {
                throw std::runtime_error(
                    fmt::format("failed on deflate(Z_FINISH) with error: {}", stream.msg)
                );
            }

            if (deflateEnd(&stream) != Z_OK)
            {
                throw std::runtime_error(
                    fmt::format("failed on deflateEnd() with error: {}", stream.msg)
                );
            }
        }

        /// The length of the compressed data.
        std::uint32_t length() const
        {
            return static_cast<std::uint32_t>(bytes.size());
        }

        void write_data(std::ostream &os) const
        {
            os.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        }

        std::uint32_t hash_data(std::uint32_t crc) const
        {
            return crc32(crc, bytes.data(), static_cast<uInt>(bytes.size()));
        }
    };

    /**
     * @brief An [IEND](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IEND) chunk.
     */
    struct end_type
    {
        /// The 4-byte chunk type.
        static constexpr std::array type = detail::to_type("IEND");

        /// The length of the data. As the IEND chunk is always empty, this returns 0.
        constexpr std::uint32_t length() const
        {
            return 0;
        }

        void write_data([[maybe_unused]] std::ostream &os) const
        {
        }

        std::uint32_t hash_data(std::uint32_t crc) const
        {
            return crc;
        }
    };

    /// An instance of the png::end_type serving to avoid having to construct a new png::end_type every time an IEND
    /// chunk is needed.
    constexpr png::end_type end;

    /**
     * @brief Calculates the 32-bit CRC for a given chunk.
     * @tparam T The chunk type. This should usually not be provided explicitly, and should instead be deduced.
     * @param[in] chunk The chunk to calculate the CRC for.
     * @return A 32-bit CRC.
     */
    template<typename T>
    std::uint32_t calculate_chunk_crc(const T &chunk)
    {
        std::uint32_t crc = crc32(0, nullptr, 0);
        crc = crc32(crc, T::type.data(), static_cast<uInt>(T::type.size()));
        crc = chunk.hash_data(crc);
        return crc;
    }

    /**
     * @brief Writes a chunk to the provided std::ostream.
     * @tparam T The chunk type. This should usually not be provided explicitly, and should instead be deduced.
     * @param[in] chunk The chunk to be written.
     * @param[in] os The output stream.
     */
    template<typename T>
    void write_chunk(const T &chunk, std::ostream &os)
    {
        std::uint32_t length = chunk.length();
        if constexpr (std::endian::native == std::endian::little)
        {
            length = std::byteswap(length);
        }

        os.write(reinterpret_cast<const char *>(&length), sizeof(length));
        os.write(reinterpret_cast<const char *>(T::type.data()), T::type.size());
        chunk.write_data(os);

        std::uint32_t crc = calculate_chunk_crc(chunk);
        if constexpr (std::endian::native == std::endian::little)
        {
            crc = std::byteswap(crc);
        }

        os.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
    }

    /**
     * @brief A 32-bit true-color PNG image.
     */
    struct image
    {
        /// The raw RGBA bytes of the image.
        std::vector<std::uint8_t> raw_bytes;
        /// The width of the image.
        std::uint32_t             width;
        /// The height of the image.
        std::uint32_t             height;

        /**
         * @brief Calculates the uncompressed size in bytes of a PNG image with a given width and height.
         * @param[in] width The width of the image.
         * @param[in] height The height of the image.
         * @return The size of the uncompressed image in bytes.
         */
        static constexpr std::size_t uncompressed_size(std::uint32_t width, std::uint32_t height)
        {
            return width * (32 / 8) * height;
        }

        void write_to(std::ostream &os) const
        {
            const png::header header = {
                .width              = width,
                .height             = height,
                .bit_depth          = 8,
                .color_type         = 6,
                .compression_method = 0,
                .filter_method      = 0,
                .interlace_method   = 0,
            };

            const png::data data(width, height, raw_bytes);

            os.write(reinterpret_cast<const char *>(png::signature.data()), png::signature.size());
            write_chunk(header, os);
            write_chunk(data, os);
            write_chunk(end, os);
        }
    };
}

#endif // !RAYTRACER_PNG_HPP
