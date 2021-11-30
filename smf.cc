#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace {
    constexpr std::size_t SMF_NIDENT = 4;

    template <typename T, typename = std::is_integral<T>>
    T exchange_order(T from) {
        std::array<std::uint8_t, sizeof(T)> buf;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            buf[i] = (from >> (sizeof(T) - i - 1) * 8) & 0xFF;
        }
        T result;
        std::memcpy(&result, buf.data(), buf.size());
        return result;
    }
} // namespace

struct SMF_Header {
    std::uint8_t chunk_type[SMF_NIDENT];
    std::uint32_t data_length;
    std::uint16_t format;
    std::uint16_t n_track;
    std::uint16_t time_unit;

    std::string get_chunk_type() {
        return std::string(reinterpret_cast<char *>(chunk_type));
    }

    std::uint32_t get_data_length() { return exchange_order(data_length); }

    std::uint16_t get_format() { return exchange_order(format); }

    std::uint16_t get_n_track() { return exchange_order(n_track); }

    std::uint16_t get_time_unit() { return exchange_order(time_unit); }
} __attribute__((packed));

struct SMF_Event {
    enum class EventType {
        SMF_MIDI,
        SMF_META,
        SMF_SYSEX,
    };

    EventType smf_event_type;
    int delta_time;

    SMF_Event(EventType type) : smf_event_type(type) {}
};

struct SMF_MidiEvent : public SMF_Event {
    SMF_MidiEvent() : SMF_Event(SMF_Event::EventType::SMF_MIDI) {}

    enum class EventType : std::uint8_t {
        MIDI_NOTE_OFF,
        MIDI_NOTE_ON,
        MIDI_CC,
    };

    EventType type;
    std::uint8_t channel;
    union {
        std::uint8_t note;
        std::uint8_t control;
    };
    union {
        std::uint8_t velocity;
        std::uint8_t data;
    };
};

struct SMF_MetaEvent : public SMF_Event {
    SMF_MetaEvent() : SMF_Event(SMF_Event::EventType::SMF_META) {}

    std::uint8_t type;
    int length;
    std::uint8_t *ptr;
};

struct SMF_SysExEvent : public SMF_Event {
    SMF_SysExEvent() : SMF_Event(SMF_Event::EventType::SMF_SYSEX) {}

    int length;
    std::uint8_t *ptr;
};

struct SMF_EventBuffer {
    std::uint8_t *buffer;
    std::size_t length;
    std::size_t cursor;

    static SMF_EventBuffer *from_buffer(std::uint8_t *data,
                                        std::size_t length) {
        SMF_EventBuffer *events = new SMF_EventBuffer;
        events->length = length;
        events->buffer = new std::uint8_t[length];
        events->cursor = 0;
        std::memcpy(events->buffer, data, length);
        return events;
    }

    std::uint8_t running_status = 0;

