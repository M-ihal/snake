#ifndef P_UTILS_H
#define P_UTILS_H

inline bool
is_big_endian(void) {
    int a = 1;
    return !((char *)&a)[0];
}

inline u32 get_u32(byte *ptr, byte **to_advance = nullptr) {
    u32 result = 0;
    if(!is_big_endian()) {
        ((byte *)&result)[0] = *(ptr + 0);
        ((byte *)&result)[1] = *(ptr + 1);
        ((byte *)&result)[2] = *(ptr + 2);
        ((byte *)&result)[3] = *(ptr + 3);
    }
    else {
        ((byte *)&result)[0] = *(ptr + 3);
        ((byte *)&result)[1] = *(ptr + 2);
        ((byte *)&result)[2] = *(ptr + 1);
        ((byte *)&result)[3] = *(ptr + 0);
    }
    
    if(to_advance) { *to_advance += 4; }
    return result;
}

inline u16 get_u16(byte *ptr, byte **to_advance = nullptr) {
    u16 result = 0;
    if(!is_big_endian()) {
        ((byte *)&result)[0] = *(ptr + 0);
        ((byte *)&result)[1] = *(ptr + 1);
    }
    else {
        ((byte *)&result)[0] = *(ptr + 1);
        ((byte *)&result)[1] = *(ptr + 0);
    }
    
    if(to_advance) { *to_advance += 2; }
    return result;
}

#define WAV_RIFF_ID_VALUE ('R' << 0 | 'I' << 8 | 'F' << 16 | 'F' << 24)
#define WAV_WAVE_ID_VALUE ('W' << 0 | 'A' << 8 | 'V' << 16 | 'E' << 24)
#define WAV_FMT_ID_VALUE  ('f' << 0 | 'm' << 8 | 't' << 16 | ' ' << 24)
#define WAV_FACT_ID_VALUE ('f' << 0 | 'a' << 8 | 'c' << 16 | 't' << 24)
#define WAV_LIST_ID_VALUE ('L' << 0 | 'I' << 8 | 'S' << 16 | 'T' << 24)
#define WAV_DATA_ID_VALUE ('d' << 0 | 'a' << 8 | 't' << 16 | 'a' << 24)

struct Wav_riff {
    u32 riff_id;
    u32 chunk_size;
    u32 wave_id;
};

struct Wav_fmt {
    u32 fmt_id;
    u32 chunk_size;
    u16 format_tag; // NOTE: values other than 1 indicates compression
    u16 channels;
    u32 samples_per_second;
    u32 average_bytes_per_second;
    u16 block_align;
    u16 bits_per_sample;
};

struct Wav_fact {
    u32 fact_id;
    u32 chunk_size;
    u32 sample_length;
};

struct Wav_data {
    u32 data_id;
    u32 chunk_size;
};

struct LoadedSound {
    u32 channels;
    u32 bits_per_sample;
    u32 sample_rate;
    
    u32   data_size;
    void *data;
};

static LoadedSound
load_sound_wav(const char *path, PlatformProcs *procs, /* out */ Wav_fmt *format = nullptr) {
    FileContents wav_file;
    if(procs->read_file(&wav_file, path, false)) {
        byte *cursor = (byte *)wav_file.contents;
        
        Wav_riff *riff = nullptr;
        Wav_fmt  *fmt  = nullptr;
        Wav_fact *fact = nullptr;
        Wav_data *data = nullptr;
        
        LoadedSound sound = {};
        bool done = false;
        while(!done) {
            switch(*(u32 *)cursor) {
                case WAV_RIFF_ID_VALUE: {
                    riff = (Wav_riff *)cursor;
                    cursor += sizeof(Wav_riff);
                    
                    if(riff->wave_id != WAV_WAVE_ID_VALUE) {
                        ASSERT(false, "WAVE CHUNK NOT FOUND\n");
                        done = true;
                    }
                } break;
                
                case WAV_FMT_ID_VALUE: {
                    fmt = (Wav_fmt *)cursor;
                    cursor += fmt->chunk_size;
                    cursor += sizeof(fmt->fmt_id);
                    cursor += sizeof(fmt->chunk_size);
                } break;
                
                case WAV_FACT_ID_VALUE: {
                    fact = (Wav_fact *)cursor;
                    cursor += fact->chunk_size;
                    cursor += sizeof(fact->fact_id);
                    cursor += sizeof(fact->chunk_size);
                } break;
                
                case WAV_LIST_ID_VALUE: {
                    u32 *list_id = (u32 *)cursor;
                    u32 *chunk_size = (u32 *)(cursor + 4);
                    cursor += *chunk_size;
                    cursor += sizeof(u32);
                    cursor += sizeof(u32);
                } break;
                
                case WAV_DATA_ID_VALUE: {
                    data = (Wav_data *)cursor;
                    void *sound_data = cursor + sizeof(Wav_data);
                    
                    sound.data_size = data->chunk_size;
                    sound.data = procs->alloc(sound.data_size);
                    if(!sound.data) {
                        ASSERT(false, "FAILED TO ALLOCATE DATA\n");
                        done = true;
                    }
                    copy_memory(sound_data, sound.data, sound.data_size);
                    
                    sound.channels        = fmt->channels;
                    sound.bits_per_sample = fmt->bits_per_sample;
                    sound.sample_rate     = fmt->samples_per_second;
                    
                    if(format) {
                        copy_memory(fmt, format, sizeof(Wav_fmt));
                    }
                    
                    done = true;
                } break;
                
                default: {
                    print_ptr(riff);
                    print_ptr(fmt);
                    print_ptr(fact);
                    print_ptr(data);
                    
                    ASSERT(false, "");
                    return {};
                };
            };
        }
        procs->free_file(&wav_file);
        return sound;
    }
    return {};
}

static void
free_sound(LoadedSound *sound, PlatformProcs *procs) {
    procs->free(sound->data);
    sound->data = nullptr;
    sound->data_size = 0;
}

#endif /* P_UTILS_H */
