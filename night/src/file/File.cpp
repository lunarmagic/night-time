
#include "nightpch.h"
#include "File.h"
//#include "log/log.h"

namespace night
{
	u8 File::write()
	{
		if (_data.empty())
		{
			ERROR("file not open!, or you changed root data.");
			return false;
		}

		if (!std::filesystem::exists(_data))
		{
			ERROR("file does not exist!, or you changed root data.");
			return false;
		}

		return write(_data);
	}

	u8 File::write(string const& path)
	{
		function<void(File const&, ofstream&, s32)> rf = [&](File const& df, ofstream& out_stream, s32 depth)
		{
			for (auto i = df.branches().begin(); i != df.branches().end(); i++)
			{
				if ((*i).first == "Header")
				{
					continue;
				}

				for (s32 j = 0; j < depth; j++)
				{
					out_stream << '\t';
				}

				auto& branch = (*i);

				if (!branch.second.is_leaf())
				{
					out_stream << branch.first << ":\n";
					rf(branch.second, out_stream, depth + 1);
				}
				else
				{
					out_stream << branch.first << ": " << branch.second.data() << "\n";
				}
			}
		};

		ofstream file(path.c_str());
		if (file.is_open())
		{
			auto header = _branches.find("Header");
			if (header != _branches.end())
			{
				file << "#Header\n";
				rf((*header).second, file, 0);
				file << "#Body\n";
			}

			rf(*this, file, 0);
		}
		else
		{
			ERROR("File did not open!");
			return false;
		}

		file.close();

		return true;
	}

	// TODO: add header only option.
	u8 File::read(string const& path)
	{
		if (!std::filesystem::exists(path))
		{
			ofstream file(path.c_str());
			if (file.is_open())
			{
				file.close();
			}
		}

		clear();
		this->data(path);

		ifstream file(path.c_str());

		if (file.is_open())
		{
			string line;

			function<void(File&, s32, sstream&)> rf = [&](File& current_node, s32 num_indents, sstream& stream)
				{
					while (std::getline(stream, line))
					{
					GO:
						if (line.empty())
						{
							continue;
						}

						s32 indents = (s32)line.find_first_not_of('\t');

						if (indents < num_indents)
						{
							return;
						}

						string data = line;
						data.erase(std::remove(data.begin(), data.end(), '\t'), data.end());
						string label = data.substr(0, data.find(':'));
						data.erase(0, data.find(':') + 1);
						data.erase(0, 1);

						ASSERT(label != "Header");

						if (!data.empty())
						{
							current_node[label].data(data);
							line.clear();
						}
						else
						{
							auto& child = current_node[label];
							rf(child, num_indents + 1, stream);
							goto GO;
						}
					}
				};

			string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			string hdelimiter = "#Header\n";
			string bdelimiter = "#Body\n";

			size_t hend = data.find(hdelimiter);
			size_t bend = data.find(bdelimiter);

			if (hend == string::npos && bend == string::npos)
			{
				sstream str(data);
				rf((*this), 0, str);
			}
			else
			{
				ASSERT(hend < bend);
				sstream header(string(data.substr(hend + hdelimiter.size(), bend - hdelimiter.size())));
				sstream body(string(data.substr(bend + bdelimiter.size(), data.size() - (bend + bdelimiter.size()))));

				rf((*this)["Header"], 0, header);
				rf((*this), 0, body);
			}

			return true;
		}

		return false;
	}

	void File::clear()
	{
		_branches.clear();
		_data.clear();
	}

	File& File::operator[](string const& name)
	{
		return _branches[name];
	}

	EDataType File::data_type()
	{
		if (_data[0] == '"' && _data[_data.size() - 1] == '"')
		{
			return EDataType::String;
		}
		else if (_data[0] == '{' && _data[_data.size() - 1] == '}')
		{
			return EDataType::Vector;
		}
		else if (_data.find(".") != string::npos)
		{
			return EDataType::Real;
		}
		else
		{
			return EDataType::Int;
		}
	}

	void File::set_string(string const& x)
	{
		_data = '"' + x + '"';
	}

	void File::set_s32(s32 const& x)
	{
		_data = to_string(x);
	}

	void File::set_real(real const& x)
	{
		_data = to_string(x);
	}

	string File::get_string() const
	{
		if (_data.empty())
		{
			return "";
		}

		string result = _data;
		result.pop_back();
		result.erase(result.begin());
		return result;
	}

	s32 File::get_s32() const
	{
		if (_data.empty())
		{
			return 0;
		}

		return std::atoi(_data.c_str());
	}

	real File::get_real() const
	{
		if (_data.empty())
		{
			return 0;
		}

		return (real)std::stod(_data.c_str());
	}
}