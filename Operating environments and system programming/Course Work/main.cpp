#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cerrno>
#include <clocale>
#include <cwchar>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <curses.h>
#include <locale>

std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return L"";
    size_t len = mbstowcs(nullptr, str.c_str(), 0);
    if (len == (size_t)-1) return L"";
    std::vector<wchar_t> buf(len + 1);
    mbstowcs(buf.data(), str.c_str(), len + 1);
    return std::wstring(buf.data());
}

std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    size_t len = wcstombs(nullptr, wstr.c_str(), 0);
    if (len == (size_t)-1) return "";
    std::vector<char> buf(len + 1);
    wcstombs(buf.data(), wstr.c_str(), len + 1);
    return std::string(buf.data());
}

std::ofstream logFile;
void log(const std::string& msg) {
    if (logFile.is_open()) {
        logFile << msg << std::endl;
    }
}

struct FileEntry {
    std::wstring name;
    bool isDirectory;
    off_t size;
    mode_t mode;
};

struct TreeNode {
    std::wstring name;
    std::string fullPath;
    bool isDirectory;
    bool expanded;
    std::vector<std::unique_ptr<TreeNode>> children;
    TreeNode* parent;

    TreeNode(const std::string& path, const std::wstring& n, bool isDir, TreeNode* p = nullptr)
        : name(n), fullPath(path), isDirectory(isDir), expanded(false), parent(p) {}

    void loadChildren() {
        if (!isDirectory || expanded) return;
        children.clear();
        DIR* dir = opendir(fullPath.c_str());
        if (!dir) {
            log("Error opening dir " + fullPath + ": " + strerror(errno));
            return;
        }
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            std::string childPath = fullPath + "/" + entry->d_name;
            struct stat st;
            if (stat(childPath.c_str(), &st) == -1) {
                log("stat error on " + childPath + ": " + strerror(errno));
                continue;
            }
            bool isDir = S_ISDIR(st.st_mode);
            std::wstring wname = utf8_to_wstring(entry->d_name);
            children.push_back(std::make_unique<TreeNode>(childPath, wname, isDir, this));
        }
        closedir(dir);
        std::sort(children.begin(), children.end(),
                  [](const auto& a, const auto& b) {
                      if (a->isDirectory != b->isDirectory)
                          return a->isDirectory > b->isDirectory;
                      return a->name < b->name;
                  });
        expanded = true;
    }

    void unloadChildren() {
        if (!isDirectory) return;
        children.clear();
        expanded = false;
    }
};

class FileManager {
private:
    std::unique_ptr<TreeNode> root;
    std::vector<TreeNode*> flatList;
    int selectedIndex;
    int scrollOffset;
    bool treeMode;
    std::string currentPath;                  
    std::vector<FileEntry> flatEntries;
    WINDOW* listWin;
    WINDOW* statusWin;
    int maxY, maxX;
    std::wstring statusMsg;

    void buildFlatListFromTree(TreeNode* node, int depth) {
        if (!node) return;
        flatList.push_back(node);
        if (node->isDirectory && node->expanded) {
            for (auto& child : node->children)
                buildFlatListFromTree(child.get(), depth + 1);
        }
    }

    void updateFlatList() {
        flatList.clear();
        if (treeMode && root)
            buildFlatListFromTree(root.get(), 0);
    }

    void loadTree(const std::string& startPath) {
        char realBuf[PATH_MAX];
        std::string realPath;
        if (realpath(startPath.c_str(), realBuf))
            realPath = realBuf;
        else
            realPath = startPath;
        std::wstring rootName = utf8_to_wstring(realPath.substr(realPath.find_last_of('/') + 1));
        if (rootName.empty()) rootName = L"/";
        root = std::make_unique<TreeNode>(realPath, rootName, true, nullptr);
        root->loadChildren();
        updateFlatList();
    }