    SMF_Event *get_next() {
#define CHECK_BOUNDARY()                      \
    do {                                      \
        if (cursor >= length) return nullptr; \
    } while (false)

        CHECK_BOUNDARY();

        int delta_time = parse_varint();
        CHECK_BOUNDARY();

        if ((buffer[cursor] & 0x80) == 0) {
            if (running_status == 0 || running_status == 0xF0 ||
                running_status == 0xF7 || running_status == 0xFF) {
                return nullptr;
            }
        } else {
            running_status = buffer[cursor];
            cursor++;
            CHECK_BOUNDARY();
        }

#undef CHECK_BOUNDARY
#define CHECK_BOUNDARY(forward)                                     \
    do {                                                            \
        if (cursor + static_cast<std::size_t>(forward) >= length) { \
            delete ev;                                              \
            return nullptr;                                         \
        }                                                           \
    } while (false)
        if (running_status == 0xF0) {
            SMF_SysExEvent *ev = new SMF_SysExEvent;
            ev->delta_time = delta_time;
            ev->length = parse_varint();
            ev->ptr = buffer + cursor;
            cursor += ev->length;
            cursor++;
            return ev;
        } else if (running_status == 0xF7) {
            SMF_SysExEvent *ev = new SMF_SysExEvent;
            ev->delta_time = delta_time;
            ev->length = parse_varint();
            if (ev->length != 0) {
                CHECK_BOUNDARY(ev->length);
                ev->ptr = buffer + cursor;
                cursor += static_cast<std::size_t>(ev->length);
            }
            return ev;
        } else if ((running_status & 0xF0) >> 4 == 0x8) {
            SMF_MidiEvent *ev = new SMF_MidiEvent;
            ev->type = SMF_MidiEvent::EventType::MIDI_NOTE_OFF;
            ev->delta_time = delta_time;
            ev->channel = running_status & 0x0F;
            ev->note = buffer[cursor];
            cursor++;
            CHECK_BOUNDARY(0);
            ev->velocity = buffer[cursor];
            cursor++;
            return ev;
        } else if ((running_status & 0xF0) >> 4 == 0x9) {
            SMF_MidiEvent *ev = new SMF_MidiEvent;
            ev->type = SMF_MidiEvent::EventType::MIDI_NOTE_ON;
            ev->delta_time = delta_time;
            ev->channel = running_status & 0x0F;
            ev->note = buffer[cursor];
            cursor++;
            CHECK_BOUNDARY(0);
            ev->velocity = buffer[cursor];
            cursor++;
            return ev;
        } else if ((running_status & 0xF0) >> 4 == 0xB) {
            SMF_MidiEvent *ev = new SMF_MidiEvent;
            ev->type = SMF_MidiEvent::EventType::MIDI_CC;
            ev->delta_time = delta_time;
            ev->channel = running_status & 0x0F;
            ev->control = buffer[cursor];
            cursor++;
            CHECK_BOUNDARY(0);
            ev->data = buffer[cursor];
            cursor++;
            return ev;
        } else if (running_status == 0xFF) {
            SMF_MetaEvent *ev = new SMF_MetaEvent;
            ev->delta_time = delta_time;
            ev->type = buffer[cursor];
            cursor++;
            CHECK_BOUNDARY(0);
            ev->length = parse_varint();
            if (ev->length != 0) {
                CHECK_BOUNDARY(ev->length - 1);
                ev->ptr = buffer + cursor;
                cursor += static_cast<std::size_t>(ev->length);
            }
            return ev;
        }
#undef CHECK_BOUNDARY

        return nullptr;
    }

private:
    int parse_varint() {
        int result = 0;
        while ((buffer[cursor] & 0x80) != 0) {
            result <<= 7;
            result |= buffer[cursor] & 0x7f;
            cursor++;
        }
        result <<= 7;
        result |= buffer[cursor] & 0x7f;
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

    std::uint32_t get_data_length() { return exchange_order(data_length); }
} __attribute__((packed));

class SMFReader {
    std::vector<std::uint8_t> data;
    std::size_t cursor = 0;

public:
    SMFReader(std::vector<std::uint8_t> &&data) : data(std::move(data)) {}

    SMF_Header *read_header() {
        if (cursor + sizeof(SMF_Header) > data.size()) {
            return nullptr;
        }

        SMF_Header *hdr = new SMF_Header;
        std::memcpy(hdr, data.data() + cursor, sizeof(SMF_Header));
        cursor += sizeof(SMF_Header);
        return hdr;
    }

    SMF_Track *read_track_header() {
        if (cursor + sizeof(SMF_Track) > data.size()) {
            return nullptr;
        }

        SMF_Track *trk = new SMF_Track;
        std::memcpy(trk, data.data() + cursor, sizeof(SMF_Track));
        cursor += sizeof(SMF_Track);
        return trk;
    }

    SMF_EventBuffer *create_event_reader(std::size_t length) {
        if (cursor + length > data.size()) {
            return nullptr;
        }

        std::size_t first = cursor;
        cursor += length;
        return SMF_EventBuffer::from_buffer(data.data() + first, length);
    }
};

std::vector<std::uint8_t> data = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00,
    0x02, 0x01, 0xE0, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x2A,
    0x00, 0xFF, 0x51, 0x03, 0x0F, 0x42, 0x40, 0x00, 0xFF, 0x58, 0x04,
    0x04, 0x02, 0x18, 0x08, 0x00, 0xFF, 0x59, 0x02, 0x00, 0x00, 0x00,
    0xFF, 0x06, 0x04, 0x3F, 0x3F, 0x3F, 0x3F, 0x91, 0xCA, 0x00, 0xFF,
    0x06, 0x03, 0x3F, 0x3F, 0x3F, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54,
    0x72, 0x6B, 0x00, 0x00, 0x00, 0x26, 0x00, 0xFF, 0x03, 0x07, 0x4B,
    0x6F, 0x6E, 0x74, 0x61, 0x6B, 0x74, 0x00, 0xFF, 0x7F, 0x09, 0x50,
    0x72, 0x65, 0x53, 0x01, 0xFF, 0xDD, 0x61, 0x05, 0x8F, 0x00, 0x90,
    0x3C, 0x66, 0x8F, 0x00, 0x80, 0x3C, 0x66, 0x00, 0xFF, 0x2F, 0x00,
};

