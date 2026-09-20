#pragma once

#include <QColor>
#include <QIcon>

// Hand-drawn pixel-style icons (no antialiasing on a small canvas -> chunky).
namespace PixelIcons {

QIcon play(const QColor& c);
QIcon pause(const QColor& c);
QIcon next(const QColor& c);
QIcon prev(const QColor& c);
QIcon stop(const QColor& c);
QIcon folder(const QColor& c);
QIcon music(const QColor& c);
QIcon eq(const QColor& c);
QIcon repeat(const QColor& c);
QIcon shuffle(const QColor& c);
QIcon volume(const QColor& c);
QIcon mute(const QColor& c);
QIcon mini(const QColor& c);
QIcon plus(const QColor& c);
QIcon trash(const QColor& c);
QIcon search(const QColor& c);
QIcon lang(const QColor& c);
QIcon heart(const QColor& c);
QIcon download(const QColor& c);
QIcon more(const QColor& c);
QIcon appLogo();

// Convenience variants tinted with the theme palette.
QIcon playGreen();
QIcon playRed();
QIcon pauseGreen();
QIcon pauseRed();
QIcon playDim();
QIcon pauseDim();
QIcon nextDim();
QIcon prevDim();
QIcon folderDim();
QIcon musicDim();
QIcon eqGreen();
QIcon repeatDim();
QIcon repeatGreen();
QIcon shuffleDim();
QIcon shuffleGreen();
QIcon volumeDim();
QIcon muteRed();
QIcon miniDim();
QIcon plusGreen();
QIcon trashRed();
QIcon searchDim();
QIcon langDim();
QIcon heartRed();
QIcon heartDim();
QIcon downloadDim();
QIcon moreDim();

} // namespace PixelIcons