#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include <map>
#include <unordered_map>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/flacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>

// Enum for media types
enum class MediaType
{
    AUDIO,
    VIDEO,
    UNKNOWN
};

enum class AUDIO_CODEC_t
{
    mp3,
    flac,
    m4a,
    ogg,
    wv,
    ape,
    mka,
    opus,
    mpc,
    tak,
    alac,
    amr,
    ofr,
    tta,
    ra,
    spx
};

enum class VIDEO_CODEC_t
{
    mkv,
    webm,
    mp4,
    mov,
    avi,
    ogm,
    m2ts,
    ts,
    mpg,
    wmv,
    mxf,
    flv,
    rm,
    rmvb,
    dv
};

class MediaFileModel
{
private:
    std::string filename;
    std::string filepath;
    int duration; // in seconds
    MediaType type;
    std::map<std::string, std::string> metadata;

public:
    MediaFileModel() {};
    MediaFileModel(const std::string &path) : filepath(path), duration(0), type(MediaType::UNKNOWN)
    {
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            filename = path.substr(lastSlash + 1);
        else
            filename = path;
    }

    virtual ~MediaFileModel() {};

    const std::string& getFilename() const;
    const std::string& getFilepath() const;
    int getDuration() const;
    MediaType getType() const;

    void setDuration(int dur);
    void setType(MediaType t);

    void setMetadata(const std::string &key, const std::string &value);

    const std::string& getMetadata(const std::string &key) const;

    const std::map<std::string, std::string>& getAllMetadata() const;

    virtual bool loadMetadata();

    virtual bool saveMetadata();
};

// Audio file class
class AudioFileModel : public MediaFileModel
{
public:
    AudioFileModel(const std::string &path) : MediaFileModel(path)
    {
        setType(MediaType::AUDIO);
        loadMetadata();
    }
};


class VideoFileModel : public MediaFileModel
{
public:
    VideoFileModel(const std::string &path) : MediaFileModel(path)
    {
        setType(MediaType::VIDEO);
        loadMetadata();
    }

    bool loadMetadata() override;
};

#endif // MEDIA_H