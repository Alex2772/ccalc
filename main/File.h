#pragma once

#include <string>
#include <vector>


class File
{
protected:
	std::string mPath;
	bool opened = false;
	FILE *fil;
public:
	File(const std::string& s):
		mPath(s)
	{
		
	}
	File() {}
    File(const File& o);

    void close();
	~File()
	{
        close();
	}

    const std::string& getPath() const;

    std::string convert() const;
	size_t write(const char* buf, size_t size);
	size_t read(char* buf, size_t size);

	std::string getFilename() const;


	bool isOpened() const;
	bool isFile() const;
	bool isDir() const;

    int stat(struct stat& s) const;


	void remove();

	int seek(long l, int k);

enum Open
{
	R = 1,
	W = 2,
};
	void open(Open t);

	long tellg();

    size_t size();

	template<typename T>
	size_t read(T& data) {
		return read(reinterpret_cast<char*>(&data), sizeof(data));
	}
	template<typename T>
	size_t write(T& data) {
		return write(reinterpret_cast<const char*>(&data), sizeof(data));
	}
};

class Dir : public File
{
public:
	Dir(const std::string s):
	File(s)
	{
		
	}

	void list(std::vector<File>& f);

    int mkdir(int mode);
};
