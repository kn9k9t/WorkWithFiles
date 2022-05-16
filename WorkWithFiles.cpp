#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <Windows.h>
using namespace std;

class Exception
{
public:
	enum ErrorCode
	{
		ERROR_OPEN_FILE,
		ERROR_WRITE_FILE,
		EMPTY_FILE,
		NO_SEPARATOR,
		WRONG_STRUCTURE,
		NUMBER_COLUMN
	};
	Exception(ErrorCode code, const string& message) 
	{
		this->code = code;
		this->message = message;
	}
	ErrorCode GetErrorCode()
	{
		return code;
	}
	string GetErrorMessage()
	{
		return message;
	}
private:
	ErrorCode code;
	string message;
};
class FileHandler
{
public:
	vector<vector<string>> ReadFile(const string &path, const char &separator = ',')
	{
		try
		{
			ifstream input_stream(path);
			if (input_stream.fail())
			{
				throw Exception(Exception::ERROR_OPEN_FILE, "can't open file: " + path);
			}
			vector<vector<string>> file_elements;
			vector<string> row_elements;
			unsigned int column_count = 0;
			string row;
			unsigned int row_number = 1;
			while (getline(input_stream, row))
			{
				if (!row.empty())
				{
						row_elements = this->SeparateLine(row, separator, row_number);
						if (row_number == 1)
						{
							column_count = row_elements.size();
							this->files_elements.push_back(row_elements);
							row_number++;
							continue;
						}
						if (column_count != row_elements.size())
						{
							int considered_count = row_elements.size();
							throw Exception(Exception::WRONG_STRUCTURE, "row " + to_string(row_number) + " - wrong column count: expected " + to_string(column_count) + ", considered " + to_string(considered_count));
						}
						this->files_elements.push_back(row_elements);
				}
				row_number++;
			}
			if (this->files_elements.empty())
			{
				throw Exception(Exception::EMPTY_FILE, "file: '" + path + "' is empty");
			}
			cout << "read file: '" + path + "'" << endl;
			return this->files_elements;
		}
		catch (Exception err)
		{
			cout << "Error code #" << err.GetErrorCode() << " : " << err.GetErrorMessage() << endl;
		}
	}
	bool WriteFile(const string& path, const unsigned int& column_number, const string& value)
	{
		try
		{
			ofstream output_stream(path);
			if (output_stream.fail())
			{
				throw Exception(Exception::ERROR_WRITE_FILE, "can't write in file: " + path);
			}
			for (vector<string> row : this->files_elements)
			{
				if (column_number > row.size() - 1)
				{
					throw Exception(Exception::NUMBER_COLUMN, "there is no " + to_string(column_number + 1) + " column in '" + path + "'");
				}
				if (row[column_number] == value)
				{
					for (int i = 0; i < row.size(); i++)
					{
						if (i == row.size() - 1)
						{
							output_stream << row[i];
						}
						else
						{
							output_stream << row[i] << ";";
						}
					}
					output_stream << endl;
				}
			}
			output_stream.close();
			cout << "'" + path << "' is wrote" << endl;
			return true;
		}
		catch (Exception err)
		{
			cout << "Error code #" << err.GetErrorCode() << " : " << err.GetErrorMessage() << endl;
		}
	}
	vector<filesystem::path> ParseFiles(const string& path)
	{
		vector<filesystem::path> input_files;
		auto files = filesystem::directory_iterator(path);
		copy_if(filesystem::begin(files), filesystem::end(files), back_inserter(input_files),
			[](const auto& entry) {
				return filesystem::is_regular_file(entry);
			});
		return input_files;
	}
protected:
	vector<string> SeparateLine(const string& row, const char& separator = ',', const unsigned int& row_number = 0)
	{
		if (row.find(separator) == -1)
		{
			throw Exception(Exception::NO_SEPARATOR, "row " + to_string(row_number) + " - hasn't separators");
		}
		vector<string> row_elements;
		string element;
		for (char c : row)
		{
			if (c == separator)
			{
				if (!element.empty())
				{
					row_elements.push_back(element);
					element.clear();
				}
				else
				{
					continue;
				}
			}
			else
			{
				element.push_back(c);
			}
		}
		if (!element.empty())
		{
			row_elements.push_back(element);
		}
		return row_elements;
	}
	vector<vector<string>> files_elements;
};

int main()
{
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
	FileHandler handler;
	vector<filesystem::path> start_files = handler.ParseFiles("files\\input");
	vector<filesystem::file_time_type> last_updated_files;
	for_each(start_files.begin(), start_files.end(), [&handler, &start_files, &last_updated_files](filesystem::path path) 
		{ 
			handler.ReadFile(path.string(), ',');
			last_updated_files.push_back(filesystem::last_write_time(path));
		});
	handler.WriteFile("files\\output\\output1.txt", 2, "producer1");
	handler.WriteFile("files\\output\\output2.txt", 2, "producer3");
	handler.WriteFile("files\\output\\output3.txt", 0, "product1");
	while (true)
	{
		Sleep(50);
		start_files = handler.ParseFiles("files\\input");
		vector<filesystem::file_time_type> updated_files;
		for (auto& file : start_files)
		{
			if (filesystem::exists(file))
			{
				updated_files.push_back(filesystem::last_write_time(file));
			}
		}
		if (last_updated_files != updated_files)
		{
			for_each(start_files.begin(), start_files.end(), [&handler, &start_files, &last_updated_files](filesystem::path path)
				{
					handler.ReadFile(path.string(), ',');
					last_updated_files.push_back(filesystem::last_write_time(path));
				});
			handler.WriteFile("files\\output\\output1.txt", 2, "producer1");
			handler.WriteFile("files\\output\\output2.txt", 2, "producer3");
			handler.WriteFile("files\\output\\output3.txt", 0, "product1");
			last_updated_files = updated_files;
		}
	}
}