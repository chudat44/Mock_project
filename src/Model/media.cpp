#include "media.h"



// MediaFileModel
const std::string &MediaFileModel::getFilename() const { return filename; }
const std::string &MediaFileModel::getFilepath() const { return filepath; }
int MediaFileModel::getDuration() const { return duration; }
MediaType MediaFileModel::getType() const { return type; }

void MediaFileModel::setDuration(int dur) { duration = dur; }
void MediaFileModel::setType(MediaType t) { type = t; }

void MediaFileModel::setMetadata(const std::string &key, const std::string &value)
{
    metadata[key] = value;
}

const std::string &MediaFileModel::getMetadata(const std::string &key) const
{
    auto it = metadata.find(key);
    if (it != metadata.end())
        return it->second;
    return NULL;
}

const std::map<std::string, std::string> &MediaFileModel::getAllMetadata() const
{
    return metadata;
}

bool MediaFileModel::loadMetadata()
{
    TagLib::FileRef f(filepath.c_str());
    if (f.isNull())
        return false;

    if (f.tag())
    {
        TagLib::Tag *tag = f.tag();

        if (!tag->title().isEmpty())
            metadata["Title"] = tag->title().to8Bit(true);
        if (!tag->artist().isEmpty())
            metadata["Artist"] = tag->artist().to8Bit(true);
        if (!tag->album().isEmpty())
            metadata["Album"] = tag->album().to8Bit(true);
        if (!tag->comment().isEmpty())
            metadata["Comment"] = tag->comment().to8Bit(true);
        if (!tag->genre().isEmpty())
            metadata["Genre"] = tag->genre().to8Bit(true);
        if (tag->year() != 0)
            metadata["Year"] = std::to_string(tag->year());
        if (tag->track() != 0)
            metadata["Track"] = std::to_string(tag->track());
    }

    if (f.audioProperties())
    {
        TagLib::AudioProperties *props = f.audioProperties();
        duration = props->lengthInSeconds();
        metadata["Bitrate"] = std::to_string(props->bitrate()) + " kbps";
        metadata["Channels"] = std::to_string(props->channels());
        metadata["Sample Rate"] = std::to_string(props->sampleRate()) + " Hz";
    }

    return true;
}

bool MediaFileModel::saveMetadata()
{
    TagLib::FileRef f(filepath.c_str());
    if (f.isNull())
        return false;

    TagLib::Tag *tag = f.tag();
    if (!tag)
        return false;

    if (metadata.find("Title") != metadata.end())
        tag->setTitle(TagLib::String(metadata["Title"], TagLib::String::UTF8));
    if (metadata.find("Artist") != metadata.end())
        tag->setArtist(TagLib::String(metadata["Artist"], TagLib::String::UTF8));
    if (metadata.find("Album") != metadata.end())
        tag->setAlbum(TagLib::String(metadata["Album"], TagLib::String::UTF8));
    if (metadata.find("Comment") != metadata.end())
        tag->setComment(TagLib::String(metadata["Comment"], TagLib::String::UTF8));
    if (metadata.find("Genre") != metadata.end())
        tag->setGenre(TagLib::String(metadata["Genre"], TagLib::String::UTF8));
    if (metadata.find("Year") != metadata.end())
        tag->setYear(std::stoi(metadata["Year"]));
    if (metadata.find("Track") != metadata.end())
        tag->setTrack(std::stoi(metadata["Track"]));

    return f.save();
}

// Video_File_Metadata_Model Implementation
bool VideoFileModel::loadMetadata()
{
    bool success = MediaFileModel::loadMetadata();

    // Add video-specific metadata extraction here
    // This would typically use other libraries like FFmpeg
    setMetadata("Codec", "Unknown");      // Placeholder
    setMetadata("Resolution", "Unknown"); // Placeholder

    return success;
}