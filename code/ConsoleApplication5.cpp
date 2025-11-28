// ConsoleApplication5.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include <cstring>
#pragma comment(lib, "winmm.lib")

using namespace std;

// Song class
class Song {
public:
    string title;
    string artist;
    string filePath;

    Song(string t, string a, string path) {
        title = t;
        artist = a;
        filePath = path;
    }
};

// Song node in doubly linked list
class SongNode {
public:
    Song data;
    SongNode* next;
    SongNode* prev;

    // تم إصلاح الكونستركتور باستخدام initializer list
    SongNode(Song s) : data(s) {
        next = nullptr;
        prev = nullptr;
    }
};

// Playlist class
class PlaylistNode {
public:
    string name;
    SongNode* head;
    SongNode* tail;
    SongNode* current;
    PlaylistNode* next;
    PlaylistNode* prev;

    PlaylistNode(string n) {
        name = n;
        head = nullptr;
        tail = nullptr;
        current = nullptr;
        next = nullptr;
        prev = nullptr;
    }
};

class MusicPlayer {
private:
    PlaylistNode* playlistHead;
    PlaylistNode* playlistTail;
    PlaylistNode* activePlaylist;
    bool isPlaying;
    bool isPaused;

    void clearSongs(PlaylistNode* playlist) {
        if (!playlist) return;

        SongNode* current = playlist->head;
        while (current) {
            SongNode* temp = current;
            current = current->next;
            delete temp;
        }
        playlist->head = nullptr;
        playlist->tail = nullptr;
        playlist->current = nullptr;
    }

    PlaylistNode* findPlaylist(string name) {
        PlaylistNode* temp = playlistHead;
        while (temp) {
            if (temp->name == name) return temp;
            temp = temp->next;
        }
        return nullptr;
    }

    // Play audio file using Windows MCI
    bool playAudioFile(string filePath, string title, string artist) {
        if (filePath.empty()) {
            cout << "[!] File path is empty!" << endl;
            return false;
        }

        // Stop any currently playing audio
        stopAudio();

        // Open and play using MCI
        string openCmd = "open \"" + filePath + "\" type mpegvideo alias SONG";
        if (mciSendStringA(openCmd.c_str(), NULL, 0, NULL) != 0) {
            cout << "[!] Error opening file!" << endl;
            return false;
        }

        if (mciSendStringA("play SONG", NULL, 0, NULL) != 0) {
            cout << "[!] Error playing file!" << endl;
            return false;
        }

        isPlaying = true;
        isPaused = false;

        cout << "\n[PLAYING] " << title << " - " << artist << endl;
        cout << "[FILE] " << filePath << "\n" << endl;
        return true;
    }

public:
    MusicPlayer() {
        playlistHead = nullptr;
        playlistTail = nullptr;
        activePlaylist = nullptr;
        isPlaying = false;
        isPaused = false;
    }

    ~MusicPlayer() {
        stopAudio();

        PlaylistNode* current = playlistHead;
        while (current) {
            PlaylistNode* temp = current;
            current = current->next;
            clearSongs(temp);
            delete temp;
        }
    }

    // Stop currently playing audio
    void stopAudio() {
        if (isPlaying) {
            mciSendStringA("stop SONG", NULL, 0, NULL);
            mciSendStringA("close SONG", NULL, 0, NULL);
            isPlaying = false;
            isPaused = false;
        }
    }

    // Pause/Resume audio
    void togglePause() {
        if (!isPlaying) {
            cout << "[!] No song is playing!" << endl;
            return;
        }

        if (isPaused) {
            mciSendStringA("resume SONG", NULL, 0, NULL);
            isPaused = false;
            cout << "\n[RESUMED] Playback resumed" << endl;
        }
        else {
            mciSendStringA("pause SONG", NULL, 0, NULL);
            isPaused = true;
            cout << "\n[PAUSED] Playback paused" << endl;
        }
    }

    bool createPlaylist(string name) {
        if (name.empty()) {
            cout << "[!] Name cannot be empty!" << endl;
            return false;
        }

        if (findPlaylist(name)) {
            cout << "[!] Playlist '" << name << "' already exists!" << endl;
            return false;
        }

        PlaylistNode* newNode = new PlaylistNode(name);

        if (!playlistHead) {
            playlistHead = playlistTail = newNode;
        }
        else {
            playlistTail->next = newNode;
            newNode->prev = playlistTail;
            playlistTail = newNode;
        }

        cout << "[+] Created Playlist: " << name << endl;
        return true;
    }