    void loadFlatDirectory() {
        flatEntries.clear();
        DIR* dir = opendir(currentPath.c_str());
        if (!dir) {
            statusMsg = L"Error: cannot open directory " + utf8_to_wstring(currentPath) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot open directory " + currentPath + " - " + strerror(errno));
            return;
        }
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            std::string fullPath = currentPath + "/" + entry->d_name;
            struct stat st;
            if (stat(fullPath.c_str(), &st) == -1) {
                log("stat error on " + fullPath + ": " + strerror(errno));
                continue;
            }
            FileEntry fe;
            fe.name = utf8_to_wstring(entry->d_name);
            fe.isDirectory = S_ISDIR(st.st_mode);
            fe.size = st.st_size;
            fe.mode = st.st_mode;
            flatEntries.push_back(fe);
        }
        closedir(dir);
        std::sort(flatEntries.begin(), flatEntries.end(),
                  [](const FileEntry& a, const FileEntry& b) {
                      if (a.isDirectory != b.isDirectory)
                          return a.isDirectory > b.isDirectory;
                      return a.name < b.name;
                  });
        if (selectedIndex >= (int)flatEntries.size())
            selectedIndex = flatEntries.empty() ? 0 : flatEntries.size() - 1;
        if (selectedIndex < 0) selectedIndex = 0;
        scrollOffset = 0;
    }

    void displayList() {
        werase(listWin);
        int visibleRows = maxY - 3;
        int startIdx = scrollOffset;
        int endIdx;

        if (treeMode) {
            endIdx = std::min((int)flatList.size(), startIdx + visibleRows);
            for (int i = startIdx; i < endIdx; ++i) {
                TreeNode* node = flatList[i];
                int depth = 0;
                TreeNode* p = node->parent;
                while (p) { depth++; p = p->parent; }
                std::wstring indent(depth * 2, L' ');
                std::wstring prefix = node->isDirectory ? (node->expanded ? L"[-] " : L"[+] ") : L"    ";
                std::wstring line = indent + prefix + node->name;
                if ((int)line.length() > maxX - 1) line.resize(maxX - 1);
                int row = i - startIdx;
                if (i == selectedIndex) {
                    wattron(listWin, A_REVERSE);
                    mvwaddwstr(listWin, row, 0, line.c_str());
                    wattroff(listWin, A_REVERSE);
                } else {
                    mvwaddwstr(listWin, row, 0, line.c_str());
                }
            }
        } else {
            endIdx = std::min((int)flatEntries.size(), startIdx + visibleRows);
            for (int i = startIdx; i < endIdx; ++i) {
                const auto& e = flatEntries[i];
                std::wstring prefix = e.isDirectory ? L"[DIR]  " : L"[FILE] ";
                std::wstring line = prefix + e.name;
                if ((int)line.length() > maxX - 1) line.resize(maxX - 1);
                int row = i - startIdx;
                if (i == selectedIndex) {
                    wattron(listWin, A_REVERSE);
                    mvwaddwstr(listWin, row, 0, line.c_str());
                    wattroff(listWin, A_REVERSE);
                } else {
                    mvwaddwstr(listWin, row, 0, line.c_str());
                }
            }
        }
        wrefresh(listWin);
    }

    void displayStatus() {
        werase(statusWin);
        std::wstring line0, line1, line2;
        if (treeMode) {
            line0 = L"Mode: TREE";
            if (root)
                line0 += L" Path: " + utf8_to_wstring(root->fullPath);
        } else {
            line0 = L"Mode: FLAT";
            line0 += L" Path: " + utf8_to_wstring(currentPath);
        }
        line1 = L"Status: " + statusMsg;
        line2 = L"Commands: ↑↓ - navigate, Enter - open/expand, Backspace - parent, c - copy, m - move, d - delete, n - new, t - toggle tree/flat, q - quit";

        if ((int)line0.length() > maxX) line0.resize(maxX);
        if ((int)line1.length() > maxX) line1.resize(maxX);
        if ((int)line2.length() > maxX) line2.resize(maxX);

        mvwaddwstr(statusWin, 0, 0, line0.c_str());
        mvwaddwstr(statusWin, 1, 0, line1.c_str());
        mvwaddwstr(statusWin, 2, 0, line2.c_str());
        wrefresh(statusWin);
    }

    void adjustScroll() {
        int visibleRows = maxY - 3;
        if (selectedIndex < scrollOffset)
            scrollOffset = selectedIndex;
        else if (selectedIndex >= scrollOffset + visibleRows)
            scrollOffset = selectedIndex - visibleRows + 1;
        if (scrollOffset < 0) scrollOffset = 0;
        int totalItems = treeMode ? flatList.size() : flatEntries.size();
        if (scrollOffset > totalItems - visibleRows && totalItems > 0)
            scrollOffset = std::max(0, totalItems - visibleRows);
    }

    std::wstring getSelectedName() {
        if (treeMode && selectedIndex >= 0 && selectedIndex < (int)flatList.size())
            return flatList[selectedIndex]->name;
        if (!treeMode && selectedIndex >= 0 && selectedIndex < (int)flatEntries.size())
            return flatEntries[selectedIndex].name;
        return L"";
    }

    std::string getSelectedPath() {
        if (treeMode && selectedIndex >= 0 && selectedIndex < (int)flatList.size())
            return flatList[selectedIndex]->fullPath;
        if (!treeMode && selectedIndex >= 0 && selectedIndex < (int)flatEntries.size())
            return currentPath + "/" + wstring_to_utf8(flatEntries[selectedIndex].name);
        return "";
    }

    std::string getCreationPath() {
        if (treeMode) {
            if (selectedIndex >= 0 && selectedIndex < (int)flatList.size()) {
                TreeNode* node = flatList[selectedIndex];
                if (node->isDirectory)
                    return node->fullPath;
                else {
                    std::string parent = node->fullPath;
                    size_t pos = parent.find_last_of('/');
                    if (pos != std::string::npos)
                        parent = parent.substr(0, pos);
                    return parent;
                }
            }
            return root ? root->fullPath : ".";
        } else {
            return currentPath;
        }
    }

    std::string getDefaultDestinationPath() {
        if (treeMode && selectedIndex >= 0 && selectedIndex < (int)flatList.size()) {
            TreeNode* node = flatList[selectedIndex];
            if (node->isDirectory)
                return node->fullPath;
            else {
                std::string parent = node->fullPath;
                size_t pos = parent.find_last_of('/');
                if (pos != std::string::npos)
                    parent = parent.substr(0, pos);
                return parent;
            }
        }
        return currentPath;
    }

    bool isSelectedDirectory() {
        if (treeMode && selectedIndex >= 0 && selectedIndex < (int)flatList.size())
            return flatList[selectedIndex]->isDirectory;
        if (!treeMode && selectedIndex >= 0 && selectedIndex < (int)flatEntries.size())
            return flatEntries[selectedIndex].isDirectory;
        return false;
    }

    std::wstring inputStringWithDefault(const std::wstring& prompt, const std::wstring& defaultValue) {
        echo();
        curs_set(1);
        int height = 3, width = maxX - 4;
        int startY = maxY / 2 - 1, startX = 2;
        WINDOW* inputWin = newwin(height, width, startY, startX);
        box(inputWin, 0, 0);
        mvwaddwstr(inputWin, 1, 1, (prompt + L": ").c_str());
        wrefresh(inputWin);

        std::wstring current = defaultValue;
        int curPos = current.length();
        mvwaddwstr(inputWin, 1, 1 + prompt.length() + 2, current.c_str());
        wmove(inputWin, 1, 1 + prompt.length() + 2 + curPos);
        wrefresh(inputWin);

        int ch;
        while ((ch = wgetch(inputWin)) != '\n' && ch != KEY_ENTER) {
            switch (ch) {
                case KEY_LEFT:
                    if (curPos > 0) curPos--;
                    break;
                case KEY_RIGHT:
                    if (curPos < (int)current.length()) curPos++;
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (curPos > 0) {
                        current.erase(curPos - 1, 1);
                        curPos--;
                    }
                    break;
                case KEY_DC:
                    if (curPos < (int)current.length())
                        current.erase(curPos, 1);
                    break;
                case 21: // Ctrl-U
                    current.clear();
                    curPos = 0;
                    break;
                default:
                    if (ch >= 32 && ch <= 126) {
                        current.insert(curPos, 1, (wchar_t)ch);
                        curPos++;
                    }
                    break;
            }
            mvwaddwstr(inputWin, 1, 1 + prompt.length() + 2, std::wstring(width - 2 - prompt.length() - 2, L' ').c_str());
            mvwaddwstr(inputWin, 1, 1 + prompt.length() + 2, current.c_str());
            wmove(inputWin, 1, 1 + prompt.length() + 2 + curPos);
            wrefresh(inputWin);
        }

        delwin(inputWin);
        curs_set(0);
        noecho();
        return current;
    }

    bool exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    bool copyFile(const std::string& src, const std::string& dst) {
        int fdSrc = open(src.c_str(), O_RDONLY);
        if (fdSrc == -1) {
            statusMsg = L"Error: cannot open source file " + utf8_to_wstring(src) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot open source file " + src + " - " + strerror(errno));
            return false;
        }
        int fdDst = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fdDst == -1) {
            statusMsg = L"Error: cannot create destination file " + utf8_to_wstring(dst) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot create destination file " + dst + " - " + strerror(errno));
            close(fdSrc);
            return false;
        }
        char buffer[8192];
        ssize_t n;
        while ((n = read(fdSrc, buffer, sizeof(buffer))) > 0) {
            if (write(fdDst, buffer, n) != n) {
                statusMsg = L"Error: write error to " + utf8_to_wstring(dst) + L" - " + utf8_to_wstring(strerror(errno));
                log("Error: write error to " + dst + " - " + strerror(errno));
                close(fdSrc);
                close(fdDst);
                return false;
            }
        }
        if (n == -1) {
            statusMsg = L"Error: read error from " + utf8_to_wstring(src) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: read error from " + src + " - " + strerror(errno));
        }
        close(fdSrc);
        close(fdDst);
        if (n != -1) {
            log("Copied file: " + src + " -> " + dst);
        }
        return n != -1;
    }

    bool copyDirectory(const std::string& src, const std::string& dst) {
        DIR* dir = opendir(src.c_str());
        if (!dir) {
            statusMsg = L"Error: cannot open source directory " + utf8_to_wstring(src) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot open source directory " + src + " - " + strerror(errno));
            return false;
        }
        if (mkdir(dst.c_str(), 0755) == -1 && errno != EEXIST) {
            statusMsg = L"Error: cannot create destination directory " + utf8_to_wstring(dst) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot create destination directory " + dst + " - " + strerror(errno));
            closedir(dir);
            return false;
        }
        struct dirent* entry;
        bool success = true;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            std::string srcPath = src + "/" + entry->d_name;
            std::string dstPath = dst + "/" + entry->d_name;
            struct stat st;
            if (stat(srcPath.c_str(), &st) == -1) {
                log("stat error on " + srcPath + ": " + strerror(errno));
                success = false;
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                if (!copyDirectory(srcPath, dstPath))
                    success = false;
            } else {
                if (!copyFile(srcPath, dstPath))
                    success = false;
            }
        }
        closedir(dir);
        if (success) {
            log("Copied directory: " + src + " -> " + dst);
        }
        return success;
    }

    bool removeDirectory(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            statusMsg = L"Error: cannot open directory " + utf8_to_wstring(path) + L" - " + utf8_to_wstring(strerror(errno));
            log("Error: cannot open directory " + path + " - " + strerror(errno));
            return false;
        }
        struct dirent* entry;
        bool success = true;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            std::string fullPath = path + "/" + entry->d_name;
            struct stat st;
            if (stat(fullPath.c_str(), &st) == -1) {
                log("stat error on " + fullPath + ": " + strerror(errno));
                success = false;
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                if (!removeDirectory(fullPath))
                    success = false;
            } else {
                if (unlink(fullPath.c_str()) == -1) {
                    log("unlink error on " + fullPath + ": " + strerror(errno));
                    success = false;
                }
            }
        }
        closedir(dir);
        if (rmdir(path.c_str()) == -1) {
            log("rmdir error on " + path + ": " + strerror(errno));
            success = false;
        }
        if (success) {
            log("Removed directory: " + path);
        }
        return success;
    }

    void refreshAfterResize() {
        getmaxyx(stdscr, maxY, maxX);
        if (listWin) delwin(listWin);
        if (statusWin) delwin(statusWin);
        listWin = newwin(maxY-3, maxX, 0, 0);
        statusWin = newwin(3, maxX, maxY-3, 0);
        scrollok(listWin, FALSE);
        adjustScroll();
        displayList();
        displayStatus();
    }

