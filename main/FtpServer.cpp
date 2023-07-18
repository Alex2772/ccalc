//
// Created by alex2772 on 21.04.18.
//

#include <cstring>
#include <sys/stat.h>
#include <map>
#include <lwip/sockets.h>
#include "FtpServer.h"
#include "Socket.h"
#include "config.h"
#include "Wifi.h"
#include "File.h"
#include "CCOSCore.h"
#include "Process.h"
#include "FirmwareConfig.h"

extern std::map<lua_State*, Process*> processes;

FtpServer::FtpServer() {
/*
    debugger = new TaskHelper([&]() {
        ServerSocket s(2228);
        while (Socket *client = s.accept()) {
            CCOSCore::runOnUiThread([]() {
                for (auto& p : processes) {
                    if (p.second)
                        CCOSCore::killMePlz("", p.first);
                }
            });
            File f("remote.lua");
            f.open(File::W);

            int read;
            char buf[4096];
            while ((read = client->read_bytes(buf, sizeof(buf)))) {
                f.write(buf, read);
            }

            f.close();

            CCOSCore::execute(f);
            delete client;
        }
    });
*/
    acceptor = new TaskHelper([&]() {
        ServerSocket s(21);
        while (Socket* client = s.accept()) {
            pool.push_back(new TaskHelper([client]() {
                {
                    char c[128];
                    sprintf(c, "220 %s internal FTP server\r\n", FirmwareConfig::read().mName);
                    client->write_bytes(c, strlen(c));
                }
                std::string currentDir = "/";
                ServerSocket* ps = nullptr;
                while (client->isOk()) {
                    std::string str;
                    while (client->isOk()) {
                        char buf;
                        if (client->read_bytes(&buf, 1) == 0) {
                            goto ex;
                        }
                        if (buf == '\r')
                            break;
                        str += buf;
                    }

                    printf("%s\n", str.c_str());

                    char cmd[128];

                    sscanf(str.c_str(), "%s", cmd);
                    if (!strcmp(cmd, "USER")) {
                        const char *c = "230 User ok, proceed\r\n";
                        client->write_bytes(c, strlen(c));
                    } else if (!strcmp(cmd, "PWD")) {
                        char buf[512];
                        sprintf(buf, "257 \"%s\" current dir\r\n", currentDir.c_str());
                        client->write_bytes(buf, strlen(buf));
                    } else if (!strcmp(cmd, "TYPE")) {
                        const char *c = "200 Binary mode\r\n";
                        client->write_bytes(c, strlen(c));
                    } else if (!strcmp(cmd, "PASV")) {
                        while(ps);
                        char buf[512];
                        Wifi::ip_address s = Wifi::getIpAddress();
                        sprintf(buf, "227 Passive mode (%d,%d,%d,%d,%d,%d)\r\n", s.b1, s.b2, s.b3, s.b4, 233, 160);
                        client->write_bytes(buf, strlen(buf));
                        ps = new ServerSocket(233 * 256 + 160);
                    } else if (!strcmp(cmd, "LIST")) {
                        if (ps) {
                            if (Socket *sc = ps->accept()) {
                                const char *c = "150 Directory list\r\n";
                                client->write_bytes(c, strlen(c));
                                Dir d(currentDir.substr(0, currentDir.length() - 1));
                                std::vector<File> files;
                                d.list(files);
                                for (auto &f : files) {
                                    char buf[256];
                                    size_t s = f.isDir() ? 4096 : f.size();
                                    char t[64];
                                    struct stat st;
                                    f.stat(st);
                                    strftime(t, 64, "%b %d %H:%M", localtime(&st.st_mtime));
                                    sprintf(buf, "%crwxrwxrwx %d 1000 1000 %u %s %s\r\n", f.isDir() ? 'd' : '-',
                                            f.isDir() ? 2 : 1, s, t, f.getFilename().c_str());
                                    printf(buf);
                                    sc->write_bytes(buf, strlen(buf));
                                }
                                c = "226 Directory sent\r\n";
                                client->write_bytes(c, strlen(c));
                                delete sc;
                                delete ps;
                                ps = nullptr;
                            }
                        }
                    } else if (!strcmp(cmd, "RETR")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        if (ps) {
                            if (Socket *sc = ps->accept()) {

                                f.open(File::R);
                                f.seek(0, SEEK_END);
                                long len = f.tellg();
                                f.seek(0, SEEK_SET);
                                sprintf(buf, "150 Opening file (%ld bytes)\r\n", len);
                                client->write_bytes(buf, strlen(buf));
                                long read = 0;
                                while (read < len) {
                                    char tmp[2048];
                                    size_t r = f.read(tmp, sizeof(tmp));
                                    sc->write_bytes(tmp, r);
                                    read += r;
                                }

                                sprintf(buf, "226 Transfer complete\r\n");
                                client->write_bytes(buf, strlen(buf));
                                delete sc;
                            }
                            delete ps;
                            ps = nullptr;
                        }
                    } else if (!strcmp(cmd, "SIZE")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        sprintf(buf, "213 %u\r\n", f.size());
                        client->write_bytes(buf, strlen(buf));
                    } else if (!strcmp(cmd, "MDTM")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        struct stat st;
                        f.stat(st);
                        sprintf(buf, "213 %lu\r\n", st.st_mtime);
                        client->write_bytes(buf, strlen(buf));
                    } else if (!strcmp(cmd, "STOR")) {
                        const char *c = "150 Ready to receive\r\n";
                        client->write_bytes(c, strlen(c));
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        f.open(File::W);

                        if (ps) {
                            if (Socket *sc = ps->accept()) {
                                char buf[1024];
                                size_t read = 0;
                                while ((read = sc->read_bytes(buf, sizeof(buf)))) {
                                    f.write(buf, read);
                                }
                                delete sc;
                            }
                            delete ps;
                            ps = nullptr;
                        }
                        c = "226 Transfer complete\r\n";
                        client->write_bytes(c, strlen(c));
                    } else if (!strcmp(cmd, "CWD")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        std::string path;
                        if (buf[0] != '/') {
                            path = currentDir + buf;
                        } else {
                            path = buf;
                        }
                        if (path.back() != '/') {
                            path.push_back('/');
                        }
                        Dir fi(path.substr(0, path.length() - 1));
                        if (fi.isDir()) {
                            const char *c = "250 Directory changed successly\r\n";
                            client->write_bytes(c, strlen(c));
                            currentDir = path;
                        } else {
                            const char *c = "550 No such directory\r\n";
                            client->write_bytes(c, strlen(c));
                        }
                    } else if(!strcmp(cmd, "CDUP")) {
                        if (currentDir == "/") {
                            const char *c = "550 You are in root directory\r\n";
                            client->write_bytes(c, strlen(c));
                        } else {
                            std::string path = currentDir.substr(0, currentDir.length() - 1);
                            path = path.substr(0, path.find_last_of("/"));
                            if (path.back() != '/') {
                                path.push_back('/');
                            }
                            Dir fi(path.substr(0, path.length() - 1));
                            if (fi.isDir()) {
                                const char *c = "250 Directory changed successly\r\n";
                                client->write_bytes(c, strlen(c));
                                currentDir = path;
                            } else {
                                const char *c = "550 No such directory\r\n";
                                client->write_bytes(c, strlen(c));
                            }
                        }
                    } else if (!strcmp(cmd, "DELE") || !strcmp(cmd, "RMD")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        if (f.isDir() || f.isFile()) {
                            f.remove();
                            const char *c = "250 Successly deleted\r\n";
                            client->write_bytes(c, strlen(c));
                        } else {
                            const char *c = "550 No such file or directory\r\n";
                            client->write_bytes(c, strlen(c));
                        }
                    } else if (!strcmp(cmd, "MKD")) {
                        char buf[256];
                        sscanf(str.c_str(), "%s%s", buf, buf);
                        File f;
                        if (buf[0] != '/') {
                            f = File(currentDir + buf);
                        } else {
                            f = File(buf);
                        }
                        if (f.isDir() || f.isFile()) {
                            const char *c = "550 Already exists\r\n";
                            client->write_bytes(c, strlen(c));
                        } else {
                            Dir(f.getPath()).mkdir(0777);
                            const char *c = "250 Directory created successly\r\n";
                            client->write_bytes(c, strlen(c));
                        }
                    } else {
                        const char* c = "502 Command not implemented\r\n";
                        client->write_bytes(c, strlen(c));
                    }
                }
                ex:
                if (ps)
                    delete ps;
                printf("Client disconnected\n");
                delete client;
            }));

        }
    });
}

FtpServer::~FtpServer() {
    for (TaskHelper* t : pool)
        delete t;
    delete acceptor;
    //delete debugger;
}