    bool selectPlaylist(string name) {
        PlaylistNode* found = findPlaylist(name);
        if (!found) {
            cout << "[!] Playlist '" << name << "' not found" << endl;
            return false;
        }

        activePlaylist = found;
        cout << "[+] Selected Playlist: " << name << endl;
        return true;
    }

    void showAllPlaylists() {
        if (!playlistHead) {
            cout << "\n[!] No playlists!" << endl;
            return;
        }

        cout << "\n=============== ALL PLAYLISTS ===============" << endl;
        PlaylistNode* temp = playlistHead;
        int num = 1;

        while (temp) {
            if (temp == activePlaylist) {
                cout << "\n> " << num << ". [" << temp->name << "]";
            }
            else {
                cout << "\n  " << num << ". [" << temp->name << "]";
            }

            int count = 0;
            SongNode* song = temp->head;
            while (song) {
                count++;
                song = song->next;
            }
            cout << " (" << count << " songs)" << endl;

            // Display songs
            SongNode* s = temp->head;
            int sNum = 1;
            while (s) {
                if (temp == activePlaylist && s == temp->current) {
                    cout << "  > " << sNum << ". " << s->data.title << " [NOW PLAYING]" << endl;
                }
                else {
                    cout << "    " << sNum << ". " << s->data.title << endl;
                }
                s = s->next;
                sNum++;
            }

            temp = temp->next;
            num++;
        }
        cout << "============================================\n" << endl;
    }

    bool deletePlaylist(string name) {
        PlaylistNode* toDelete = findPlaylist(name);
        if (!toDelete) {
            cout << "[!] Playlist '" << name << "' not found" << endl;
            return false;
        }

        clearSongs(toDelete);

        if (toDelete->prev) {
            toDelete->prev->next = toDelete->next;
        }
        else {
            playlistHead = toDelete->next;
        }

        if (toDelete->next) {
            toDelete->next->prev = toDelete->prev;
        }
        else {
            playlistTail = toDelete->prev;
        }

        if (activePlaylist == toDelete) {
            stopAudio();
            activePlaylist = playlistHead;
        }

        delete toDelete;
        cout << "[-] Deleted Playlist: " << name << endl;
        return true;
    }

    // Add song with file path
    bool addSong(string title, string artist, string filePath) {
        if (!activePlaylist) {
            cout << "[!] No playlist selected!" << endl;
            return false;
        }

        if (title.empty() || filePath.empty()) {
            cout << "[!] Song title and file path cannot be empty!" << endl;
            return false;
        }

        Song newSong(title, artist, filePath);
        SongNode* newNode = new SongNode(newSong);

        if (!activePlaylist->head) {
            activePlaylist->head = activePlaylist->tail = newNode;
            activePlaylist->current = newNode;
        }
        else {
            activePlaylist->tail->next = newNode;
            newNode->prev = activePlaylist->tail;
            activePlaylist->tail = newNode;
        }

        cout << "[+] Added song: " << title << endl;
        return true;
    }

    // Add song to specific playlist by name
    bool addSongToPlaylist(string playlistName, string title, string artist, string filePath) {
        PlaylistNode* playlist = findPlaylist(playlistName);
        if (!playlist) {
            cout << "[!] Playlist '" << playlistName << "' not found" << endl;
            return false;
        }

        if (title.empty() || filePath.empty()) {
            cout << "[!] Song title and file path cannot be empty!" << endl;
            return false;
        }

        Song newSong(title, artist, filePath);
        SongNode* newNode = new SongNode(newSong);

        if (!playlist->head) {
            playlist->head = playlist->tail = newNode;
            playlist->current = newNode;
        }
        else {
            playlist->tail->next = newNode;
            newNode->prev = playlist->tail;
            playlist->tail = newNode;
        }

        cout << "[+] Added song: " << title << " to playlist: " << playlistName << endl;
        return true;
    }

    bool removeSong(string title) {
        if (!activePlaylist || !activePlaylist->head) {
            cout << "[!] No songs to remove!" << endl;
            return false;
        }

        SongNode* temp = activePlaylist->head;

        while (temp) {
            if (temp->data.title == title) {
                if (temp->prev) {
                    temp->prev->next = temp->next;
                }
                else {
                    activePlaylist->head = temp->next;
                }

                if (temp->next) {
                    temp->next->prev = temp->prev;
                }
                else {
                    activePlaylist->tail = temp->prev;
                }

                if (activePlaylist->current == temp) {
                    stopAudio();
                    activePlaylist->current = temp->next ? temp->next : activePlaylist->head;
                }

                delete temp;
                cout << "[-] Deleted song: " << title << endl;
                return true;
            }
            temp = temp->next;
        }

        cout << "[!] Song '" << title << "' not found" << endl;
        return false;
    }

