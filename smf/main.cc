#include <cstdio>
#include <cstring>
#include <array>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>

namespace {
    constexpr std::size_t SMF_NIDENT = 4;

    template<typename T, typename = std::is_integral<T>>
    T exchange_order(T from) {
        std::array<std::uint8_t, sizeof(T)> buf;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            buf[i] = (from >> (sizeof(T) - i - 1) * 8) & 0xFF;
        }
        T result;
        std::memcpy(&result, buf.data(), buf.size());
        return result;
    }
}

struct SMF_Header {
    std::uint8_t chunk_type[SMF_NIDENT];
    std::uint32_t data_length;
    std::uint16_t format;
    std::uint16_t n_track;
    std::uint16_t time_unit;

    std::string get_chunk_type() {
        return std::string(reinterpret_cast<char *>(chunk_type));
    }

    std::uint32_t get_data_length() {
        return exchange_order(data_length);
    }

    std::uint16_t get_format() {
        return exchange_order(format);
    }

    std::uint16_t get_n_track() {
        return exchange_order(n_track);
    }

    std::uint16_t get_time_unit() {
        return exchange_order(time_unit);
    }
} __attribute__((packed));

struct SMF_Event {
    int delta_time;
};

struct SMF_MidiEvent: public SMF_Event {
};

struct SMF_MetaEvent: public SMF_Event {
};

struct SMF_EventBuffer {
    std::uint8_t *buffer;
    std::size_t length;
    std::size_t cursor;

    static SMF_EventBuffer from_file(std::FILE *f, std::uint32_t length) {
        SMF_EventBuffer events;
        events.length = length;
        events.buffer = new std::uint8_t[static_cast<std::size_t>(length)];
        events.cursor = 0;
        std::fread(events.buffer, 1, events.length, f);
        return events;
    }

    int parse_delta_time(std::uint8_t *data) {
        int result = 0;
        while ((data[cursor] & 0x80) != 0) {
            result <<= 7;
            result |= data[cursor] & 0x7f;
            cursor++;
        }
        result <<= 7;
        result |= data[cursor] & 0x7f;
        cursor++;
        return result;
    }
};

struct SMF_Track {
    std::uint8_t chunk_type[SMF_NIDENT];
    std::uint32_t data_length;

    std::string get_chunk_type() {
        return std::string(reinterpret_cast<char *>(chunk_type));
    }

    std::uint32_t get_data_length() {
        return exchange_order(data_length);
    }
} __attribute__((packed));

int main() {
    std::FILE *f = std::fopen("teaparty.mid", "rb");
    if (!f) {
        std::cout << "File not found.\n";
        return 1;
    }
    SMF_Header hdr;
    std::fread(&hdr, sizeof(SMF_Header), 1, f);
    std::cout << "Type:\t\t" << hdr.get_chunk_type() << '\n';
    std::cout << "Length:\t\t" <<hdr.get_data_length() << '\n';
    std::cout << "Format:\t\t" << hdr.get_format() << '\n';
    std::cout << "Track:\t\t" << hdr.get_n_track() << '\n';
    std::cout << "Time Unit:\t" << hdr.get_time_unit() << '\n';

    for (std::uint16_t i = 0; i < hdr.get_n_track(); ++i) {
        std::cout << '\n';

        SMF_Track trk;
        std::fread(&trk, sizeof(SMF_Track), 1, f);
        std::cout << "Type:\t"<< trk.get_chunk_type() << '\n';
        std::cout << "Lenght:\t" << trk.get_data_length() << '\n';
        SMF_EventBuffer events = SMF_EventBuffer::from_file(f, trk.get_data_length());
        (void)events;
    }

    std::fclose(f);
}
