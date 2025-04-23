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

const std::string MediaFileModel::getMetadata(const std::string &key) const
{
    if (metadata.empty())
        return "";

    auto it = metadata.find(key);
    if (it != metadata.end())
        return it->second;
    return "";
}

const std::map<std::string, std::string> &MediaFileModel::getAllMetadata() const
{
    return metadata;
}

const std::map<std::string, std::string> &MediaFileModel::getAllAddMetadata() const
{
    return addMetadata;
}