/*
  FileDB.h - Library for creating simple databases.
  Created by Nicholas Pitt, February 26, 2015.
  Released under the LGPL license.
*/

#ifndef FileDB_H
#define FileDB_H

#include "Arduino.h"
#include "SD.h"

struct Record
{
	bool used;
};

template <class T, typename N>
class FileDB
{
	public:
		typedef bool KeysMatch(N *, N *);

		FileDB(int records, KeysMatch *keysMatch) : records(records), keysMatch(keysMatch) {};

		bool begin(char *filePath)
		{
			file = SD.open(filePath, FILE_WRITE);
			if (!file)
			{
				return false;
			}
			file.seek(0);
			for (int i = 0; i < records; i++)
			{
				for (int j = 0; j < sizeof(T); j++)
				{
					file.write((byte)'\0');
				}
				file.flush();
			}
			return true;
		}

		bool add(T &newRecord)
		{
			T record;

			file.seek(0);
			for (int i = 0; i < records; i++)
			{
				next(record);
				if (!record.used || keysMatch(record.key, newRecord.key))
				{
					file.seek(i * sizeof(T));
					newRecord.used = true;
					file.write(raw(newRecord), sizeof(T));
					file.flush();
					return true;
				}
			}
			return false;
		};

		bool get(N *key, T &record)
		{
			file.seek(0);
			for (int i = 0; i < records; i++)
			{
				next(record);
				if (record.used && keysMatch(record.key, key))
				{
					return true;
				}
			}
			return false;
		}

		bool del(N *key)
		{
			T record;

			file.seek(0);
			for (int i = 0; i < records; i++)
			{
				next(record);
				if (record.used && keysMatch(record.key, key))
				{
					file.seek(i * sizeof(T));
					record.used = false;
					file.write(raw(record), sizeof(T));
					file.flush();
					return true;
				}
			}
			return false;
		};

	private:
		const int records;
		KeysMatch *keysMatch;
		File file;

		byte *raw(T &record) { return (byte *)(&record); }

		void next(T &record)
		{
			byte *rawRecord = raw(record);

			for (int i = 0; i < sizeof(T); i++)
			{
				rawRecord[i] = file.read();
			}
		};
};

#endif