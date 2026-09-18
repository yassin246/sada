#pragma once

#include "core/Song.h"

#include <QImage>

// Reads audio metadata (tags) and embedded cover art via TagLib.
namespace TagReader {

Song readSong(const QString& path);
QImage coverImage(const QString& path);

} // namespace TagReader