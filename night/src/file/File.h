#pragma once

#include "log/log.h"
//#include "stl/stl.h"
//#include <sstream>

namespace night
{

	enum EDataType : s32
	{
		Int,
		Real,
		String,
		Vector
	};

	struct NIGHT_API File
	{
		File() = default;

		u8 is_leaf() const
		{
			return _branches.empty();
		}

		void data(string const& data)
		{
			_data = data;
		}

		u8 write();
		u8 write(string const& path);
		u8 read(string const& path);

		void clear();

		File& operator[](string const& name);
		
		string const& data() const { return _data; }
		umap<string, File> const& branches() const { return _branches; }
		EDataType data_type();

		string get_string() const;
		s32 get_s32() const;
		real get_real() const;

		template<typename T>
		void get_vector(vector<T>& out_vector) const;

		template<typename... Args>
		void get_vector(Args&... args) const; // TODO: get based on type.

		template<typename... Args>
		void get_vector(vector<tuple<Args...>>& out_vector) const;

		void set_string(string const& x);
		void set_s32(s32 const& x);
		void set_real(real const& x);

		template<typename T>
		void set_vector(vector<T> const& vec);

		template<typename... Args>
		void set_vector(const Args&... args); // TODO: set based on type.

		template<typename... Args>
		void set_vector(vector<tuple<Args...>>const& vec); // TODO: set based on type.

		// TODO: evaluate arg type recursively, handle the types accordingly.
		void test_set_vector(vector<tuple<real, vector<tuple<real, real>>>> const& vecs);
		void test_get_vector(vector<tuple<real, vector<tuple<real, real>>>>& out_vecs) const;
		
		// TODO: get vector.

		u8 empty()
		{
			return _data.empty();
		}

	private:

		string _data;
		umap<string, File> _branches;
	};

	template<typename T>
	static void _get_vector(sstream& stream, T& t)
	{
		string segment;

		if (std::getline(stream, segment, ','))
		{
			//t = (real)std::stod(segment.c_str()); // TODO: get type of arg

			if constexpr (std::is_same_v<T, real>)
			{
				t = (real)std::stod(segment.c_str()); // TODO: get type of arg
			}
			else if constexpr (std::is_same_v<T, s32>)
			{
				t = std::stoi(segment.c_str()); // TODO: get type of arg
			}
			else
			{
				static_assert(false);
			}
		}
	}

	template<typename T, typename... Args>
	static void _get_vector(sstream& stream, T& t, Args&... args)
	{
		string segment;

		if (std::getline(stream, segment, ','))
		{
			if constexpr (std::is_same_v<T, real>)
			{
				t = (real)std::stod(segment.c_str()); // TODO: get type of arg
			}
			else if constexpr (std::is_same_v<T, s32>)
			{
				t = std::stoi(segment.c_str()); // TODO: get type of arg
			}
			else
			{
				static_assert(false);
			}

			_get_vector(stream, std::forward<Args&>(args)...);
		}
	}

	template<typename T>
	inline void File::get_vector(vector<T>& out_vector) const
	{
		if (_data.empty())
		{
			return;
		}

		string trim = _data;
		trim.erase(0, 1);
		trim.erase(trim.size() - 1);
		sstream stream(trim);

		string segment;

		while (std::getline(stream, segment, ','))
		{
			if constexpr (std::is_same_v<T, s32>)
			{
				out_vector.push_back((s32)std::stoi(segment.c_str()));
			}
			else if constexpr (std::is_same_v<T, real>)
			{
				out_vector.push_back((real)std::stof(segment.c_str()));
			}
			else
			{
				static_assert(false);
			}
		}
	}

	template<typename ...Args>
	inline void File::get_vector(Args & ...args) const
	{
		if (_data.empty())
		{
			return;
		}

		string trim = _data;
		trim.erase(0, 1);
		trim.erase(trim.size() - 1);
		sstream stream(trim);

		_get_vector(stream, args...);
	}

	static string _line;

	template<typename ...Args>
	static void _get_vector2(Args & ...args)
	{
		string trim = _line;
		trim.erase(0, 1);
		trim.erase(trim.size() - 1);
		sstream stream(trim);
	
		_get_vector(stream, args...);
	}

	template<typename ...Args>
	inline void File::get_vector(vector<tuple<Args...>>& out_vector) const
	{
		if (_data.empty())
		{
			return;
		}

		string vectors = data();
		vectors.erase(0, 1);
		vectors.erase(vectors.size() - 1);
		sstream stream(vectors);

		stack<s32> opens;
		for (s32 i = 0; i < vectors.size(); i++)
		{
			if (vectors[i] == '{')
			{
				opens.push(i);
			}
			else if (vectors[i] == '}')
			{
				_line = vectors.substr(opens.top(), i - opens.top() + 1);
				tuple<Args...> t;

				std::apply(_get_vector2<Args...>, t);
				out_vector.push_back(t);

				opens.pop();
			}
		}
	}

