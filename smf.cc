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

    static SMF_EventBuffer from_file(std::FILE *f, std::uint32_t length) {
        SMF_EventBuffer events;
        events.length = length;
        events.buffer = new std::uint8_t[static_cast<std::size_t>(length)];
        events.cursor = 0;
        std::fread(events.buffer, 1, events.length, f);
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

int main() {
    std::string filename = "__60bpm.mid";
    std::FILE *f = std::fopen(filename.c_str(), "rb");
    if (!f) {
        std::cout << "File not found.\n";
        return 1;
    }
    std::cout << filename << '\n';

    SMF_Header hdr;
    std::fread(&hdr, sizeof(SMF_Header), 1, f);
    std::cout << "Type:\t\t" << hdr.get_chunk_type() << '\n';
    std::cout << "Length:\t\t" << hdr.get_data_length() << '\n';
    std::cout << "Format:\t\t" << hdr.get_format() << '\n';
    std::cout << "Track:\t\t" << hdr.get_n_track() << '\n';
    std::cout << "Time Unit:\t" << hdr.get_time_unit() << '\n';

    for (std::uint16_t i = 0; i < hdr.get_n_track(); ++i) {
        std::cout << '\n';

        SMF_Track trk;
        std::fread(&trk, sizeof(SMF_Track), 1, f);
        std::cout << "Type:\t" << trk.get_chunk_type() << '\n';
        std::cout << "Length:\t" << trk.get_data_length() << '\n';
        SMF_EventBuffer events =
            SMF_EventBuffer::from_file(f, trk.get_data_length());
        SMF_Event *ev;
        while ((ev = events.get_next()) != nullptr) {
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

    std::fclose(f);
}
