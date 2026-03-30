#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QFile>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_chooseFileButton_clicked();

    void on_encryptButton_clicked();

    void on_decryptButton_clicked();

    void on_obfuscateButton_clicked();

private:
    Ui::MainWindow *ui;
    bool isObfuscated=false;

    void successStatus(QString);
    void errorStatus(QString);
    void obfuscate();
    void crypto(bool encrypt=true);
    void rename_identifiers(QString& code);
    void insert_dead_code(QString& code);
    QString readFile();


    const QByteArray key = "secret_key_1231232132asfnpdasfdjopasdja jop32je 2joe323123 sdw123q2 ebjkadbnlk dahDWD WHIWHOI DWHIDW HDI'HDE12OEHOI12H312";
    const std::set<std::string> keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
        "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
        "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
        "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await",
        "co_return", "co_yield", "decltype", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
        "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace",
        "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
        "protected", "public", "register", "reinterpret_cast", "requires", "return", "short",
        "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
        "synchronized", "template", "this", "thread_local", "throw", "true", "try", "typedef",
        "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
        "wchar_t", "while", "xor", "xor_eq",
        "std", "string", "vector", "map", "set", "unordered_map", "unordered_set",
        "list", "deque", "array", "pair", "tuple", "optional", "variant", "any",
        "unique_ptr", "shared_ptr", "weak_ptr", "make_shared", "make_unique",
        "cout", "cin", "cerr", "clog", "endl", "flush", "ios", "fstream", "ifstream",
        "ofstream", "sstream", "to_string", "stoi", "stol", "stoll", "stof", "stod",
        "getline", "sort", "find", "transform", "copy", "accumulate", "algorithm",
        "iterator", "functional", "memory", "utility", "thread", "mutex", "lock_guard",
        "unique_lock", "condition_variable", "chrono", "random", "regex", "exception",
        "runtime_error", "logic_error", "invalid_argument", "out_of_range", "length_error",
        "bad_alloc", "bad_cast", "type_info", "initializer_list", "nullptr_t", "size_t",
        "ptrdiff_t", "nullptr",
        "socket", "bind", "listen", "accept", "connect", "recv", "send", "close",
        "setsockopt", "getsockopt", "htonl", "htons", "ntohl", "ntohs", "inet_addr",
        "inet_ntoa", "gethostbyname", "select", "poll", "epoll", "fcntl", "ioctl",
        "sockaddr", "sockaddr_in", "sockaddr_un", "in_addr", "in6_addr", "AF_INET",
        "AF_INET6", "AF_UNIX", "SOCK_STREAM", "SOCK_DGRAM", "SOCK_RAW", "SOL_SOCKET",
        "SO_REUSEADDR", "SO_REUSEPORT", "SO_KEEPALIVE", "SO_LINGER", "TCP_NODELAY",
        "INADDR_ANY", "INADDR_LOOPBACK", "shutdown", "fork", "execvp", "waitpid",
        "pipe", "dup", "dup2", "open", "read", "write", "lseek", "stat", "fstat",
        "opendir", "readdir", "closedir", "pthread_create", "pthread_join", "pthread_mutex",
        "pthread_cond", "signal", "sigaction", "getpid", "getppid", "getuid", "geteuid",
        "perror", "exit", "abort", "atexit", "malloc", "free", "calloc", "realloc",
        "memcpy", "memset", "memcmp", "strcpy", "strncpy", "strcat", "strcmp", "strlen",
        "printf", "fprintf", "sprintf", "snprintf", "scanf", "fscanf", "sscanf",
        "fopen", "fclose", "fread", "fwrite", "fseek", "ftell", "rewind", "feof", "ferror",
        "main", "nullptr", "bool", "true", "false","include","iostream"
    };
};
#endif // MAINWINDOW_H