    // Search and play specific song
    bool playSong(string title) {
        if (!activePlaylist || !activePlaylist->head) {
            cout << "[!] No songs available!" << endl;
            return false;
        }

        SongNode* temp = activePlaylist->head;

        while (temp) {
            if (temp->data.title == title) {
                activePlaylist->current = temp;
                return playAudioFile(temp->data.filePath,
                    temp->data.title,
                    temp->data.artist);
            }
            temp = temp->next;
        }

        cout << "[!] Song '" << title << "' not found!" << endl;
        return false;
    }

    bool playNext() {
        if (!activePlaylist || !activePlaylist->head) {
            cout << "[!] No songs available!" << endl;
            return false;
        }

        if (!activePlaylist->current) {
            activePlaylist->current = activePlaylist->head;
        }
        else if (activePlaylist->current->next) {
            activePlaylist->current = activePlaylist->current->next;
        }
        else {
            activePlaylist->current = activePlaylist->head;
            cout << "[~] Starting from beginning..." << endl;
        }

        return playAudioFile(activePlaylist->current->data.filePath,
            activePlaylist->current->data.title,
            activePlaylist->current->data.artist);
    }

    bool playPrevious() {
        if (!activePlaylist || !activePlaylist->head) {
            cout << "[!] No songs available!" << endl;
            return false;
        }

        if (!activePlaylist->current) {
            activePlaylist->current = activePlaylist->tail;
        }
        else if (activePlaylist->current->prev) {
            activePlaylist->current = activePlaylist->current->prev;
        }
        else {
            cout << "[!] This is the first song!" << endl;
            return false;
        }

        return playAudioFile(activePlaylist->current->data.filePath,
            activePlaylist->current->data.title,
            activePlaylist->current->data.artist);
    }

    bool playCurrent() {
        if (!activePlaylist || !activePlaylist->current) {
            cout << "[!] No song selected" << endl;
            return false;
        }

        return playAudioFile(activePlaylist->current->data.filePath,
            activePlaylist->current->data.title,
            activePlaylist->current->data.artist);
    }

    bool playFirst() {
        if (!activePlaylist || !activePlaylist->head) {
            cout << "[!] Playlist is empty!" << endl;
            return false;
        }

        activePlaylist->current = activePlaylist->head;
        return playAudioFile(activePlaylist->current->data.filePath,
            activePlaylist->current->data.title,
            activePlaylist->current->data.artist);
    }

    // Start playback loop with keyboard controls
    void startPlaybackLoop() {
        if (!activePlaylist) {
            cout << "[!] Select a playlist first!" << endl;
            return;
        }

        if (!activePlaylist->head) {
            cout << "[!] Playlist is empty!" << endl;
            return;
        }

        playFirst();

        cout << "\n========== PLAYBACK CONTROLS ==========" << endl;
        cout << "  <- (Left Arrow)  : Previous Song" << endl;
        cout << "  -> (Right Arrow) : Next Song" << endl;
        cout << "  SPACE            : Pause/Resume" << endl;
        cout << "  ESC              : Stop & Exit" << endl;
        cout << "========================================\n" << endl;

        bool exitRequested = false;

        while (isPlaying && !exitRequested) {
            // Check if current song has finished
            char status[128];
            mciSendStringA("status SONG mode", status, sizeof(status), NULL);

            if (strcmp(status, "stopped") == 0 && !isPaused) {
                // Song finished, play next automatically
                if (activePlaylist->current && activePlaylist->current->next) {
                    cout << "\n[AUTO] Playing next song..." << endl;
                    playNext();
                }
                else {
                    cout << "\n[END] Playlist finished!" << endl;
                    stopAudio();
                    break;
                }
            }

            if (_kbhit()) {
                int key = _getch();

                // Handle special keys (arrows)
                if (key == 0 || key == 224) {
                    key = _getch();

                    if (key == 75) {  // Left Arrow
                        playPrevious();
                    }
                    else if (key == 77) {  // Right Arrow
                        playNext();
                    }
                }
                // Handle regular keys
                else if (key == ' ') {  // Space
                    togglePause();
                }
                else if (key == 27) {  // ESC
                    stopAudio();
                    cout << "\n[+] Exiting playback mode..." << endl;
                    exitRequested = true;
                }
            }
            Sleep(100);
        }
    }

    PlaylistNode* getActivePlaylist() {
        return activePlaylist;
    }
};