public:
    FileManager(const std::string& startPath) : selectedIndex(0), scrollOffset(0), treeMode(true) {
        char realBuf[PATH_MAX];
        if (realpath(startPath.c_str(), realBuf))
            currentPath = realBuf;
        else
            currentPath = startPath;
        loadTree(currentPath);
        loadFlatDirectory();
    }

    void run() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        getmaxyx(stdscr, maxY, maxX);
        clear();
        refresh();

        listWin = newwin(maxY-3, maxX, 0, 0);
        statusWin = newwin(3, maxX, maxY-3, 0);
        scrollok(listWin, FALSE);
        adjustScroll();
        displayList();
        displayStatus();

        int ch;
        while ((ch = getch()) != 'q') {
            if (ch == KEY_RESIZE) {
                refreshAfterResize();
                continue;
            }

            int cmd = ch;
            if (ch >= 'A' && ch <= 'Z') cmd = tolower(ch);

            switch (cmd) {
                case KEY_UP:
                    if (selectedIndex > 0) { --selectedIndex; adjustScroll(); }
                    break;
                case KEY_DOWN:
                    if (treeMode && selectedIndex < (int)flatList.size()-1) {
                        ++selectedIndex; adjustScroll();
                    } else if (!treeMode && selectedIndex < (int)flatEntries.size()-1) {
                        ++selectedIndex; adjustScroll();
                    }
                    break;
                case '\n':
                case KEY_ENTER:
                    if (treeMode) {
                        if (selectedIndex >= 0 && selectedIndex < (int)flatList.size()) {
                            TreeNode* node = flatList[selectedIndex];
                            if (node->isDirectory) {
                                if (node->expanded)
                                    node->unloadChildren();
                                else
                                    node->loadChildren();
                                updateFlatList();
                                for (size_t i = 0; i < flatList.size(); ++i) {
                                    if (flatList[i] == node) {
                                        selectedIndex = i;
                                        break;
                                    }
                                }
                                adjustScroll();
                            } else {
                                statusMsg = L"Selected file: " + node->name;
                            }
                        }
                    } else {
                        if (selectedIndex >= 0 && selectedIndex < (int)flatEntries.size() &&
                            flatEntries[selectedIndex].isDirectory) {
                            std::string newPath = currentPath + "/" + wstring_to_utf8(flatEntries[selectedIndex].name);
                            currentPath = newPath;
                            selectedIndex = 0;
                            loadFlatDirectory();
                        } else if (selectedIndex >= 0 && selectedIndex < (int)flatEntries.size() &&
                                   !flatEntries[selectedIndex].isDirectory) {
                            statusMsg = L"Selected file: " + flatEntries[selectedIndex].name;
                        }
                    }
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (treeMode) {
                        if (root && root->fullPath != "/") {
                            std::string parent = currentPath;
                            size_t pos = parent.find_last_of('/');
                            if (pos != std::string::npos && pos > 0)
                                parent = parent.substr(0, pos);
                            else
                                parent = "/";
                            currentPath = parent;
                            loadTree(currentPath);
                            loadFlatDirectory();
                            selectedIndex = 0;
                            adjustScroll();
                        } else {
                            statusMsg = L"Already at root";
                        }
                    } else {
                        std::string parent = currentPath;
                        size_t pos = parent.find_last_of('/');
                        if (pos != std::string::npos && pos > 0)
                            parent = parent.substr(0, pos);
                        else
                            parent = "/";
                        if (parent != currentPath) {
                            currentPath = parent;
                            selectedIndex = 0;
                            loadFlatDirectory();
                        } else {
                            statusMsg = L"Already at root";
                        }
                    }
                    break;
                case 't':
                    treeMode = !treeMode;
                    selectedIndex = 0;
                    scrollOffset = 0;
                    if (treeMode) updateFlatList();
                    else loadFlatDirectory();
                    break;
                case 'c': {
                    std::string src = getSelectedPath();
                    if (src.empty()) {
                        statusMsg = L"No file selected";
                        break;
                    }
                    std::string defaultDst = getDefaultDestinationPath();
                    std::wstring dstW = inputStringWithDefault(L"Enter destination path", utf8_to_wstring(defaultDst));
                    if (dstW.empty()) {
                        statusMsg = L"Copy cancelled";
                        break;
                    }
                    std::string dst = wstring_to_utf8(dstW);
                    if (exists(dst)) {
                        statusMsg = L"Error: destination already exists";
                    } else {
                        bool ok = isSelectedDirectory() ? copyDirectory(src, dst) : copyFile(src, dst);
                        if (ok) {
                            statusMsg = L"Copied successfully";
                            if (treeMode) {
                                loadTree(currentPath);
                                updateFlatList();
                            } else {
                                loadFlatDirectory();
                            }
                        }
                    }
                    break;
                }
                case 'm': {
                    std::string src = getSelectedPath();
                    if (src.empty()) {
                        statusMsg = L"No file selected";
                        break;
                    }
                    std::wstring defaultName = getSelectedName();
                    std::wstring dstW = inputStringWithDefault(L"Enter new name/path", defaultName);
                    if (dstW.empty()) {
                        statusMsg = L"Move cancelled";
                        break;
                    }
                    std::string dst = wstring_to_utf8(dstW);
                    if (rename(src.c_str(), dst.c_str()) == -1) {
                        statusMsg = L"Error moving: " + utf8_to_wstring(strerror(errno));
                        log("Error moving: " + std::string(strerror(errno)));
                    } else {
                        statusMsg = L"Moved successfully";
                        log("Moved " + src + " -> " + dst);
                        if (treeMode) {
                            loadTree(currentPath);
                            updateFlatList();
                        } else {
                            loadFlatDirectory();
                        }
                    }
                    break;
                }
                case 'd': {
                    std::string target = getSelectedPath();
                    if (target.empty()) {
                        statusMsg = L"No file selected";
                        break;
                    }
                    std::string confirm = wstring_to_utf8(inputStringWithDefault(L"Confirm delete (y/n)", L""));
                    if (confirm == "y" || confirm == "Y") {
                        bool ok = isSelectedDirectory() ? removeDirectory(target) : (unlink(target.c_str()) == 0);
                        if (ok) {
                            statusMsg = L"Deleted successfully";
                            log("Deleted " + target);
                            if (treeMode) {
                                loadTree(currentPath);
                                updateFlatList();
                            } else {
                                loadFlatDirectory();
                            }
                        } else {
                            statusMsg = L"Error deleting: " + utf8_to_wstring(strerror(errno));
                            log("Error deleting: " + std::string(strerror(errno)));
                        }
                    } else {
                        statusMsg = L"Delete cancelled";
                    }
                    break;
                }
                case 'n': {
                    std::string type = wstring_to_utf8(inputStringWithDefault(L"Enter type (f=file, d=dir)", L""));
                    std::string creationPath = getCreationPath();
                    if (type == "f") {
                        std::string name = wstring_to_utf8(inputStringWithDefault(L"Enter file name", L""));
                        if (!name.empty()) {
                            std::string fullPath = creationPath + "/" + name;
                            int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
                            if (fd == -1) {
                                statusMsg = L"Error creating file: " + utf8_to_wstring(strerror(errno));
                                log("Error creating file: " + std::string(strerror(errno)));
                            } else {
                                close(fd);
                                statusMsg = L"File created";
                                log("Created file " + fullPath);
                                if (treeMode) {
                                    loadTree(currentPath);
                                    updateFlatList();
                                } else {
                                    loadFlatDirectory();
                                }
                            }
                        }
                    } else if (type == "d") {
                        std::string name = wstring_to_utf8(inputStringWithDefault(L"Enter directory name", L""));
                        if (!name.empty()) {
                            std::string fullPath = creationPath + "/" + name;
                            if (mkdir(fullPath.c_str(), 0755) == -1) {
                                statusMsg = L"Error creating directory: " + utf8_to_wstring(strerror(errno));
                                log("Error creating directory: " + std::string(strerror(errno)));
                            } else {
                                statusMsg = L"Directory created";
                                log("Created directory " + fullPath);
                                if (treeMode) {
                                    loadTree(currentPath);
                                    updateFlatList();
                                } else {
                                    loadFlatDirectory();
                                }
                            }
                        }
                    } else {
                        statusMsg = L"Invalid type";
                    }
                    break;
                }
                default:
                    break;
            }
            displayList();
            displayStatus();
        }

        endwin();
    }
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));

    logFile.open("file_manager.log", std::ios::app);
    if (!logFile.is_open())
        std::cerr << "Warning: Could not open log file" << std::endl;

    std::string startPath = ".";
    if (argc > 1)
        startPath = argv[1];

    FileManager fm(startPath);
    fm.run();

    if (logFile.is_open()) logFile.close();
    return 0;
}