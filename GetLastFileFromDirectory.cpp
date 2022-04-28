//============================================================================
// Name        : GetLastFileFromDirectory.cpp
// Author      : 
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>

#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include <map>
#include <string>
using namespace std;

struct mytimespec:public timespec
{
	mytimespec(timespec& t){*this = t;}
	mytimespec(){tv_sec=0; tv_nsec=0;}

	bool operator<(const mytimespec& t2)
	const {
		if(tv_sec < t2.tv_sec)
			return true;
		if(tv_sec == t2.tv_sec && tv_nsec < t2.tv_nsec)
			return true;
		return false;
	}

/*	bool operator<(const timespec& t2)
	const {
		if(tv_sec < t2.tv_sec)
			return true;
		if(tv_sec == t2.tv_sec && tv_nsec < t2.tv_nsec)
			return true;
		return false;
	}*/

	mytimespec& operator=(const timespec& t2)
	{
		tv_sec = t2.tv_sec;
		tv_nsec = t2.tv_nsec;
		return *this;
	}
};

map<struct mytimespec, string> mapExistingFiles;

#define DIRNAME "/tmp/joptris"

int main()
{
	printf("Берём второй по давности файл с расширением .g8 .rgb из каталога " DIRNAME ", более старые - удаляем.\n");

/*
 * Читаем все файлы в каталоге и запоминаем самое большое время модификации
 */

	char szFullName[256];
	strcpy(szFullName, DIRNAME);
	strcat(szFullName, "/");
	int posAppend = strlen(szFullName);

	// while(1)
	for(int i=0; i<3; i++)
	{
		usleep(10000);

		struct mytimespec tsNewest /*= {0,0}*/, tsSecond /*= {0,0}*/;

		DIR *dir;
		struct dirent *ent;

		dir = opendir(DIRNAME);
		while ((ent=readdir(dir)) != NULL)
		{
			printf("%s\n", ent->d_name);

			// Сравниваем только файлы с известными расширениями
			if(strstr(ent->d_name, ".rgb") != NULL || strstr(ent->d_name, ".g8") != NULL)
			{
				strcpy(szFullName + posAppend, ent->d_name); // Чтобы каждый раз рне копировать строки без толку, меняем только имя файла

				struct stat st;
				if(stat(szFullName, &st) == -1)
					fprintf(stderr, "stat() error %d\r\n", errno);
				else
				{
					printf("\tTime of last access %ld.%09ld\n", st.st_atim.tv_sec, st.st_atim.tv_nsec);
					printf("\tTime of last modification %ld.%09ld\n", st.st_mtim.tv_sec, st.st_mtim.tv_nsec);
					printf("\tTime of last status change %ld.%09ld\n", st.st_ctim.tv_sec, st.st_ctim.tv_nsec);

					mapExistingFiles[(mytimespec)(st.st_mtim)] = szFullName;

					if( tsNewest < st.st_mtim )
					{
						tsSecond = tsNewest;   // Сдвигаем предыдущий "самый новый" файл вниз
						tsNewest = st.st_mtim;
					}
					else if(tsSecond < st.st_mtim) 	// В случае, когда каталог читается от нового файла к старому,
					{										// второй файл никогда не инициализируется
						tsSecond = st.st_mtim;
					}
				}
			}
		}
		closedir(dir);

		if(mapExistingFiles.size() <2)
			printf("Недостаточно файлов\n");
		else
		{
			printf("Второй файл - %s\n", mapExistingFiles[tsSecond].c_str());
			// Убираем из мапа два самых новых файла, остальные удалим с диска по именам
			mapExistingFiles.erase(tsNewest);
			mapExistingFiles.erase(tsSecond);
		}
	}

	if(mapExistingFiles.empty() == false)
	{
		printf("Map has %d elements\n", mapExistingFiles.size());

		map<struct mytimespec, string>::iterator it = mapExistingFiles.begin();
		while(it != mapExistingFiles.end())
		{
			printf("%s modtime is %d.%09d\n", it->second.c_str(), it->first.tv_sec, it->first.tv_nsec);
			unlink(it->second.c_str());
			it++;
		}
	}

	return 0;
}

