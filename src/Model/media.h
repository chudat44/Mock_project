#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include <map>


// Enum for media types
enum class MediaType
{
    AUDIO,
    VIDEO,
    UNKNOWN
};

class MediaFileModel
{
private:
    std::string filename;
    std::string filepath;
    int duration; // in seconds
    MediaType type;
    std::map<std::string, std::string> metadata;
    std::map<std::string, std::string> addMetadata;

public:
    MediaFileModel() {};
    MediaFileModel(const std::string &path) : filepath(path), duration(0), type(MediaType::UNKNOWN)
    {
        size_t lastSlash = std::string(path).find_last_of("/\\");
        if (lastSlash != std::string::npos)
            filename = std::string(path).substr(lastSlash + 1);
        else
            filename = path;
    }

    virtual ~MediaFileModel() {};

    const std::string &getFilename() const;
    const std::string &getFilepath() const;
    int getDuration() const;
    MediaType getType() const;

    void setDuration(int dur);
    void setType(MediaType t);

    void setMetadata(const std::string &key, const std::string &value);

    const std::string getMetadata(const std::string &key) const;

    const std::map<std::string, std::string> &getAllMetadata() const;

    const std::map<std::string, std::string> &getAllAddMetadata() const;

};

// Audio file class
class AudioFileModel : public MediaFileModel
{
public:
    AudioFileModel(const std::string &path) : MediaFileModel(path)
    {
        setType(MediaType::AUDIO);
    }
};

class VideoFileModel : public MediaFileModel
{
public:
    VideoFileModel(const std::string &path) : MediaFileModel(path)
    {
        setType(MediaType::VIDEO);
    }
};

#endif // MEDIA_H