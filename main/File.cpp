#include <sys/stat.h>
#include <cstdio>
#include <dirent.h>
#include "File.h"

bool File::isFile() const
{

    struct stat info;

    int s = stat(info);
    printf("stat %s %d %d\n", mPath.c_str(), s, info.st_mode);
    return s == 0 && (info.st_mode & S_IFREG);
}

void File::open(Open t)
{
    if (t & Open::W) {
        if (t & Open::R) {
            fil = fopen(convert().c_str(), "w+b");
        } else {
            fil = fopen(convert().c_str(), "wb");
        }
    } else if (t & Open::R) {
		fil = fopen(convert().c_str(), "rb");
	}
    opened = true;
}

std::string File::getFilename() const
{
	size_t t = convert().find_last_of('/');
	if (t != std::string::npos) {
		if (t < convert().length())
			t++;
		return convert().substr(t);
	}
	return convert();
}
void Dir::list(std::vector<File>& f)
{
	if (isDir()) {
        std::string s = convert();
        if (s[s.length() - 1] == '/' && s[s.length() - 2] != '\\') {
            s.resize(s.length() - 1);
        }
		DIR* d = opendir(s.c_str());
        if (d) {
            struct dirent *p = readdir(d);
            while (p != nullptr) {
                std::string px = getPath();
                if (px[px.length() - 1] == '/' && px[px.length() - 2] != '\\') {
                    px.resize(px.length() - 1);
                }
                px += "/";
                f.push_back(File(px + (char *) p->d_name));
                p = readdir(d);
            }
            closedir(d);
        }
	}
}

int Dir::mkdir(int mode) {
    std::string s = convert();
    if (s[s.length() - 1] == '/' && s[s.length() - 2] != '\\') {
        s.resize(s.length() - 1);
    }
    return ::mkdir(s.c_str(), mode);
}

std::string File::convert() const {
    if (!mPath.empty() && mPath != "/") {
        if (mPath[0] == '/') {
            return std::string("/spiffs/ccos") + mPath;
        } else {
            return std::string("/spiffs/ccos/") + mPath;
        }
    }
    return std::string("/spiffs/ccos");
}

bool File::isDir() const
{
	struct stat info;

    int s = stat(info);
	return s == 0 && !(info.st_mode & S_IFREG);
}

size_t File::write(const char *buf, size_t size) {
	return fwrite(buf, sizeof(char), size, fil);
}

size_t File::read(char *buf, size_t size) {
	return fread(buf, sizeof(char), size, fil);
}

void File::close() {
    if (isOpened())
    {
        fclose(fil);
        opened = false;
    }
}

bool File::isOpened() const {
    return opened && fil;
}

int File::stat(struct stat & s) const {
    int v = ::stat(convert().c_str(), &s);
    return v;
}

const std::string& File::getPath() const {
    return mPath;
}

File::File(const File &o) {
    mPath = o.mPath;
}

void File::remove() {
    if (isDir()) {
        Dir d(mPath);
        std::vector<File> files;
        d.list(files);
        for (auto& f : files) {
            f.remove();
        }
    }
    ::remove(convert().c_str());
}

int File::seek(long l, int k) {
    return fseek(fil, l, k);
}
long File::tellg() {
    return ftell(fil);
}

size_t File::size() {
    size_t s = 0;
    if (isDir()) {
        Dir d(mPath);
        std::vector<File> files;
        d.list(files);
        for (File& f : files) {
            s += f.size();
        }
    }
    struct stat st;
    stat(st);
    s += st.st_size;
    return s;
}