int main() {
#if 0
    std::string filename = "__60bpm.mid";
    std::cout << filename << '\n';

    std::FILE *f = std::fopen(filename.c_str(), "rb");
    if (!f) {
        std::cout << "File not found.\n";
        return 1;
    }
    std::vector<std::uint8_t> data;
    std::uint8_t buf[1024];
    for (;;) {
        std::size_t nr = std::fread(buf, 1, 1024, f);
        if (nr == 0) break;
        data.insert(data.end(), buf, buf + nr);
    }
    std::fclose(f);
#endif

    SMFReader reader(std::move(data));

    SMF_Header *hdr = reader.read_header();
    std::cout << "Type:\t\t" << hdr->get_chunk_type() << '\n';
    std::cout << "Length:\t\t" << hdr->get_data_length() << '\n';
    std::cout << "Format:\t\t" << hdr->get_format() << '\n';
    std::cout << "Track:\t\t" << hdr->get_n_track() << '\n';
    std::cout << "Time Unit:\t" << hdr->get_time_unit() << '\n';

    for (std::uint16_t i = 0; i < hdr->get_n_track(); ++i) {
        std::cout << '\n';

        SMF_Track *trk = reader.read_track_header();
        std::cout << "Type:\t" << trk->get_chunk_type() << '\n';
        std::cout << "Length:\t" << trk->get_data_length() << '\n';
        SMF_EventBuffer *events = reader.create_event_reader(
            static_cast<std::size_t>(trk->get_data_length()));
        SMF_Event *ev;
        while ((ev = events->get_next()) != nullptr) {
            std::cout << "  delta time: " << ev->delta_time << '\n';
            if (ev->smf_event_type == SMF_Event::EventType::SMF_META) {
                SMF_MetaEvent *mev = static_cast<SMF_MetaEvent *>(ev);
                if (mev->type == 0x00) {
                    std::uint8_t buf[2] = {0};
                    buf[0] = mev->ptr[1];
                    buf[1] = mev->ptr[0];
                    std::uint16_t n;
                    std::memcpy(&n, buf, 2);
                    std::cout << "    Sequence Number: " << n << '\n';
                } else if (mev->type == 0x51) {
                    std::uint8_t buf[4] = {0};
                    buf[0] = mev->ptr[2];
                    buf[1] = mev->ptr[1];
                    buf[2] = mev->ptr[0];
                    int t;
                    std::memcpy(&t, buf, 4);
                    float tempo = 60'000'000.0 / t;
                    std::cout << "    Tempo: " << tempo << '\n';
                } else if (mev->type == 0x02) {
                    std::cout << "    Copyright: ";
                    std::cout << std::string(reinterpret_cast<char *>(mev->ptr),
                                             mev->length)
                              << '\n';
                } else if (mev->type == 0x03) {
                    std::cout << "    Track Name: ";
                    std::cout << std::string(reinterpret_cast<char *>(mev->ptr),
                                             mev->length)
                              << '\n';
                } else if (mev->type == 0x06) {
                    std::cout << "    Marker: ";
                    std::cout << std::string(reinterpret_cast<char *>(mev->ptr),
                                             mev->length)
                              << '\n';
                } else if (mev->type == 0x2F) {
                    std::cout << "    End of Track:\n";
                } else if (mev->type == 0x58) {
                    std::cout << "    Time Signature: ";
                    std::cout << +mev->ptr[0] << '/'
                              << std::pow(2, +mev->ptr[1]) << '\n';
                } else if (mev->type == 0x59) {
                    std::cout << "    Key Signature: ";
                    std::cout << +mev->ptr[0] << ',' << +mev->ptr[1] << '\n';
                } else if (mev->type == 0x7F) {
                    std::cout << "    Sequencer Defined Meta Event: \n";
                } else {
                    std::cout << "    meta event: " << +mev->type << '\n';
                }
            } else if (ev->smf_event_type == SMF_Event::EventType::SMF_MIDI) {
                SMF_MidiEvent *mev = static_cast<SMF_MidiEvent *>(ev);
                if (mev->type == SMF_MidiEvent::EventType::MIDI_NOTE_ON) {
                    std::cout << "    midi event: NOTE_ON\n";
                } else if (mev->type ==
                           SMF_MidiEvent::EventType::MIDI_NOTE_OFF) {
                    std::cout << "    midi event: NOTE_OFF\n";
                } else {
                    std::cout << "    midi event: CC\n";
                }
            } else if (ev->smf_event_type == SMF_Event::EventType::SMF_SYSEX) {
                std::cout << "    exclusive\n";
            }
        }
    }
}