	template<typename T>
	static void _set_vector(sstream& out_stream, T& t)
	{
		out_stream << t;
	}

	template<typename T, typename... Args>
	static void _set_vector(sstream& out_stream, T& t, Args&... args)
	{
		out_stream << t << ", ";
		_set_vector(out_stream, std::forward<Args&>(args)...);
	}

	template<typename ...Args>
	inline void File::set_vector(const Args& ...args)
	{
		sstream stream;
		stream << '{';
		_set_vector(stream, args...);
		stream << '}';
		_data = stream.str();
	}

	static sstream out_stream2;
	template<typename T>
	static void _set_vector2(const T& t)
	{
		out_stream2 << t;
	}

	template<typename T, typename... Args>
	static void _set_vector2(const T& t, const Args&... args)
	{
		out_stream2 << t << ", ";
		_set_vector(out_stream2, std::forward<Args&>(args)...);
	}

	template<typename ...Args>
	inline void File::set_vector(vector<tuple<Args...>> const& vec)
	{
		//sstream stream;
		out_stream2 = sstream();

		out_stream2 << "{";
		for (s32 i = 0; i < vec.size(); i++)
		{
			out_stream2 << "{";
			const tuple<Args...>& t = vec[i];
			std::apply(_set_vector2<const Args&...>, t);
			if (i == vec.size() - 1)
			{
				out_stream2 << "}";
			}
			else
			{
				out_stream2 << "}, ";
			}
		}
		out_stream2 << "}";

		_data = out_stream2.str();
	}

	template<typename T>
	inline void File::set_vector(vector<T> const& vec)
	{
		out_stream2 = sstream();

		out_stream2 << "{";
		for (s32 i = 0; i < vec.size(); i++)
		{
			out_stream2 << vec[i];
			if (i != vec.size() - 1)
			{
				out_stream2 << ", ";
			}
		}
		out_stream2 << "}";

		_data = out_stream2.str();
	}

	inline void File::test_set_vector(vector<tuple<real, vector<tuple<real, real>>>> const& vecs)
	{
		out_stream2 = sstream();

		out_stream2 << "{";

		for (s32 i = 0; i < vecs.size(); i++)
		{
			out_stream2 << "{";
			auto& [val, vec] = vecs[i];

			out_stream2 << val;
			if (!vec.empty())
			{
				out_stream2 << ", {";
			}

			for (s32 j = 0; j < vec.size(); j++)
			{
				out_stream2 << "{";
				const tuple<real, real>& t = vec[j];
				std::apply(_set_vector2<const real&, const real&>, t);

				if (j == vec.size() - 1)
				{
					out_stream2 << "}";
				}
				else
				{
					out_stream2 << "}, ";
				}
			}

			if (i == vecs.size() - 1)
			{
				out_stream2 << "}}";
			}
			else
			{
				out_stream2 << "}}, ";
			}
		}

		out_stream2 << "}";

		_data = out_stream2.str();
	}

	inline void File::test_get_vector(vector<tuple<real, vector<tuple<real, real>>>>& out_vecs) const
	{
		string vectors = data();
		vectors.erase(0, 1);
		vectors.erase(vectors.size() - 1);
		sstream stream(vectors);

		stack<s32> opens;

		for (s32 i = 0; i < vectors.size(); i++)
		{
			if (vectors[i] == '{')
			{
				opens.push(i);
			}
			else if (vectors[i] == '}')
			{
				if (opens.size() == 1)
				{
					string vecs_elt = vectors.substr(opens.top() + 1, i - opens.top() - 1);

					stack<s32> opens2;
					for (s32 j = 0; j < vecs_elt.size(); j++)
					{
						if (vecs_elt[j] == '{')
						{
							opens2.push(j);
						}
						else if (vecs_elt[j] == '}')
						{
							if (opens2.size() == 1)
							{
								string prev = vecs_elt.substr(0, opens2.top() - 2);
								string next = vecs_elt.substr(opens2.top(), j - opens2.top() + 1);
								
								real w;
								vector<tuple<real, real>> tw;
								File dummy;

								w = (real)std::stod(prev.c_str());

								dummy.data(next);
								dummy.get_vector(tw);

								out_vecs.push_back({ w, tw });
							}

							opens2.pop();
						}
					}
				}

				opens.pop();
			}
		}
	}

	namespace debug
	{
		template<> inline string _Log::_print_format<File>(File& df)
		{
			sstream stream;
			stream << '\n';

			function<void(const File&, s32)> rec = [&](const File& current_node, s32 indents)
			{
				for (auto i = current_node.branches().begin(); i != current_node.branches().end(); i++)
				{
					if ((*i).second.is_leaf())
					{
						for (s32 j = 0; j < indents; j++)
						{
							stream << '\t';
						}
						stream << (*i).first << ": " << (*i).second.data();
						stream << '\n';
					}
					else
					{
						stream << (*i).first << ":\n";
						rec((*i).second, indents + 1);
					}
				}
			};

			rec(df, 0);

			return stream.str();
		}
	}

}