void printMainMenu() {
    cout << "\n============================================" << endl;
    cout << "       AUDIO PLAYLIST MANAGER" << endl;
    cout << "============================================" << endl;
    cout << " 1. Add New Playlist" << endl;
    cout << " 2. Add Song to Playlist" << endl;
    cout << " 3. Display All Playlists" << endl;
    cout << " 4. Select Playlist" << endl;
    cout << " 5. Delete Playlist" << endl;
    cout << " 0. Exit" << endl;
    cout << "============================================" << endl;
    cout << "Your choice: ";
}

int main() {
    MusicPlayer player;
    int choice;
    string playlistName, songTitle, artist, path;

    system("cls");
    system("color 0A");

    cout << "=== AUDIO PLAYLIST MANAGER ===" << endl;
    cout << "\nNOTE: This player uses MCI (Media Control Interface)" << endl;
    cout << "Supported formats: MP3, WAV, WMA, etc.\n" << endl;

    // Creating playlists with songs using the new method
    cout << "[INFO] Loading playlists..." << endl;

    // Playlist 1: Al_Munshawy
    player.createPlaylist("Al_Munshawy");
    player.addSongToPlaylist("Al_Munshawy", "Yasin", "Sheikh Menshawy",
        "C:\\Users\\king\\Desktop\\AL_Minshwi\\الشيخ المنشاوي - سورة يس - تلاوة مُرتَّلة(MP3_160K).mp3");
    player.addSongToPlaylist("Al_Munshawy", "Gafer", "Sheikh Menshawy",
        "C:\\Users\\king\\Desktop\\AL_Minshwi\\الشيخ المنشاوي - سورة غافر (مُرتَّل)(MP3_160K).mp3");

    cout << "\n[INFO] All playlists loaded successfully!" << endl;
    cout << "[INFO] Select a playlist using option 4 to start playing\n" << endl;

    // Main Loop
    do {
        printMainMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            cout << "Playlist name: ";
            getline(cin, playlistName);
            player.createPlaylist(playlistName);
            break;

        case 2:
            cout << "Playlist name: ";
            getline(cin, playlistName);
            cout << "Song title: ";
            getline(cin, songTitle);
            cout << "Artist name: ";
            getline(cin, artist);
            cout << "Full path to audio file: ";
            getline(cin, path);
            player.addSongToPlaylist(playlistName, songTitle, artist, path);
            break;

        case 3:
            player.showAllPlaylists();
            break;

        case 4: {
            cout << "Playlist name: ";
            getline(cin, playlistName);
            player.selectPlaylist(playlistName);

            if (player.getActivePlaylist() != nullptr) {
                int playlistChoice;
                do {
                    cout << "\n============================================" << endl;
                    cout << "    PLAYLIST: [" << player.getActivePlaylist()->name << "]" << endl;
                    cout << "============================================" << endl;
                    cout << " 1. Play from Beginning" << endl;
                    cout << " 2. Search & Play Specific Song" << endl;
                    cout << " 3. Add Song to This Playlist" << endl;
                    cout << " 4. Delete Song from This Playlist" << endl;
                    cout << " 0. Exit Playlist" << endl;
                    cout << "============================================" << endl;
                    cout << "Your choice: ";
                    cin >> playlistChoice;
                    cin.ignore();

                    switch (playlistChoice) {
                    case 1:
                        player.startPlaybackLoop();
                        break;

                    case 2:
                        cout << "Song title: ";
                        getline(cin, songTitle);
                        player.playSong(songTitle);
                        break;

                    case 3:
                        cout << "Song title: ";
                        getline(cin, songTitle);
                        cout << "Artist name: ";
                        getline(cin, artist);
                        cout << "Full path to audio file: ";
                        getline(cin, path);
                        player.addSong(songTitle, artist, path);
                        break;

                    case 4:
                        cout << "Song title: ";
                        getline(cin, songTitle);
                        player.removeSong(songTitle);
                        break;

                    case 0:
                        cout << "\n[+] Exiting playlist..." << endl;
                        break;

                    default:
                        cout << "[!] Invalid choice!" << endl;
                    }

                } while (playlistChoice != 0);
            }
            break;
        }

        case 5:
            cout << "Playlist name: ";
            getline(cin, playlistName);
            player.deletePlaylist(playlistName);
            break;

        case 0:
            player.stopAudio();
            cout << "\n=== Thanks for using Audio Playlist Manager! ===" << endl;
            cout << "Goodbye!\n" << endl;
            break;

        default:
            cout << "[!] Invalid choice!" << endl;
        }

    } while (choice != 0);

    return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
