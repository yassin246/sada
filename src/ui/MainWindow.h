#pragma once

#include "core/I18n.h"
#include "core/Player.h"
#include "core/Song.h"
#include "ui/NowPlayingBar.h"

#include <QMainWindow>
#include <QSettings>

class PlaylistModel;
class LibraryProxy;
class LibraryScanner;
class SongTable;
class Sidebar;
class EqualizerDialog;
class TrayIcon;
class MiniPlayer;
class NowPlayingPage;
class Toast;
class QButtonGroup;
class QAction;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openFiles(const QStringList& paths); // external drops / CLI args
    void setLanguage(I18n::Lang lang);
    I18n::Lang language() const { return m_lang; }
    bool playerValid() const { return m_player && m_player->valid(); }
    void debugOpenTab(int tab); // dev: switch tab by index (0 lib, 1 queue, 2 eq, 3 now)

protected:
    void closeEvent(QCloseEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;

private slots:
    void retranslate();
    void playOrPause();
    void nextTrack();
    void prevTrack();
    void playQueueRow(int row);
    void onEndOfFile();
    void pollPosition();
    void updateNowPlaying();
    void onSearchChanged(const QString& text);
    void onFolderActivated(const QString& path);
    void onArtistActivated(const QString& artist);
    void onClearArtist();
    void onBrowseFolder();
    void onRescan();
    void onLibraryDoubleClicked(const QModelIndex& idx);
    void onQueueDoubleClicked(const QModelIndex& idx);
    void playLibrarySelection();
    void addLibrarySelectionToQueue();
    void onEqGains(const QVector<float>& gains);
    void toggleRepeat();
    void openFilesDialog();
    void openFolderDialog();
    void savePlaylistDialog();
    void onQueueContextMenu(const QPoint& pos);
    void showMini();
    void showAbout();
    void quitApp();

private:
    void buildUi();
    void connectSignals();
    void saveSettings();
    void loadSettings();
    void applyLayoutDirection();
    void scanLibrary(const QString& path);
    void loadVisibleLibraryToQueue(int startProxyRow);
    QPixmap coverPixmap(const Song& song, int size = 64);
    void setShuffle(bool on);
    void removeSelectedFromQueue();

    Player* m_player = nullptr;
    PlaylistModel* m_queue = nullptr;
    PlaylistModel* m_library = nullptr;
    LibraryProxy* m_libraryProxy = nullptr;
    LibraryScanner* m_scanner = nullptr;

    SongTable* m_queueView = nullptr;
    SongTable* m_libraryView = nullptr;
    QLabel* m_libraryStatus = nullptr;
    QLineEdit* m_search = nullptr;
    QStackedWidget* m_stack = nullptr;
    QButtonGroup* m_tabs = nullptr;
    QPushButton* m_tabLibrary = nullptr;
    QPushButton* m_tabQueue = nullptr;
    QPushButton* m_tabEq = nullptr;
    QPushButton* m_tabNow = nullptr;
    QMenu* m_menu = nullptr;
    QAction* m_aboutAction = nullptr;
    Sidebar* m_sidebar = nullptr;
    NowPlayingBar* m_npb = nullptr;
    NowPlayingPage* m_npPage = nullptr;
    EqualizerDialog* m_eq = nullptr;
    TrayIcon* m_tray = nullptr;
    MiniPlayer* m_mini = nullptr;
    Toast* m_toast = nullptr;
    QTimer* m_posTimer = nullptr;

    NowPlayingBar::RepeatMode m_repeat = NowPlayingBar::RepeatAll;
    bool m_shuffle = false;
    I18n::Lang m_lang = I18n::En;
    QSettings m_settings;
    QString m_lastScanRoot;
    QString m_lastDir;
    QString m_lastPlayedPath;
    bool m_miniMode = false;
    bool m_quitting = false;
    bool m_queueChanged = false;
};